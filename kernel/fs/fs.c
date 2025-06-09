#include <os/string.h>
#include <os/fs.h>
#include <pgtable.h>
#include <assert.h>
#include <os/kernel.h>
#include <os/sched.h>
#include <os/smp.h>
#include <screen.h>
#include <os/time.h>

static fdesc_t fdesc_array[NUM_FDESCS];

static inode_t current_inode;
static superblock_t superblock;
static char imap[INODE_MAP_SEC_NUM*SECTOR_SIZE];
static char bmap[BLOCK_MAP_SEC_NUM*SECTOR_SIZE];
static char buffer[BLOCK_SIZE];
static char buffer2[BLOCK_SIZE];
static char level_buffer[3][BLOCK_SIZE];

// 检查是否已经存在文件系统
static inline int fs_exist(){
    // 超级块已经载入内存
    if(superblock.magic_number == SUPERBLOCK_MAGIC)
        return 1;
    // 从磁盘中读取文件系统
    bios_sd_read(kva2pa(&superblock), 1, FS_START_SEC);
    if(superblock.magic_number == SUPERBLOCK_MAGIC){
        // 读入imap和bmap信息
        bios_sd_read(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
        bios_sd_read(kva2pa(bmap), BLOCK_MAP_SEC_NUM, FS_START_SEC + BLOCK_MAP_OFFSET);
        // 初始化current_inode信息
        bios_sd_read(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET);
        memcpy(&current_inode, buffer, sizeof(inode_t));
        return 1;
    }
    return 0;
}

// 返回空闲block在磁盘上的地址
static inline int alloc_block(uint32_t* addr_array, int num){
    static int i, j =0;
    static int mask = 1;
    int cnt = 0;
    for(; i<BLOCK_MAP_SEC_NUM*SECTOR_SIZE; i++){
        for(; j<8; j++, mask<<=1){
            if((bmap[i] & mask) == 0){
                bmap[i] |= mask;

                int blk_index = i*8+j;
                uint32_t data_blk_addr = blk_index*BLOCK_SIZE/SECTOR_SIZE + FS_START_SEC + DATA_BLOCK_OFFSET;
                bzero(buffer2, BLOCK_SIZE);
                bios_sd_write(kva2pa(buffer2), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
                addr_array[cnt] = data_blk_addr;
                cnt++;
                // 将未使用的inode结点置为已用，改成一并写回，否则上板可能会卡住
                if(cnt==num){
                    // 只需写回sec_num个块
                    int sec_num = CEIL_DIV(blk_index, SECTOR_SIZE);
                    sec_num = sec_num == 0 ? 1 : sec_num; // 至少写回一个sector
                    bios_sd_write(kva2pa(bmap), sec_num, FS_START_SEC + BLOCK_MAP_OFFSET);
                    return 1;
                }
            }  
        }
        if(j==8){
                j=0;
                mask=1;
        }
        if(i==BLOCK_MAP_SEC_NUM*SECTOR_SIZE-1 && cnt<num){
            // 如果已经遍历完所有的block map，说明没有空闲的block了
            i = BLOCK_MAP_SEC_NUM*SECTOR_SIZE; // 退出循环
            j = 0; // 重置j
            mask = 1; // 重置mask
        }
    }
    printk("[ALLOC_BLOCK] Warning: data block has been used up!\n");
}

// 返回空闲inode的下标
static inline int alloc_inode(){
    int i, j, mask;
    // 读取inode map info
    // bios_sdread(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
    for(i=0; i<INODE_MAP_SEC_NUM*SECTOR_SIZE; i++){
        for(j=0, mask=1; j<8; j++, mask<<=1){
            if((imap[i] & mask) == 0){
                // 将未使用的inode结点置为已用
                imap[i] |= mask;
                bios_sd_write(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
                return i*8+j;
            }
        }
    }
}

// 做基础的inode初始化
static inline inode_t set_inode(short type, int mode, int ino){
    inode_t node;
    bzero(&node, sizeof(inode_t));
    node.type = type;
    node.mode = mode;
    node.ino = ino;
    node.size = 0;
    node.nlink = 1;
    node.atime = node.mtime = node.ctime =get_timer();
    return node;
}
// 根据ino获取inode
static inline inode_t* ino2inode(int ino){
    assert(ino>=0 && ino<INODE_NUM*SECTOR_SIZE/sizeof(inode_t));
    int offset = ino/IPSEC;
    bios_sd_read(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    return ((inode_t *)buffer)+ ino - offset * IPSEC;
}

// 根据name查找对应inode（在node指示的目录下）,查找到了返回1，并将inode存于res
static inline int get_inode_from_name(inode_t node, char* name, inode_t* res){
    // 基准inode非目录类型
    if(node.type != T_DIR)
        return 0;
    // 读入对应目录页
    bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, node.direct_addrs[0]);
    dentry_t* de = (dentry_t*) buffer;
    for(int i=0; i<DPBLK; i++){
        if(de[i].name[0]==0)
            continue;
        else if(strcmp(de[i].name, name)==0){
            if(res!=NULL)
                *res = *ino2inode(de[i].ino);
            return 1;
        }
    }
    return 0;
}   

// 以current_node为基准解析路径，找到返回1，并将对应inode存储于res
static inline int parse_path(inode_t node, char* path, inode_t* res){
    if(path==NULL || path[0]==0){
        if(res!=NULL)
            *res = node;
        return 1;
    }
    int len;
    for(len=0; len<strlen(path); len++){
        if(path[len]=='/')
            break;
    }
    char name[len+1];
    memcpy(name, path, len);
    name[len] = '\0';
    inode_t tmp;
    if(get_inode_from_name(node, name, &tmp)==0)
        return 0;
    return parse_path(tmp, path+len+1, res);
}

void setup_level_index(uint32_t data_blk_addr, int level){
    uint32_t* addr_array = level_buffer[level];
    // printk("I am in setup_level_index\n");
    alloc_block(addr_array, IA_PER_BLOCK);
    if(level)
        for(int i=0; i<IA_PER_BLOCK; i++)
            setup_level_index(addr_array[i], level-1);
    bios_sd_write(kva2pa(level_buffer[level]), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
}

void recursive_recycle(uint32_t data_blk_addr, int level, char*zero_buff){
    if(level){
        uint32_t* addr_array = level_buffer[level-1];
        bios_sd_read(kva2pa(level_buffer[level-1]), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        for(int i=0; i<IA_PER_BLOCK; i++){
            recursive_recycle(addr_array[i], level-1, zero_buff);
        }
    }
    // 清空数据块
    bios_sd_write(kva2pa(zero_buff), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
    // 修改bmap
    int bno = (data_blk_addr - FS_START_SEC - DATA_BLOCK_OFFSET) *SECTOR_SIZE / BLOCK_SIZE;
    bmap[bno/8] &= ~(1 << (bno%8));
}

void recycle_level_index(uint32_t data_blk_addr, int level){
    bzero(buffer, BLOCK_SIZE);
    recursive_recycle(data_blk_addr, level, buffer);
    // 最后统一写回bmap
    bios_sd_write(kva2pa(bmap), BLOCK_MAP_SEC_NUM, FS_START_SEC + BLOCK_MAP_OFFSET);
}

uint32_t get_data_block_addr(inode_t node, int size){
    // printk("I am in get_data_block_addr\n");
    // 使用直接索引
    if(size<DIRECT_SIZE){
        int index = size/BLOCK_SIZE;
        // 未分配的情况
        if(node.direct_addrs[index]==0){
            uint32_t data_blk_addr;
            alloc_block(&data_blk_addr, 1);
            inode_t* node_ptr = ino2inode(node.ino);
            node_ptr->direct_addrs[index] = data_blk_addr;
            int offset = node.ino / IPSEC;
            bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
            return node_ptr->direct_addrs[index];
        }
        return node.direct_addrs[index];
    }
    // 使用一级索引
    else if(size < DIRECT_SIZE + INDIRECT_1ST_SIZE){
        size -= DIRECT_SIZE;
        int index1 = size/(BLOCK_SIZE*IA_PER_BLOCK);
        int index2 = (size - index1*BLOCK_SIZE*IA_PER_BLOCK)/BLOCK_SIZE;
        uint32_t data_blk_addr;
        if(node.indirect_addrs_1st[index1]==0){
            alloc_block(&data_blk_addr, 1);
            setup_level_index(data_blk_addr, 0);
            // printk("Return from setup_level_index\n");
            inode_t* node_ptr = ino2inode(node.ino);
            node_ptr->indirect_addrs_1st[index1] = data_blk_addr;
            int offset = node.ino / IPSEC;
            bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
        }
        else
            data_blk_addr = node.indirect_addrs_1st[index1];
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        uint32_t* addr_array = buffer;
        return addr_array[index2];
    }
    // 使用二级索引
    else if(size < DIRECT_SIZE + INDIRECT_1ST_SIZE + INDIRECT_2ND_SIZE){
        size -= (DIRECT_SIZE + INDIRECT_1ST_SIZE);
        int index1 = size/(BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK);
        int index2 = (size - index1*BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK)/(BLOCK_SIZE*IA_PER_BLOCK);
        int index3 = (size - index1*BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK - index2*IA_PER_BLOCK)/BLOCK_SIZE;
        uint32_t data_blk_addr;
        if(node.indirect_addrs_2nd[index1]==0){
            alloc_block(&data_blk_addr, 1);
            setup_level_index(data_blk_addr, 1);
            inode_t* node_ptr = ino2inode(node.ino);
            node_ptr->indirect_addrs_2nd[index1] = data_blk_addr;
            int offset = node.ino / IPSEC;
            bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
        }
        else
            data_blk_addr = node.indirect_addrs_2nd[index1];
        // 获取二级索引项
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        uint32_t* addr_array = buffer;
        data_blk_addr = addr_array[index2];
        // 获取一级索引项
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        return addr_array[index3];
    }
    // 使用三级索引
    else if(size < DIRECT_SIZE + INDIRECT_1ST_SIZE + INDIRECT_2ND_SIZE + INDIRECT_3RD_SIZE){
        size -= (DIRECT_SIZE + INDIRECT_1ST_SIZE + INDIRECT_2ND_SIZE);
        int index1 = size/(BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK);
        int index2 = (size - index1*BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK)/(BLOCK_SIZE*IA_PER_BLOCK);
        uint32_t data_blk_addr;
        if(node.indirect_addrs_3rd==0){
            alloc_block(&data_blk_addr, 1);
            setup_level_index(data_blk_addr, 1);
            inode_t* node_ptr = ino2inode(node.ino);
            node_ptr->indirect_addrs_3rd = data_blk_addr;
            int offset = node.ino / IPSEC;
            bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
            
        }
        else
            data_blk_addr = node.indirect_addrs_3rd;
        // 获取二级索引项
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        uint32_t* addr_array = buffer;
        data_blk_addr = addr_array[index1];
        // 获取一级索引项
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        return addr_array[index2];
    }
    else{
        printk("[Error] size out of bound!\n");
        return 0;
    }
}

int do_mkfs(int force_flag)
{
    // TODO [P6-task1]: Implement do_mkfs
    if(force_flag){
        // 只需要清理元数据
        int blk_num =CEIL_DIV( DATA_BLOCK_OFFSET * SECTOR_SIZE, BLOCK_SIZE);
        // 清除内存中缓存
        bzero((char*)&superblock, sizeof(superblock_t));
        // 清除磁盘内容
        bzero(buffer, BLOCK_SIZE);
        int cur_y = current_running[cpu_id]->cursor_y;
        for(int i=0; i<blk_num; i+=BLOCK_SIZE/SECTOR_SIZE){
            bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE ,FS_START_SEC+i);
            screen_move_cursor(0, cur_y);
            printk("[FS] Cleaning blocks %d/%d\n", i, blk_num);
        }
        printk("[FS] Cleanning finished! Reseting...\n");
    }
    // 检查是否已经建立文件系统
    else if(fs_exist()){
        printk("[FS] Warning: filesystem has already been set up!\n");
        printk("[FS] Info: use 'mkfs -f' to reset filesystem.\n");
        return 1;   // do_mkfs failed
    }
    printk("[FS] Start initialize filesystem!\n");
    printk("[FS] Setting superblock...\n");
    // 初始化相关参数
    superblock.magic_number = SUPERBLOCK_MAGIC;
    superblock.fs_start_sec = FS_START_SEC;
    superblock.fs_size = FS_SIZE;
    superblock.block_map_offset = BLOCK_MAP_OFFSET;
    superblock.inode_map_offset = INODE_MAP_OFFSET;
    superblock.inode_offset = INODE_OFFSET;
    superblock.inode_num = INODE_NUM;
    superblock.data_block_offset = DATA_BLOCK_OFFSET;
    superblock.data_block_num = DATA_BLOCK_NUM;
    // 打印相关信息
    printk("\t magic: 0x%x\n", superblock.magic_number);
    printk("\t num sector: %d, start sector: %d\n", superblock.fs_size, superblock.fs_start_sec);
    printk("\t block map offset: %d(%d)\n", superblock.block_map_offset, BLOCK_MAP_SEC_NUM);
    printk("\t inode map offset: %d(%d)\n", superblock.inode_map_offset, INODE_MAP_SEC_NUM);
    printk("\t inode offset: %d(%d), inode num: %d\n", superblock.inode_offset, INODE_SEC_NUM, superblock.inode_num);
    printk("\t data offset: %d(%d), data block num: %d\n", superblock.data_block_offset, DATA_BLOCK_SEC, superblock.data_block_num);
    // 将superblock写入扇区
    bios_sd_write(kva2pa(&superblock), 1, FS_START_SEC);
    // 初始化inode map
    printk("[FS] Setting inode-map...\n");
    bzero(imap, INODE_MAP_SEC_NUM*SECTOR_SIZE);   // 全部置为0，表示为未使用
    bios_sd_write(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
    // 初始化block map
    printk("[FS] Setting sector-map...\n");
    bzero(bmap, BLOCK_MAP_SEC_NUM*SECTOR_SIZE);
    bios_sd_write(kva2pa(bmap), BLOCK_MAP_SEC_NUM, FS_START_SEC + BLOCK_MAP_OFFSET);
    // 创建根目录
    // 1. 创建inode
    printk("[FS] Setting inode...\n");
    int root_index = alloc_inode();
    // 2.创建dentry，并写回数据块
    bzero(buffer, 512);
    dentry_t * de = (dentry_t*) buffer;
    strcpy(de[0].name, ".");
    strcpy(de[1].name, "..");
    de[0].ino = de[1].ino = root_index;
    uint32_t data_blk_addr;
    alloc_block(&data_blk_addr, 1);
    bios_sd_write(kva2pa(buffer), 1,  data_blk_addr);
    // 3.写回inode
    inode_t *root_inode = ino2inode(root_index);
    *root_inode = set_inode(T_DIR, O_RDWR, root_index);
    root_inode->direct_addrs[0] = data_blk_addr;
    root_inode->size = 2*sizeof(dentry_t);
    current_inode = *root_inode; // 更新当前inode
    int offset = root_index / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    // 初始化文件描述符
    bzero(fdesc_array, sizeof(fdesc_t)*NUM_FDESCS);
    printk("[FS] Initialize filesystem finished!\n");
    return 0;  // do_mkfs succeeds
}

int do_statfs(void)
{
    // TODO [P6-task1]: Implement do_statfs
    if(!fs_exist()){
        printk("[FS] Warning: filesystem has not been set up!\n");
        return 1;
    }
    int iused_cnt=0, bused_cnt=0, i, j, mask;
    for(i=0; i<INODE_MAP_SEC_NUM*SECTOR_SIZE; i++){
        for(j=0, mask=1; j<8; j++, mask<<=1){
            if(imap[i] & mask)
                iused_cnt++;
        }
    }
    for(i=0; i<BLOCK_MAP_SEC_NUM*SECTOR_SIZE; i++){
        for(j=0, mask=1; j<8; j++, mask<<=1){
            if(bmap[i] & mask)
                bused_cnt++;
        }
    }
    printk("[FS] state:\n");
    printk("\t magic: 0x%x\n", superblock.magic_number);
    printk("\t total sector: %d, start sector: %d(%08x)\n", superblock.fs_size, superblock.fs_start_sec, superblock.fs_start_sec);
    printk("\t block map offset: %d, occupied sector: %d\n", superblock.block_map_offset, BLOCK_MAP_SEC_NUM);
    printk("\t inode map offset: %d, occupied sector: %d\n", superblock.inode_map_offset, INODE_MAP_SEC_NUM);
    printk("\t inode block offset: %d, usage %d/%d\n", superblock.inode_offset, iused_cnt, superblock.inode_num);
    printk("\t data block offset: %d, usage %d/%d\n", superblock.data_block_offset, bused_cnt, superblock.data_block_num);
    printk("\t inode size: %dB, dir entry size: %dB\n", sizeof(inode_t), sizeof(dentry_t));
    return 0;  // do_statfs succeeds
}

int do_cd(char *path)
{
    // TODO [P6-task1]: Implement do_cd
    if(!fs_exist()){
        printk("[CD] Warning: filesystem has not been set up!\n");
        return 1;
    }
    inode_t tmp;
    // 不存在该目录
    if(parse_path(current_inode, path, &tmp)==0){
        printk("[CD] Error: No such directory!\n");
        return 2;
    } 
    if(tmp.type == T_FILE){
        printk("[CD] Error: Cannot enter a file!\n");
        return 3;
    }
    current_inode = tmp;
    return 0;  // do_cd succeeds
}

int do_mkdir(char *path)
{
    // TODO [P6-task1]: Implement do_mkdir
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 同名文件/目录已经存在
    inode_t tmp;
    inode_t father_tmp;
    char father_path[20];
    father_path[0]='.';
    for(int j=1; j<20; j++){
        father_path[j] = '\0';
    }
    for(int i = strlen(path);i>=0;i--){
        if(path[i]=='/'){
            memcpy(father_path, path, i);
            father_path[i] = '\0';
            int path_len = strlen(path);
            for(int j=0;j<20;j++){
                if(j<path_len-i)
                    path[j] = path[i+1+j];
                else
                    path[j] = '\0';
            }
            break;
        }
    }

    parse_path(current_inode, father_path, &father_tmp);

    if(get_inode_from_name(father_tmp, path, NULL))
        return 1;
    // 创建目录
    // 1. 创建inode
    int ino = alloc_inode();
    // 2.创建dentry，并写回数据块
    bzero(buffer, 512);
    dentry_t * de = (dentry_t*) buffer;
    strcpy(de[0].name, ".");
    strcpy(de[1].name, "..");
    de[0].ino = ino;
    de[1].ino = father_tmp.ino;
    uint32_t data_blk_addr;
    alloc_block(&data_blk_addr, 1);
    bios_sd_write(kva2pa(buffer), 1,  data_blk_addr);
    // 3.写回inode
    inode_t *node = ino2inode(ino);
    *node = set_inode(T_DIR, O_RDWR, ino);
    node->direct_addrs[0] = data_blk_addr;
    node->size = 2*sizeof(dentry_t);
    int offset = ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    int start_sec = father_tmp.direct_addrs[0] + father_tmp.size/SECTOR_SIZE;
    bios_sd_read(kva2pa(buffer), 1, start_sec);
    int i;
    for(i=0; i<DPSEC; i++){
        if(de[i].name[0]==0)
            break;
    }
    strcpy(de[i].name, path);
    de[i].ino = ino;
    bios_sd_write(kva2pa(buffer), 1, start_sec);
    // 3.修改父目录inode结点
    inode_t* node_ptr = ino2inode(father_tmp.ino);
    node_ptr->size += sizeof(dentry_t);
    offset = father_tmp.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);

    return 0;  // do_mkdir succeeds
}

int do_rmdir(char *path)
{
    // TODO [P6-task1]: Implement do_rmdir
    if(!fs_exist()){
        printk("[RMDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    inode_t node;
    // 未找到该目录
    if(get_inode_from_name(current_inode, path, &node)==0){
        printk("[RMDIR] Error: No such directory!\n");
        return 1;
    }
    // 判断是否为目录
    if(node.type!=T_DIR){
        printk("[RMDIR] Failed!It is not a directory!\n");
        return 1;
    }
    // 判断是否含有子目录
    if(node.size>sizeof(dentry_t)*2){
        printk("[RMDIR] Failed!The diretory contains sub-dirs!\n");
        return 1;
    }
    // 删除目录
    node.nlink--;
    if(node.nlink==0){
        // 1.待删除目录的inode处理
        int offset = node.ino/IPSEC;
        inode_t* tmp = ino2inode(node.ino);
        // bios_sdread(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);    // 调用get_inode_from_name后buffer里已经为该内容
        bzero(tmp, sizeof(inode_t));
        bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
        // 2.处理inode bitmap
        imap[node.ino/8] &= ~(1 << (node.ino%8));
        bios_sd_write(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
        // 3.待删除目录分配的数据块处理
        uint32_t data_blk_addr = node.direct_addrs[0];
        bzero(buffer, BLOCK_SIZE);
        bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, data_blk_addr);
        // 4.处理数据块的bitmap
        int bno = (data_blk_addr - FS_START_SEC - DATA_BLOCK_OFFSET) *SECTOR_SIZE / BLOCK_SIZE;
        bmap[bno/8] &= ~(1 << (bno%8));
        bios_sd_write(kva2pa(bmap), BLOCK_MAP_SEC_NUM, FS_START_SEC + BLOCK_MAP_OFFSET);
        // 5.父目录的间址块处理
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, current_inode.direct_addrs[0]);
        dentry_t* de = (dentry_t*) buffer;
        int cur;
        for(cur=0; cur<BLOCK_SIZE/sizeof(dentry_t); cur++)
            if(de[cur].ino == node.ino)
                break;
        bzero(&de[cur], sizeof(dentry_t));
        bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, current_inode.direct_addrs[0]);
        // 6.处理父目录inode的size域
        inode_t* node_ptr = ino2inode(current_inode.ino);
        node_ptr->size -= sizeof(dentry_t);
        offset = current_inode.ino / IPSEC;
        bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    }
    else{
        // 只需写回nlink域
        int offset = node.ino/IPSEC;
        inode_t* node_ptr = ino2inode(node.ino);
        node_ptr->nlink = node.nlink;
        bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    }
    return 0;  // do_rmdir succeeds
}

int do_ls(char *path, int option)
{
    // TODO [P6-task1]: Implement do_ls
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // Note: argument 'option' serves for 'ls -l' in A-core
    inode_t node;
    if(path[0]==0)
        node = current_inode;
    else if(parse_path(current_inode, path, &node)==0)
        return 1;   // 找不到该路径
    // 这里要用buffer2是因为后续in2inode时会覆盖buffer中的内容
    bios_sd_read(kva2pa(buffer2), BLOCK_SIZE/SECTOR_SIZE, node.direct_addrs[0]);
    dentry_t* de = buffer2;

    for(int i=2; i<DPSEC; i++){
        if(de[i].name[0]==0)
            continue;
        if(option){ // 需打印详细信息
            printk("ino: %d ", de[i].ino);
            // printk("inode_ptr: %x\t", ino2inode(de[i].ino));
            inode_t tmp = *ino2inode(de[i].ino);
            printk("%c%c%c nlink:%d ctime:%d atime:%d mtime:%d size:%d %s\n", 
                    tmp.type == T_DIR ? 'd' : '-',
                    (tmp.mode & O_RDONLY) ? 'r' : '-',
                    (tmp.mode & O_WRONLY) ? 'w' : '-',
                    tmp.nlink, tmp.ctime, tmp.atime, tmp.mtime, tmp.size
                    ,de[i].name
                    );
        }
        else
            printk("\t%s", de[i].name);
    }
    if(option==0)
        printk("\n");
    return 0;  // do_ls succeeds
}

int do_touch(char *path)
{
    // TODO [P6-task2]: Implement do_touch
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 同名文件/目录已经存在
    if(get_inode_from_name(current_inode, path, NULL))
        return 1;
    // 创建文件
    // 1. 创建inode并写回
    int data_blk_addr;
    alloc_block(&data_blk_addr, 1);
    int ino = alloc_inode();
    inode_t *node = ino2inode(ino);
    *node = set_inode(T_FILE, O_RDWR, ino);
    node->direct_addrs[0] = data_blk_addr;
    int offset = ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    // 2.修改父目录：增加目录项
    int start_sec = current_inode.direct_addrs[0] + current_inode.size/SECTOR_SIZE;
    bios_sd_read(kva2pa(buffer), 1, start_sec);
    dentry_t* de = (dentry_t*)buffer;
    int i;
    for(i=0; i<DPSEC; i++){
        if(de[i].name[0]==0)
            break;
    }
    strcpy(de[i].name, path);
    de[i].ino = ino;
    bios_sd_write(kva2pa(buffer), 1, start_sec);
    // 3.修改父目录inode结点
    inode_t* node_ptr = ino2inode(current_inode.ino);
    node_ptr->size += sizeof(dentry_t);
    offset = current_inode.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    return 0;  // do_touch succeeds
}

int do_cat(char *path)
{
    // TODO [P6-task2]: Implement do_cat
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 1.查找对应文件结点
    inode_t node;
    // 未找到该文件
    if(parse_path(current_inode, path, &node)==0){
        printk("[CAT] Fail to find the file!\n");
        return -1;
    }
    if(node.size > BLOCK_SIZE){
        printk("[CAT] Warning: the file is to large!\n");
        return -1;
    }
    // 判断是否为文件类型
    if(node.type!=T_FILE){
        printk("[FOPEN] Failed!It is not a file!\n");
        return -1;
    }
    // 以block为单位打印
    for(int read_ptr = 0; read_ptr<node.size; read_ptr+=BLOCK_SIZE){
        uint32_t read_addr =  get_data_block_addr(node, read_ptr);   
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, read_addr);
        printk(buffer);
    }
    return 0;  // do_cat succeeds
}


int do_open(char *path, int mode)
{
    // TODO [P6-task2]: Implement do_open
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 1.查找对应文件结点
    inode_t node;
    // 未找到该文件
    if(get_inode_from_name(current_inode, path, &node)==0){
        printk("[FOPEN] Fail to find the file!\n");
        return -1;
    }
    // 判断是否为文件类型
    if(node.type!=T_FILE){
        printk("[FOPEN] Failed!It is not a file!\n");
        return -1;
    }
    if(((node.mode & O_RDONLY)>(mode & O_RDONLY))||(node.mode & O_WRONLY)>(mode & O_WRONLY)){
        printk("[FOPEN] Failed to access file! Mode info: %d vs %d\n", node.mode, mode);
        return -1;
    }
    // 2.分配描述符
    int fd;
    for(fd=0; fd<NUM_FDESCS; fd++){
        if(fdesc_array[fd].valid==0)
            break;
    }
    fdesc_array[fd].valid = 1;
    fdesc_array[fd].mode = mode;   // [TODO]这个保存的mode是谁的mode？
    fdesc_array[fd].ref++;
    fdesc_array[fd].read_ptr = 0;
    fdesc_array[fd].write_ptr = 0;
    fdesc_array[fd].ino = node.ino;
    return fd;  // return the fd of file descriptor
}

int do_read(int fd, char *buff, int length)
{
    // TODO [P6-task2]: Implement do_read
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    if(fd>=NUM_FDESCS || fd<0 || fdesc_array[fd].valid==0){
        printk("[FREAD] Warning: invalid file descriptor!\n");
        return -1;
    }
    // 判断文件是否可读
    if((fdesc_array[fd].mode & O_RDONLY)==0){
        printk("[FREAD] No right to read file!\n");
        return -1;
    }
    inode_t node = *ino2inode(fdesc_array[fd].ino);
    int len = length> MAX_FILE_SIZE - fdesc_array[fd].read_ptr ? MAX_FILE_SIZE - fdesc_array[fd].read_ptr : length;
    // 以block为单位读取
    for(int read_ptr = fdesc_array[fd].read_ptr; read_ptr<fdesc_array[fd].read_ptr + len;){
        int partial_len = read_ptr % BLOCK_SIZE ? (BLOCK_SIZE - (read_ptr % BLOCK_SIZE)) : BLOCK_SIZE;
        int tmp = fdesc_array[fd].read_ptr + len - read_ptr;
        partial_len = partial_len > tmp ? tmp : partial_len;
        uint32_t read_addr =  get_data_block_addr(node, read_ptr);   
        bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, read_addr);
        memcpy(buff, buffer+(read_ptr%BLOCK_SIZE), partial_len);
        read_ptr += partial_len;
        buff += partial_len;
    }
    fdesc_array[fd].read_ptr += len;
    // 修改inode的atime
    inode_t *node_ptr = ino2inode(node.ino);
    node_ptr->atime = get_timer();
    int offset = node.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);

    return len;  // return the length of trully read data
}

int do_write(int fd, char *buff, int length)
{
    // TODO [P6-task2]: Implement do_write
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    if(fd>=NUM_FDESCS || fd<0 || fdesc_array[fd].valid==0){
        printk("[FWRITE] Warning: invalid file descriptor!\n");
        return -1;
    }
    if((fdesc_array[fd].mode & O_WRONLY)==0){
        printk("[FWRITE] No right to write file!\n");
        return -1;
    }
    inode_t node = *ino2inode(fdesc_array[fd].ino);
    int len = (length> (MAX_FILE_SIZE - fdesc_array[fd].write_ptr)) ? (MAX_FILE_SIZE - fdesc_array[fd].write_ptr) : length;
    // 以block为单位写
    for(int write_ptr = fdesc_array[fd].write_ptr; write_ptr<fdesc_array[fd].write_ptr + len;){
        int partial_len = write_ptr % BLOCK_SIZE ? (BLOCK_SIZE - (write_ptr % BLOCK_SIZE)) : BLOCK_SIZE;
        int tmp = fdesc_array[fd].write_ptr + len - write_ptr;
        partial_len = partial_len > tmp ? tmp : partial_len;
        uint32_t write_addr =  get_data_block_addr(node, write_ptr);  
        // printk("return from get_data_block_addr\n"); 
        if(write_ptr % BLOCK_SIZE || partial_len<BLOCK_SIZE)
            bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, write_addr);
        memcpy(buffer + (write_ptr%BLOCK_SIZE), buff, partial_len);
        bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, write_addr);
        write_ptr += partial_len;
        buff += partial_len;
    }
    fdesc_array[fd].write_ptr += len;
    // 需要更新inode的size信息：size和mtime
    inode_t *node_ptr = ino2inode(node.ino);
    node_ptr->mtime = get_timer();
    if(fdesc_array[fd].write_ptr>node.size)
        node_ptr->size = fdesc_array[fd].write_ptr;
    int offset = node.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    
    return len;  // return the length of trully written data
}

