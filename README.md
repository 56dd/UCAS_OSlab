# Pro1:

内存空间：

0x50200000：bootloader
0x50201000：kernel
0x50500000 ：kernel_stack
0x52000000 ：用户程序0
···
0x520f0000 ：用户程序15（最多16个用户程序）
0x52300000 : 用户程序info数组地址
0x52500000 ：用户程序栈

### 任务1：制作第一个引导块

```
	la a0, msg
	li a7, BIOS_PUTSTR
	jal bios_func_entry
```

调用BIOS_PUTSTR函数

### 任务2：加载和初始化内存

调用BIOS_SDREAD函数

```
	li a7, BIOS_SDREAD
	la a0, kernel
	la t0, os_size_loc
	lh a1, 0(t0)
	li a2, 1
	jal bios_func_entry
```

一个需要注意的问题是内核占了几个扇区。这个在createimage 的时候会显示，我们提供的 createimage 文件会把扇区的数目写在了头一个扇区的倒数第 4 个字节的位置(0x502001fc)，长度为 2 字节。用 lh指令可以载入两字节到寄存器。

清空BSS段，往BSS段中写入0
head.S:
```
  la t0, __bss_start
  la t1, __BSS_END__
do_clear:
  sw zero, 0(t0)
  addi t0, t0, 4
  bltu t0, t1, do_clear
```

如果此时键盘没有任何键被按下会返回-1；如果有某个键被按下则返回对应的 ASCII 码。因此，在做键盘输入相关的动作时需要自己想办法处理掉-1 的情况，避免将-1当作真正的输入直接使用，否则屏幕上会看到很奇怪的输出。
```
	//读取终端输入并回显
    int tmp;
    while(1){
        while((tmp=bios_getchar())==-1);
        bios_putchar(tmp);
    }
```

### 任务 3：加载并选择启动多个用户程序之一

fseek函数：
```
extern int fseek (FILE *__stream, long int __off, int __whence);
```

可以使用该函数定位文件指针，文件指针的位置决定你读写文件的起始位置。

参数__whence ：表示从哪里开始偏移，值有：
SEEK_SET： 文件开头
SEEK_CUR： 当前位置
SEEK_END： 文件结尾
参数__off：表示偏移的字节数，正数表示正向偏移，负数表示负向偏移。

fwrite函数：
```
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
```

ptr -- 这是指向要被写入的元素数组的指针。
size -- 这是要被写入的每个元素的大小，以字节为单位。
nmemb -- 这是元素的个数，每个元素的大小为 size 字节。
stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。

所以我们可以使用这两个函数来完成write_img_info函数，将 kernel 所占扇区数和用户程序的数目写入 image 文件的特定位置供系统启动时读取。

在文件中已包含宏 NBYTES2SEC(nbytes)，用于将字节数转换为扇区数。

```
    int nsec_kern = NBYTES2SEC(nbytes_kernel);
    fseek(img, OS_SIZE_LOC, SEEK_SET);  
    fwrite(&nsec_kern, 2, 1, img);      
    printf("Kernel size: %d sectors\n", nsec_kern);
```

我们使用这段函数将bootloader以及kernel和应用程序放在镜像的固定位置上，具体就是bootloader固定占据第一个扇区，后面kernel以及应用程序都占据15个扇区，所以我们需要使用write_padding函数来填充空闲的扇区。

```
if (strcmp(*files, "bootblock") == 0) {
            off+=1;
            write_padding(img, &phyaddr, SECTOR_SIZE);
}
else {
      off+=15;
      write_padding(img, &phyaddr, off*SECTOR_SIZE);
}
fclose(fp);
files++;
```

我们在main中使用键盘输入task_id，然后根据task_id选择启动用户程序。这里我们需要在kernel中将相应的应用程序先加载到内存当中，所以我们使用load_task_img函数来完成这个功能，然后从应用程序入口地址开始执行。

```
    int taskid;
    uint64_t entry_addr;
    void (*entry) (void);
    while(1){
        while((taskid=bios_getchar())==-1);
        bios_putchar(taskid);
        taskid -= '0';
        if(taskid>=0 && taskid<=TASK_MAXNUM){
            bios_putchar('\n');
            entry_addr = load_task_img(taskid);
            entry = (void*) entry_addr;
            entry();
        }
    }
```

load_task_img函数细节如下：
因为之前的应用程序在disk的位置已经是固定的了，所以只需要根据taskid来确定应用程序的位置即可。

```
uint64_t entry_addr;
    char info[] = "Loading task _ ...\n\r";
    for(int i=0;i<strlen(info);i++){
        if(info[i]!='_') bios_putchar(info[i]);
        else bios_putchar(taskid +'0');
    }
    entry_addr = TASK_MEM_BASE + TASK_SIZE * (taskid - 1);  
    bios_sd_read(entry_addr, 15, 1 + taskid * 15);
    return entry_addr;
```

最后我们需要完善crt0.S文件，crt0.s是每个用户程序都需要的初始化自身C语言环境功能。

至此S-core任务已完成。

### 任务4:镜像文件的紧密排列

