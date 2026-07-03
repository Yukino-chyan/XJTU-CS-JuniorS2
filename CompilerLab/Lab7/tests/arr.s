    .data
fmt_int: .asciz "%d\n"
fmt_in:  .asciz "%d"
    .text
    .global main
main:
    push {fp, lr}
    mov fp, sp
    sub sp, sp, #16
    sub r1, fp, #12
    ldr r2, =0
    add r1, r1, r2, lsl #2
    ldr r0, =10
    str r0, [r1]
    sub r1, fp, #12
    ldr r2, =1
    add r1, r1, r2, lsl #2
    ldr r0, =20
    str r0, [r1]
    sub r1, fp, #12
    ldr r2, =2
    add r1, r1, r2, lsl #2
    ldr r0, =30
    str r0, [r1]
    sub r1, fp, #12
    ldr r0, =2
    add r1, r1, r0, lsl #2
    ldr r0, [r1]
    str r0, [fp, #-16]
    ldr r1, [fp, #-16]
    ldr r0, =fmt_int
    bl printf
    ldr r0, =0
    b .Lmain_epi
.Lmain_epi:
    mov sp, fp
    pop {fp, lr}
    bx lr
