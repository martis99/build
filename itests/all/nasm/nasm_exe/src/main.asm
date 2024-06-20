%ifidn __OUTPUT_FORMAT__, elf32
%define BS 4
%define AX eax
%define BX ebx
%define CX ecx
%define DX edx
%define SP esp
%elifidn __OUTPUT_FORMAT__, elf64
%define BS 8
%define AX rax
%define BX rbx
%define CX rcx
%define DX rdx
%define SP rsp
%endif

section .data
	msg: db "NASM: ",0
	nl: db 10,0

section .text
	global _start

_start:
	; esp[0]: argc
	; esp[BS]: argv
	mov AX, SP[0], 
	lea BX, SP[BS],

	; if argc != 2
	cmp AX, 2
	je .continue

	; exit(1)
	; eax: action
	; ebx: status
	mov AX, 1		; exit
	mov BX, 1		; 1
	int 80h

.continue:
	call get_pc_thunk
get_pc_thunk:
	pop DX
	mov CX, DX
	add CX, msg - get_pc_thunk
	; print_string(msg)
	call print_string

	; print_string(argv[1])
	mov CX, BX[BS]
	call print_string

	; print_string(nl)
	mov CX, DX
	add CX, nl - get_pc_thunk
	call print_string

	; exit(0)
	; eax: number
	; ebx: status
	mov AX, 1	; exit
	xor BX, BX	; 0
	int 80h

%ifidn __OUTPUT_FORMAT__, elf32
print_string:
	; ecx: buf

	push eax
	push ebx
	push ecx
	push edx

	mov edx, ecx
.find_null:
	cmp byte edx[0], 0
	je .end_find_null
	inc edx
	jmp .find_null
.end_find_null:
	sub edx, ecx

	; write(stdout, buf, length)
	; eax: number
	; ebx: fd
	; ecx: buf
	; edx: length
	mov eax, 4		; write
	mov ebx, 1		; stdout
	int 80h

	pop edx
	pop ecx
	pop ebx
	pop eax

	ret
%elifidn __OUTPUT_FORMAT__, elf64
print_string:
	; rcx: buf

	push rax
	push rdi
	push rsi
	push rdx

	mov rsi, rcx
	mov rdx, rsi
.find_null:
	cmp byte [rdx], 0
	je .end_find_null
	inc rdx
	jmp .find_null
.end_find_null:
	sub rdx, rsi

	; rax: number
	; rdi: fd
	; rsi: buf
	; rdx: length
	mov rax, 1		; sys_write
	mov rdi, 1		; stdout
	syscall

	pop rdx
	pop rsi
	pop rdi
	pop rax

	ret
%endif
