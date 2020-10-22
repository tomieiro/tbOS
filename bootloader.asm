	BITS 16
start:
	mov sp, 0x8000
	mov ax, 07C0h
	mov ds, ax
	mov si, start_string
	call print_string
	jmp end

;;Function to print a string
;;args: si - string to print
print_string:
	mov ah, 0eh
.repeat:
	lodsb
	cmp al, 0x0
	je .done
	int 10h
	jmp .repeat
.done:
	ret

start_string db 'Azul eh a cor do ceu !', 0x0a, 0x0d, 0x0

end:
	;;jmp start

times 510-($-$$) db 0
dw 0xaa55
