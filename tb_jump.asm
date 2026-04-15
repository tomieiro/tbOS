[BITS 32]

section .text
global tb_setjmp
global tb_longjmp

tb_setjmp:
	mov edx, [esp + 4]
	mov [edx + 0], ebx
	mov [edx + 4], esi
	mov [edx + 8], edi
	mov [edx + 12], ebp
	lea ecx, [esp + 4]
	mov [edx + 16], ecx
	mov ecx, [esp]
	mov [edx + 20], ecx
	xor eax, eax
	ret

tb_longjmp:
	mov edx, [esp + 4]
	mov eax, [esp + 8]
	test eax, eax
	jnz .value_ok
	mov eax, 1
.value_ok:
	mov ebx, [edx + 0]
	mov esi, [edx + 4]
	mov edi, [edx + 8]
	mov ebp, [edx + 12]
	mov esp, [edx + 16]
	jmp [edx + 20]
