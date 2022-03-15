.section .rodata
msg:    .string "Hello World!\n"
    
.section .text
.global main
.type main, @function
main:
    la t0, msg
    li t1, 12
    li t2, 0
    li t5, 0x110C0000
    
loop:
    add t4, t0, t2
    lb t3, 0(t4)
    sb t3, 0(t5)
    addi t2, t2, 1
    blt t2, t1, loop

    jr ra
