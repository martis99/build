.data
	msg: .asciz "ASM\n"

.text
	.global main
	.extern printf

main:
	call get_pc_thunk
	add $_GLOBAL_OFFSET_TABLE_, %edx

	lea msg@GOTOFF(%edx), %ecx
	push %ecx
	mov %edx, %ebx
	call printf@PLT
	pop %ecx

	xor %eax, %eax
	ret

get_pc_thunk:
	mov (%esp), %edx
	ret
