#define STACK_ADDRESS 0x60000000
.global main

main:
    # 初始化寄存器
    addi t0, zero, 1       # 计数器
    li   t1, 200           # 循环次数
    la   sp, STACK_ADDRESS # 设置栈顶

loop:
    addi a0, t0, 0          # 参数传递
    addi sp,sp,-16
    sd   ra,8(sp)
    sw   t0,4(sp)
    sw   t1,0(sp)
    call judge
    lw   t1,0(sp)
    lw   t0,4(sp)
    ld   ra,8(sp)
    addi sp,sp,16
    addi t0, t0, 1        # 计数器加1
    blt  t0, t1, loop # 循环条件
    j end

judge:
    #判断数字是否为质数
    addi sp,sp,-8
    sd ra,0(sp)
    # 判断是否为质数
    # 判断是否为1（1不是质数）
    addi t0,zero,1
    beq  a0,t0,judge_is_not_prime
    # 判断是否为2（2是质数）
    addi t0,zero,2
    beq  a0,t0,judge_is_prime
    # 判断是否为3（3是质数）
    addi t0,zero,3
    beq  a0,t0,judge_is_prime
    addi s0,a0,0
    call sqrt
    addi t0,zero,1
judge_loop:
    addi t0,t0,1
    rem  t1,s0,t0
    beq  t1,zero,judge_is_not_prime
    ble  t0,a0,judge_loop
    j judge_is_prime
judge_is_prime:
    addi a0,zero,1
    j judge_end
judge_is_not_prime:
    addi a0,zero,0
    j judge_end
judge_end:
    ld  ra,0(sp)
    addi sp,sp,8
    ret

sqrt:
    addi sp,sp,-8
    sd ra,0(sp)
    # 计算近似平方根
    addi t0,zero,0
sqrt_loop:
    addi t0,t0,1
    mul  t1,t0,t0
    blt  t1,a0,sqrt_loop
sqrt_end:
    addi a0,t0,0
    ld   ra,0(sp)
    addi sp,sp,8
    ret



end:
    j end

# 打印反汇编结果 riscv64-unknown-linux-gnu-objdump -d zhuxuyuan


