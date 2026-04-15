BITS 16
ORG 0x7C00

%define STAGE2_LOAD_SEGMENT 0x0800
%define STAGE2_LOAD_OFFSET  0x0000
%define STAGE2_MAX_CHUNK    127

%include "build/stage2_sectors.inc"

start:
	cli
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00
	sti

	mov [boot_drive], dl
	mov si, loading_msg
	call print_string

	mov word [dap_segment], STAGE2_LOAD_SEGMENT
	mov word [dap_offset], STAGE2_LOAD_OFFSET
	mov word [sectors_remaining], STAGE2_SECTORS
	mov dword [dap_lba_low], 1
	mov dword [dap_lba_high], 0

read_loop:
	mov ax, [sectors_remaining]
	test ax, ax
	jz boot_stage2

	cmp ax, STAGE2_MAX_CHUNK
	jbe .chunk_ready
	mov ax, STAGE2_MAX_CHUNK
.chunk_ready:
	mov [dap_sector_count], ax
	mov si, dap_packet
	mov dl, [boot_drive]
	mov ah, 0x42
	int 0x13
	jc disk_error

	xor eax, eax
	mov ax, [dap_sector_count]
	sub [sectors_remaining], ax
	add [dap_lba_low], eax
	adc dword [dap_lba_high], 0

	mov bx, ax
	shl bx, 5
	add [dap_segment], bx
	jmp read_loop

boot_stage2:
	jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

disk_error:
	mov si, disk_error_msg
	call print_string
hang:
	cli
	hlt
	jmp hang

print_string:
	lodsb
	test al, al
	jz .done
	mov ah, 0x0E
	mov bh, 0x00
	int 0x10
	jmp print_string
.done:
	ret

boot_drive db 0
sectors_remaining dw 0
loading_msg db 'Loading stage2...', 0
disk_error_msg db 'Disk read failed.', 0

dap_packet:
	db 0x10
	db 0x00
dap_sector_count:
	dw 0
dap_offset:
	dw 0
dap_segment:
	dw 0
dap_lba_low:
	dd 0
dap_lba_high:
	dd 0

times 510 - ($ - $$) db 0
dw 0xAA55
