section .data
	msg: db "NASMC: %s",10,0
	nl: db 10,0

section .text
	global main
	extern printf

main:
%ifidn __OUTPUT_FORMAT__, elf32
	; esp[4]: argc
	; esp[8]: argv
	mov eax, esp[4]
	mov esi, esp[8]
%elifidn __OUTPUT_FORMAT__, elf64
	; rdi: argc
	; rsi: argv
	push rax
	mov rax, rdi
%endif
	;_if argc != 2
	cmp eax, 2
	je .continue

%ifidn __OUTPUT_FORMAT__, elf32
%elifidn __OUTPUT_FORMAT__, elf64
	pop rax
%endif
	; return(1)
	; eax: status
	mov eax, 1		; 1
	ret

.continue:
	call get_pc_thunks
get_pc_thunks:
	pop edx
	add edx, msg - get_pc_thunks
	; printf(msg, argv[1])
	; edi: fmt
	; esi: args
%ifidn __OUTPUT_FORMAT__, elf32
	mov esi, esi[4] 	; argv[1]
	push esi
	push edx
	call printf wrt ..plt
	pop eax
	pop eax
%elifidn __OUTPUT_FORMAT__, elf64
	mov rsi, rsi[8]	; argv[1]
	mov rdi, msg
	xor eax, eax
	call printf@PLT
%endif

%ifidn __OUTPUT_FORMAT__, elf32
%elifidn __OUTPUT_FORMAT__, elf64
	pop rax
%endif
	; return(0)
	; eax: status
	xor eax, eax		; 0
	ret
