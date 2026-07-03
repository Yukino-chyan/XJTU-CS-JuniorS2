    .data
fmt_int: .asciz "%d\n"
fmt_in:  .asciz "%d"
    .text
    .global main
main:
    push {fp, lr}
    mov fp, sp
    sub sp, sp, #16
    ldr r0, =0
    str r0, [fp, #-4]
    ldr r0, =1
    str r0, [fp, #-8]
L4:
    ldr r0, [fp, #-8]
    ldr r1, =5
    cmp r0, r1
    bgt L10
    ldr r0, [fp, #-4]
    ldr r1, [fp, #-8]
    add r0, r0, r1
    str r0, [fp, #-12]
    ldr r0, [fp, #-12]
    str r0, [fp, #-4]
    ldr r0, [fp, #-8]
    ldr r1, =1
    add r0, r0, r1
    str r0, [fp, #-16]
    ldr r0, [fp, #-16]
    str r0, [fp, #-8]
    b L4
L10:
    ldr r1, [fp, #-4]
    ldr r0, =fmt_int
    bl printf
    ldr r0, =0
    b .Lmain_epi
.Lmain_epi:
    mov sp, fp
    pop {fp, lr}
    bx lr
