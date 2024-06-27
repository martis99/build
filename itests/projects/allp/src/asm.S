.data
	msg: .asciz "ASM: %s\n"

.text
	.global asm_args
	.extern printf

asm_args:
#ifdef __i386__
	# esp[4]: argc
	# esp[8]: argv
	push %edi
	push %esi
	mov 4+8(%esp), %edi
	mov 8+8(%esp), %esi
#elif defined(__x86_64__)
	# rdi: argc
	# rsi: argv
#endif

	#_if argc != 2
	cmp $2, %edi
	je .continue

#ifdef __i386__
	pop %esi
	pop %edi
#endif

	# return(1)
	# eax: status
	mov $1, %eax		# 1
	ret

.continue:
	# printf(msg, argv[1])
	# rdi: fmt
	# rsi: args
#ifdef __i386__
	push %eax
	call get_pc_thunk
	add $_GLOBAL_OFFSET_TABLE_, %eax
	lea msg@GOTOFF(%eax), %ecx
	push %esi
	push 4(%esi) 	# argv[1]
	push %ecx
	mov %eax, %ebx
	call printf@PLT
	pop %ecx
	pop %esi
	pop %esi
	pop %eax
#elif defined(__x86_64__)
	push %rsi
	mov 8(%rsi), %rsi	# argv[1]
	lea msg(%rip), %rdi
	call printf@PLT
	pop %rsi
#endif

#ifdef __i386__
	pop %esi
	pop %edi
#endif

	# return(0)
	# eax: status
	xor %eax, %eax		# 0
	ret

#ifdef __i386__
get_pc_thunk:
	mov (%esp), %eax
	ret
#endif