int do_close(int fd)
{
    // TODO [P6-task2]: Implement do_close
    // 检查fd是否越界
    if(fd>=NUM_FDESCS || fd<0){
        printk("[FCLOSE] Warning: invalid file descriptor!\n");
        return -1;
    }
    fdesc_array[fd].ref--;
    if(fdesc_array[fd].ref==0){
        bzero(&fdesc_array[fd], sizeof(fdesc_t));
    }
    return 0;  // do_fclose succeeds
}

int do_ln(char *src_path, char *dst_path)
{
    // TODO [P6-task2]: Implement do_ln
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 1. 判断文件系统是否存在
    if(!fs_exist()){
        printk("[LN] Warning: filesystem has not been set up!\n");
        return 1;
    }
    inode_t node, tmp;
    // 2. 判断是否存在src文件
    if(parse_path(current_inode, src_path, &node)==0){
        printk("[LN] Error: cannot find file %s!\n", src_path);
        return 2;
    }
    if(node.type==T_DIR){
        printk("[LN] Error: cannot link a directory!\n");
        return 3;
    }
    // 3. 判断dst_path是否已经被占用
    if(get_inode_from_name(current_inode, dst_path, &tmp)){
        printk("[LN] Error: file %s has existed!\n", dst_path);
        return 4;
    }
    // 4. 修改对应inode结点
    inode_t *node_ptr = ino2inode(node.ino);
    node_ptr->nlink++;
    int offset = node.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    // 5. 修改父目录:创建目录项
    int start_sec = current_inode.direct_addrs[0] + current_inode.size/SECTOR_SIZE;
    bios_sd_read(kva2pa(buffer), 1, start_sec);
    int i;
    dentry_t* de = (dentry_t*)buffer;
    for(i=0; i<DPSEC; i++){
        if(de[i].name[0]==0)
            break;
    }
    strcpy(de[i].name, dst_path);
    de[i].ino = node.ino;
    bios_sd_write(kva2pa(buffer), 1, start_sec);
    // 6. 修改父目录inode结点
    node_ptr = ino2inode(current_inode.ino);
    node_ptr->size += sizeof(dentry_t);
    offset = current_inode.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    return 0;  // do_ln succeeds 
}

