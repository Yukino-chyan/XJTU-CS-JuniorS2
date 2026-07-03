    .data
fmt_int: .asciz "%d\n"
fmt_in:  .asciz "%d"
    .text
    .global main
main:
    push {fp, lr}
    mov fp, sp
    sub sp, sp, #8
    ldr r0, =5
    str r0, [fp, #-4]
    ldr r0, [fp, #-4]
    b .Lmain_epi
.Lmain_epi:
    mov sp, fp
    pop {fp, lr}
    bx lr
