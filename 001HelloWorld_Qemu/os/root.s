.section .text
.syntax unified
.code 32
.globl _start

_start:
ldr sp, =_stack_top       @ Initialize stack pointer
bl main                   @ Call main
b hang                    @ Infinite loop

hang:
b hang

.globl PUT32
PUT32:
str r1, [r0]
bx lr

.globl GET32
GET32:
ldr r0, [r0]
bx lr

.section .bss
.align 4
_stack_bottom:
.skip 0x1000              @ 4KB stack space
_stack_top:
