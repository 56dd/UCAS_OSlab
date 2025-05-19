/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>

#define SHELL_BEGIN 20
#define MAX_INS_LENGTH 100
#define MAX_ARG_LEN 20
#define MAX_ARG_NUM 10

char buff[MAX_INS_LENGTH];

int ins_pos = 0;
int end;
int argc;
char argv[MAX_ARG_NUM][MAX_ARG_LEN];

int parse_args(const char *buff);

int main(void)
{
    sys_move_cursor(0, SHELL_BEGIN);
    printf("------------------- COMMAND -------------------\n");
    printf("> root@UCAS_OS: ");
    int tmp;

    while (1)
    {   
        end = 0;
        // TODO [P3-task1]: call syscall to read UART port
        while((tmp = sys_getchar())==-1);
        // TODO [P3-task1]: parse input
        // note: backspace maybe 8('\b') or 127(delete)
        if (tmp == '\b' || tmp == 127)
        {
            if (ins_pos > 0)
            {
                sys_write_ch(tmp);
                sys_reflush();               
                buff[--ins_pos] = '\0';
            }
        }
        else if (tmp == '\n' || tmp == '\r'){
           sys_write_ch('\n');
           sys_reflush();
           end = 1; 
        }
        else{
            sys_write_ch(tmp);
            sys_reflush();
            buff[ins_pos++] = tmp;
        }

        if(end ==0)
            continue;
        // TODO [P3-task1]: ps, exec, kill, clear    
        else{
            buff[ins_pos] = '\0';
        }
        ins_pos = 0;
        argc = parse_args(buff);
        if(strcmp("ps", argv[0])==0 && argc==1){
            sys_ps();
        }
        else if(strcmp("exec", argv[0])==0){
            int do_wait;
            do_wait = strcmp(argv[argc-1], "&"); 
            int exec_argc;
            exec_argc = argc - 1 - (do_wait?0:1);
            char* exec_argv[MAX_ARG_LEN];
            for(int i=1;i<argc - (do_wait?0:1);i++){
                exec_argv[i-1]=argv[i];
            }
            pid_t pid = sys_exec(argv[1], exec_argc, exec_argv);
            if(pid==0){
                printf("Error: exec failed!\n");
            }
            else{
                printf("Info: excute %s successfully, pid = %d\n", argv[1], pid);
                if(do_wait)
                    sys_waitpid(pid);
            }
        }
        else if(strcmp("kill", argv[0])==0){
            int pid = atoi(argv[1]);
            if(sys_kill(pid)==0)
                printf("Info: Cannot find process with pid %d!\n", pid);
            else
                printf("Info: kill process %d successfully.\n", pid);
        }
        else if(strcmp("clear", argv[0])==0 && argc==1){
            sys_clear();
            sys_move_cursor(0, SHELL_BEGIN);
            printf("------------------- COMMAND -------------------\n");
        }
        else if(strcmp("wait", argv[0])==0){
            int pid = atoi(argv[1]);
            if(sys_waitpid(pid)==0)
                printf("Info: Cannot find process with pid %d!\n", pid);
            else
                printf("Info: Excute waitpid successfully, pid = %d.\n", pid);
        }
        else if(strcmp("taskset", argv[0])==0){
            int mode_p=0, pid=0, mask=0;
            if(argc == 3){
                mask = atoi(argv[1]);
                pid = sys_taskset(mode_p, mask, (void*)argv[2]);
                printf("Info: Excute taskset successfully, pid = %d.\n", pid);
                
            }
            else if(argc == 4){
                mode_p = 1;
                mask = atoi(argv[2]);
                pid = atoi(argv[3]);
                sys_taskset(mode_p, mask, (void*)pid);
                printf("Info: Excute taskset successfully.\n");
            }
            else{
                printf("Error: taskset command format error!");
            }
        }
        else if(strcmp("memory", argv[0])==0){
            int usepage = sys_usepage();
            printf("Use %d Pages/n",usepage);
        }
        else{
            printf("Error: Unknown command %s\n", buff);
        }

        printf("> root@UCAS_OS: ");
        /************************************************************/
        /* Do not touch this comment. Reserved for future projects. */
        /************************************************************/     
    }

    return 0;
}

int parse_args(const char *buff) {
    int argc = 0;
    int i = 0;
    for(int j=0;j<MAX_ARG_NUM;j++){
        argv[j][0] = '\0';
    }
    while (*buff) {
        // 跳过前导空白字符
        while (isspace(*buff)) buff++;

        if (*buff == '\0') break;

        // 如果参数数量已达上限，停止解析
        if (argc >= MAX_ARG_NUM) break;

        // 提取一个参数
        i = 0;
        while (*buff && !isspace(*buff) && i < MAX_ARG_LEN - 1) {
            argv[argc][i++] = *buff++;
        }
        argv[argc][i] = '\0';  // 添加字符串结尾符
        argc++;
    }
    return argc;
}