int do_rm(char *path)
{
    // TODO [P6-task2]: Implement do_rm
    // 1. 判断文件系统是否存在
    if(!fs_exist()){
        printk("[RM] Warning: filesystem has not been set up!\n");
        return 1;
    }
    // 2. 判断是否存在文件
    inode_t node;
    if(get_inode_from_name(current_inode, path, &node)==0){
        printk("[RM] Error: No such file!\n");
        return 2;
    }
    if(node.type==T_DIR){
        printk("[RM] Error: cannot rm a directory!\n");
        return 3;
    }

    // 3. 判断是否需要释放对应inode结点
    node.nlink--;
    if(node.nlink==0){
        // 3.1 修改imap
        imap[node.ino/8] &= ~(1 << (node.ino%8));
        bios_sd_write(kva2pa(imap), INODE_MAP_SEC_NUM, FS_START_SEC + INODE_MAP_OFFSET);
        // 3.2 删除结点内容
        inode_t *node_ptr = ino2inode(node.ino);
        bzero(node_ptr, sizeof(inode_t));
        int offset = node.ino / IPSEC;
        bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
        // 3.3 回收数据块
        // 3.3.1.直接索引
        for(int i=0; i<NDIRECT; i++){
            if(node.direct_addrs[i]!=0){
                // 清空数据块
                bzero(buffer, BLOCK_SIZE);
                bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, node.direct_addrs[i]);
                // 修改bmap
                int bno = (node.direct_addrs[i] - FS_START_SEC - DATA_BLOCK_OFFSET) *SECTOR_SIZE / BLOCK_SIZE;
                bmap[bno/8] &= ~(1 << (bno%8));
                bios_sd_write(kva2pa(bmap), BLOCK_MAP_SEC_NUM, FS_START_SEC + BLOCK_MAP_OFFSET);
            }
        }
        // 3.3.2.一级索引
        for(int i=0; i<3; i++){
            if(node.indirect_addrs_1st[i]!=0){
                recycle_level_index(node.indirect_addrs_1st[i], 1);
            }
        }
        // 3.3.3.二级索引
        for(int i=0; i<2; i++){
            if(node.indirect_addrs_2nd[i]!=0){
                recycle_level_index(node.indirect_addrs_2nd[i], 2);
            }
        }
        // 3.3.4.三级索引
        if(node.indirect_addrs_3rd!=0){
            recycle_level_index(node.indirect_addrs_3rd, 3);
        }

    }
    else{
        // 只需写回nlink域
        int offset = node.ino/IPSEC;
        inode_t* node_ptr = ino2inode(node.ino);
        node_ptr->nlink = node.nlink;
        bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    }
    // 4. 在父目录下删除对应目录项
    bios_sd_read(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, current_inode.direct_addrs[0]);
    dentry_t* de = (dentry_t*) buffer;
    int cur;
    for(cur=0; cur<BLOCK_SIZE/sizeof(dentry_t); cur++)
        if(de[cur].ino == node.ino)
            break;
    bzero(&de[cur], sizeof(dentry_t));
    bios_sd_write(kva2pa(buffer), BLOCK_SIZE/SECTOR_SIZE, current_inode.direct_addrs[0]);
    // 5. 处理父目录inode的size域
    inode_t* node_ptr = ino2inode(current_inode.ino);
    node_ptr->size -= sizeof(dentry_t);
    int offset = current_inode.ino / IPSEC;
    bios_sd_write(kva2pa(buffer), 1, FS_START_SEC + INODE_OFFSET + offset);
    return 0;  // do_rm succeeds  
}

int do_lseek(int fd, int offset, int whence)
{
    // TODO [P6-task2]: Implement do_lseek
    if(!fs_exist()){
        printk("[MKDIR] Warning: filesystem has not been set up!\n");
        return -1;
    }
    if(fdesc_array[fd].valid==0)
        return -1;
    if(whence==SEEK_SET){
        fdesc_array[fd].write_ptr = fdesc_array[fd].read_ptr = offset;
        return offset;
    }
    else if(whence==SEEK_CUR){
        fdesc_array[fd].write_ptr += offset;
        fdesc_array[fd].read_ptr += offset;
        return fdesc_array[fd].read_ptr;    // [TODO] read和write指针不一致？
    }
    else if(whence==SEEK_END){
        inode_t node = *ino2inode(fdesc_array[fd].ino);
        fdesc_array[fd].write_ptr = fdesc_array[fd].read_ptr = node.size + offset;
        return fdesc_array[fd].write_ptr;
    }
    printk("[LSEEK] Warning: unknown mode!\n");
    return -1;  // the resulting offset location from the beginning of the file
}

