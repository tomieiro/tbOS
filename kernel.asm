BITS 16

section .text         
global KERNEL

KERNEL:
	mov sp, 0x8000
	mov ax, 07C0h
	mov ds, ax
    mov ax, 0x0E52
    int 0x10
    mov ax, 0x0E69
    int 0x10
    mov ax, 0x0E67
    int 0x10
    mov ax, 0x0E61
    int 0x10
    mov ax, 0x0E74
    int 0x10
    mov ax, 0x0E6f
    int 0x10
    cli
    hlt

;times ((0x400) - ($ - $$)) db 0x00