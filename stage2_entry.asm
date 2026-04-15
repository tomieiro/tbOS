[BITS 16]

section .text
global stage2_start
extern stage2_main

CODE_SEG equ 0x08
DATA_SEG equ 0x10

stage2_start:
	cli
	lgdt [gdt_descriptor]

	mov eax, cr0
	or eax, 0x1
	mov cr0, eax
	jmp CODE_SEG:protected_mode_entry

[BITS 32]
protected_mode_entry:
	mov ax, DATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov esp, stack_top

	call stage2_main

.halt:
	cli
	hlt
	jmp .halt

align 8
gdt_start:
	dq 0x0000000000000000
	dq 0x00CF9A000000FFFF
	dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
	dw gdt_end - gdt_start - 1
	dd gdt_start

section .bss
align 16
stack_bottom:
	resb 4096
stack_top:
