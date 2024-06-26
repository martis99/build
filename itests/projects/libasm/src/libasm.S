.data
#ifdef LIBASM_BUILD_DLL
	msg: .asciz "DLIBASM: %s\n"
#else
	msg: .asciz "LIBASM: %s\n"
#endif

.text
#ifdef LIBASM_BUILD_DLL
	.global dlibasm_args
#else
	.global libasm_args
#endif
	.extern printf

#ifdef LIBASM_BUILD_DLL
dlibasm_args:
#else
libasm_args:
#endif
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
	push %ecx
	push %ebx
	call get_pc_thunk
	add $_GLOBAL_OFFSET_TABLE_, %eax
	lea msg@GOTOFF(%eax), %ecx
	push %esi
	mov 4(%esi), %esi 
	push %esi 	# argv[1]
	push %ecx
	mov %eax, %ebx
	call printf@PLT
	pop %ecx
	pop %esi
	pop %esi
	pop %ebx
	pop %ecx
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
