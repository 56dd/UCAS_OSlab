 #ifndef __INCLUDE_OS_FS_H__
#define __INCLUDE_OS_FS_H__

#include <type.h>

/* macros of file system */
#define SUPERBLOCK_MAGIC 0xDF4C4459
#define NUM_FDESCS 16
#define SECTOR_SIZE 512
#define BLOCK_SIZE 4096

#define CEIL_DIV(a, n)     ((a)/(n)+(((a)%(n)==0)?0:1)) // 上取整

#define INODE_NUM           512         // INODE个数
#define DATA_BLOCK_NUM      (1<<18)     // 数据块个数（4KB为单位）,共1GB
#define INODE_MAP_SEC_NUM   CEIL_DIV(INODE_NUM, SECTOR_SIZE*8)         // inode map所占sector个数
#define BLOCK_MAP_SEC_NUM   CEIL_DIV(DATA_BLOCK_NUM, SECTOR_SIZE*8)    // data block map所占sector个数
#define INODE_SEC_NUM       CEIL_DIV(sizeof(inode_t)*INODE_NUM, SECTOR_SIZE)       // inode占据的sector个数
#define DATA_BLOCK_SEC      (DATA_BLOCK_NUM/SECTOR_SIZE*BLOCK_SIZE)             // data block占据的sector个数

#define FS_SIZE  (DATA_BLOCK_OFFSET + DATA_BLOCK_SEC)      // 以sector为单位 
#define FS_START_SEC ((1<<29)/SECTOR_SIZE)        // 512MB

#define BLOCK_MAP_OFFSET    1           // 首个sector：superblock
#define INODE_MAP_OFFSET    (BLOCK_MAP_OFFSET + BLOCK_MAP_SEC_NUM)
#define INODE_OFFSET        (INODE_MAP_OFFSET + INODE_MAP_SEC_NUM)
#define DATA_BLOCK_OFFSET   (INODE_OFFSET + INODE_SEC_NUM)

#define T_DIR 0
#define T_FILE 1

#define IPSEC (SECTOR_SIZE/sizeof(inode_t))   // inode_t per sector
#define DPSEC (SECTOR_SIZE/sizeof(dentry_t))   // dentry_t per sector
#define DPBLK (BLOCK_SIZE/sizeof(dentry_t))   // dentry_t per block

#define IA_PER_BLOCK (BLOCK_SIZE/sizeof(uint32_t))
#define IA_PER_SECTOR (SECTOR_SIZE/sizeof(uint32_t))

#define DIRECT_SIZE (NDIRECT*BLOCK_SIZE)
#define INDIRECT_1ST_SIZE (3*BLOCK_SIZE*IA_PER_BLOCK)
#define INDIRECT_2ND_SIZE (2*BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK)
#define INDIRECT_3RD_SIZE (1*BLOCK_SIZE*IA_PER_BLOCK*IA_PER_BLOCK*IA_PER_BLOCK)
#define MAX_FILE_SIZE (DIRECT_SIZE + INDIRECT_1ST_SIZE + INDIRECT_2ND_SIZE + INDIRECT_3RD_SIZE)

/* data structures of file system */
typedef struct superblock {
    // TODO [P6-task1]: Implement the data structure of superblock
    uint32_t magic_number;
    uint32_t fs_start_sec;  
    uint32_t fs_size;       // Size of file system image (blocks)     
    uint32_t block_map_offset;  
    uint32_t inode_map_offset;  
    uint32_t inode_offset;
    uint32_t inode_num;
    uint32_t data_block_offset;
    uint32_t data_block_num;
} superblock_t;

typedef struct dentry {
    // TODO [P6-task1]: Implement the data structure of directory entry
    char name[16];
    int ino;
} dentry_t;

#define NDIRECT 13
typedef struct inode { 
    // TODO [P6-task1]: Implement the data structure of inode
    char type;
    char mode;
    short nlink;       // Number of links to inode in file system
    uint32_t ino;
    uint32_t ctime;
    uint32_t atime;
    uint32_t mtime;
    uint32_t size;
    uint32_t direct_addrs[NDIRECT];
    uint32_t indirect_addrs_1st[3];
    uint32_t indirect_addrs_2nd[2];
    uint32_t indirect_addrs_3rd;
} inode_t;

typedef struct fdesc {
    // TODO [P6-task2]: Implement the data structure of file descriptor
} fdesc_t;

/* modes of do_open */
#define O_RDONLY 1  /* read only open */
#define O_WRONLY 2  /* write only open */
#define O_RDWR   3  /* read/write open */

/* whence of do_lseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/* fs function declarations */
extern int do_mkfs(int force_flag);
extern int do_statfs(void);
extern int do_cd(char *path);
extern int do_mkdir(char *path);
extern int do_rmdir(char *path);
extern int do_ls(char *path, int option);
extern int do_open(char *path, int mode);
extern int do_read(int fd, char *buff, int length);
extern int do_write(int fd, char *buff, int length);
extern int do_close(int fd);
extern int do_ln(char *src_path, char *dst_path);
extern int do_rm(char *path);
extern int do_lseek(int fd, int offset, int whence);

#endif