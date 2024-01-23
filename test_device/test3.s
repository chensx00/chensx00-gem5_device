.section .text
.globl _start

_start:
    lui a1, %hi(0x400)   
    addi a1, a1, %lo(0x400)  
    li a0, 0
    sw a0, 0x10(a1)
    li a0, 10           #addA = 10
    sw a0, 0x4(a1)
    li a0, 25           #addB = 25
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
    li a0, 9            #addA = 9
    sw a0, 0x4(a1)
    li a0, 17           #addB = 17     
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




    li a7, 93         # 设置系统调用号为 93 退出程序
    ecall             # 执行系统调用

.section .data
    