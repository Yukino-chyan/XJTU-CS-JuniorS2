    .data
fmt_int: .asciz "%d\n"
fmt_in:  .asciz "%d"
    .text
    .global main
max:
    push {fp, lr}
    mov fp, sp
    sub sp, sp, #8
    str r0, [fp, #-4]
    str r1, [fp, #-8]
    ldr r0, [fp, #-4]
    ldr r1, [fp, #-8]
    cmp r0, r1
    bge L5
    ldr r0, [fp, #-8]
    b .Lmax_epi
    b L6
L5:
    ldr r0, [fp, #-4]
    b .Lmax_epi
L6:
.Lmax_epi:
    mov sp, fp
    pop {fp, lr}
    bx lr
main:
    push {fp, lr}
    mov fp, sp
    sub sp, sp, #8
    ldr r0, =5
    ldr r1, =8
    bl max
    str r0, [fp, #-8]
    ldr r0, [fp, #-8]
    str r0, [fp, #-4]
    ldr r1, [fp, #-4]
    ldr r0, =fmt_int
    bl printf
    ldr r0, =0
    b .Lmain_epi
.Lmain_epi:
    mov sp, fp
    pop {fp, lr}
    bx lr
