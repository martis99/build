%ifidn __OUTPUT_FORMAT__, elf32
%define BS 4
%define AX eax
%define SI esi
%elifidn __OUTPUT_FORMAT__, elf64
%define BS 8
%define AX rax
%define SI rsi
%endif

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
	push edi
	push esi
	mov edi, [esp+BS*1+BS*2]
	mov esi, [esp+BS*2+BS*2]
%elifidn __OUTPUT_FORMAT__, elf64
	; rdi: argc
	; rsi: argv
%endif
	;_if argc != 2
	cmp DI, 2
	je .continue

%ifidn __OUTPUT_FORMAT__, elf32
	pop esi
	pop edi
%endif
	; return(1)
	; eax: status
	mov AX, 1		; 1
	ret

.continue:
	; printf(msg, argv[1])
	; edi: fmt
	; esi: args
	push SI
%ifidn __OUTPUT_FORMAT__, elf32
	push dword [esi+BS] 	; argv[1]
	push msg
	call printf
	pop eax
	pop eax
%elifidn __OUTPUT_FORMAT__, elf64
	mov rsi, [rsi+BS]	; argv[1]
	mov rdi, msg
	call printf
%endif
	pop SI

%ifidn __OUTPUT_FORMAT__, elf32
	pop esi
	pop edi
%endif

	; return(0)
	; eax: status
	xor AX, AX		; 0
	ret
