.section .text
.globl _start

_start:
    # 设置绝对物理地址（假设为 0x80000000）
    lui a1, %hi(0x390)   # 将常数的高 20 位加载到 t0 寄存器
    addi a1, a1, %lo(0x390)  # 将常数的低 12 位加载到 t0 寄存器
    li a0, 1          # 将值 1 存储到 a0 寄存器
    sw a0, 0x0(a1)
    sw a0, 0x10(a1)
    sw a0, 0x20(a1)
    sw a0, 0x30(a1)
    sw a0, 0x40(a1)
    sw a0, 0x50(a1)
    sw a0, 0x60(a1)
    sw a0, 0x70(a1)
    sw a0, 0x80(a1)
    sw a0, 0x90(a1)
    sw a0, 0xa0(a1)
    sw a0, 0xb0(a1)
    lw a0, 0x0(a1)
    lw a0, 0x10(a1)
    lw a0, 0x20(a1)
    lw a0, 0x30(a1)
    lw a0, 0x40(a1)
    lw a0, 0x50(a1)
    lw a0, 0x60(a1)
    lw a0, 0x70(a1)
    lw a0, 0x80(a1)
    lw a0, 0x90(a1)
    lw a0, 0xa0(a1)
    lw a0, 0xb0(a1)
    #sw a0, 0(a1)      # 存储 a0 寄存器的值到绝对物理地址

    li a7, 93         # 设置系统调用号为 93（退出程序）
    ecall             # 执行系统调用

.section .data
    # 不再需要 target，因为直接访问绝对物理地址