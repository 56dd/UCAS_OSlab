#define ADDR_SUM 0x50800000

.global main

# 主程序
main:
    # 初始化寄存器
    addi t0, zero, 1      # 计数器
    li   t1, 50           # 循环次数
    li   t2, 0            # 累加结果

sum_loop:
    # 加法操作
    add  t2, t2, t0       # 累加
    addi t0, t0, 1        # 计数器加1
    blt  t0, t1, sum_loop # 循环条件
    # 将结果写入指定地址
    la   t3, ADDR_SUM     # t3 = 地址
    sd   t2, 0(t3)        # 将累加结果写入地址

end:
    j end


# 打印反汇编结果 riscv64-unknown-linux-gnu-objdump -d zhuxuyuan

