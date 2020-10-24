	BITS 16
	ORG 0

START:
	mov sp, 0x8000
	mov ax, 07C0h
	mov ds, ax
	mov ah, 0x02
	mov cl, 0x02
	mov al, 1
	mov ch, 0x00 
	mov dh, 0x00
	xor bx, bx
	mov es, bx
	mov bx, 0x8000
	int 0x13
	jmp 0x0000:0x8000

times 510-($-$$) db 0
dw 0xaa55