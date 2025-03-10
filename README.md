# Pro1:

内存空间：

0x50200000：bootloader
0x50201000：kernel

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