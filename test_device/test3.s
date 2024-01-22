.section .text
.globl _start

_start:
    # 设置绝对物理地址（假设为 0x80000000）
    lui a1, %hi(0x400)   # 将常数的高 20 位加载到 t0 寄存器
    addi a1, a1, %lo(0x400)  # 将常数的低 12 位加载到 t0 寄存器
    li a0, 0
    sw a0, 0x10(a1)
    li a0, 10          # 将值 1 存储到 a0 寄存器
    sw a0, 0x4(a1)
    li a0, 25
    sw a0, 0x8(a1)
    li a0, 0xbb
    sw a0, 0x0(a1)

    loop:
        lw a0, 0x10(a1)
        addi t0, x0,0xaa
        addi a2, x0,0x000000ff
        and a0, a2, a0
        bne a0, t0, loop


    lw t1, 0xc(a1)


    li a0, 0
    sw a0, 0x10(a1)
    li a0, 9          # 将值 1 存储到 a0 寄存器
    sw a0, 0x4(a1)
    li a0, 17
    sw a0, 0x8(a1)
    li a0, 0xbb
    sw a0, 0x00(a1)

    loop2:
        lw a0, 0x10(a1)
        addi t0, x0,0xaa
        addi a2, x0,0x000000ff
        and a0, a2, a0
        bne a0, t0, loop2

    lw t1, 0xc(a1)




    li a7, 93         # 设置系统调用号为 93（退出程序）
    ecall             # 执行系统调用

.section .data
    # 不再需要 target，因为直接访问绝对物理地址