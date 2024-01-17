.section .text
.globl _start

_start:
    li a0, 1          # 将值 1 存储到 a0 寄存器
    la a1, target     # 将目标地址加载到 a1 寄存器
    sw a0, 0(a1)      # 存储 a0 寄存器的值到目标地址
    
    li a7, 93         # 设置系统调用号为 93（退出程序）
    ecall             # 执行系统调用

.section .data
.align 2
target:
    .space 4          # 分配 4 个字节的空间，用于存储数据
