.section .data
.align 3
a: .skip 80
.section .text
.global _start
swap:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #112
    str x0, [sp, #0]
    str x1, [sp, #8]
    str x2, [sp, #16]
    ldr x9, [sp, #8]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #24]
    ldr x9, [sp, #0]
    ldr x10, [sp, #24]
    add x9, x9, x10
    str x9, [sp, #32]
    ldr x9, [sp, #32]
    ldr x10, [x9]
    str x10, [sp, #40]
    ldr x9, [sp, #8]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #48]
    ldr x9, [sp, #0]
    ldr x10, [sp, #48]
    add x9, x9, x10
    str x9, [sp, #56]
    ldr x9, [sp, #16]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #64]
    ldr x9, [sp, #0]
    ldr x10, [sp, #64]
    add x9, x9, x10
    str x9, [sp, #72]
    ldr x9, [sp, #72]
    ldr x10, [x9]
    str x10, [sp, #80]
    ldr x9, [sp, #56]
    ldr x10, [sp, #80]
    str x10, [x9]
    ldr x9, [sp, #16]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #88]
    ldr x9, [sp, #0]
    ldr x10, [sp, #88]
    add x9, x9, x10
    str x9, [sp, #96]
    ldr x9, [sp, #96]
    ldr x10, [sp, #40]
    str x10, [x9]
    mov x0, #0
    b swap_exit
swap_exit:
    add sp, sp, #112
    ldp x29, x30, [sp], #16
    ret
partition:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #160
    str x0, [sp, #0]
    str x1, [sp, #8]
    str x2, [sp, #16]
    ldr x9, [sp, #16]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #24]
    ldr x9, [sp, #0]
    ldr x10, [sp, #24]
    add x9, x9, x10
    str x9, [sp, #32]
    ldr x9, [sp, #32]
    ldr x10, [x9]
    str x10, [sp, #40]
    ldr x9, [sp, #8]
    mov x10, #1
    sub x9, x9, x10
    str x9, [sp, #48]
    ldr x9, [sp, #48]
    str x9, [sp, #56]
    ldr x9, [sp, #8]
    mov x10, #1
    sub x9, x9, x10
    str x9, [sp, #64]
    ldr x9, [sp, #64]
    str x9, [sp, #72]
l0:
    ldr x9, [sp, #72]
    mov x10, #1
    add x9, x9, x10
    str x9, [sp, #80]
    ldr x9, [sp, #80]
    str x9, [sp, #72]
    ldr x9, [sp, #80]
    ldr x10, [sp, #16]
    cmp x9, x10
    b.lt l1
    b l2
l1:
    ldr x9, [sp, #72]
    mov x10, #8
    mul x9, x9, x10
    str x9, [sp, #88]
    ldr x9, [sp, #0]
    ldr x10, [sp, #88]
    add x9, x9, x10
    str x9, [sp, #96]
    ldr x9, [sp, #96]
    ldr x10, [x9]
    str x10, [sp, #104]
    ldr x9, [sp, #104]
    ldr x10, [sp, #40]
    cmp x9, x10
    b.lt l3
    b l4
l3:
    ldr x9, [sp, #56]
    mov x10, #1
    add x9, x9, x10
    str x9, [sp, #112]
    ldr x9, [sp, #112]
    str x9, [sp, #56]
    ldr x0, [sp, #0]
    ldr x1, [sp, #56]
    ldr x2, [sp, #72]
    bl swap
    str x0, [sp, #120]
l4:
    b l0
l2:
    ldr x9, [sp, #56]
    mov x10, #1
    add x9, x9, x10
    str x9, [sp, #128]
    ldr x0, [sp, #0]
    ldr x1, [sp, #128]
    ldr x2, [sp, #16]
    bl swap
    str x0, [sp, #136]
    ldr x9, [sp, #56]
    mov x10, #1
    add x9, x9, x10
    str x9, [sp, #144]
    ldr x0, [sp, #144]
    b partition_exit
partition_exit:
    add sp, sp, #160
    ldp x29, x30, [sp], #16
    ret
qsort:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #64
    str x0, [sp, #0]
    str x1, [sp, #8]
    str x2, [sp, #16]
    ldr x9, [sp, #8]
    ldr x10, [sp, #16]
    cmp x9, x10
    b.lt l5
    b l6
l5:
    ldr x0, [sp, #0]
    ldr x1, [sp, #8]
    ldr x2, [sp, #16]
    bl partition
    str x0, [sp, #24]
    ldr x9, [sp, #24]
    mov x10, #1
    sub x9, x9, x10
    str x9, [sp, #32]
    ldr x0, [sp, #0]
    ldr x1, [sp, #8]
    ldr x2, [sp, #32]
    bl qsort
    str x0, [sp, #40]
    ldr x9, [sp, #24]
    mov x10, #1
    add x9, x9, x10
    str x9, [sp, #48]
    ldr x0, [sp, #0]
    ldr x1, [sp, #48]
    ldr x2, [sp, #16]
    bl qsort
    str x0, [sp, #56]
l6:
    mov x0, #0
    b qsort_exit
qsort_exit:
    add sp, sp, #64
    ldp x29, x30, [sp], #16
    ret
_start:
    sub sp, sp, #16
    adr x0, a
    mov x1, #0
    mov x2, #9
    bl qsort
    str x0, [sp, #0]
    mov x0, #0
    mov x8, #93
    svc #0
