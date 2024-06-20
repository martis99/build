#ifdef __i386__
#define AX %eax
#define BS 4
#define SI %esi
#define PIC @GOTOFF(%edx)
#elif defined(__x86_64__)
#define BS 8
#define AX %rax
#define SI %rsi
#define PIC (%rip)
#endif

.data
	msg: .asciz "ASMC: %s\n"

.text
	.global main
	.extern printf

main:
#ifdef __i386__
	# esp[4]: argc
	# esp[8]: argv
	push %edi
	push %esi
	mov BS*1+8(%esp), %edi
	lea BS*2+8(%esp), %esi
#elif defined(__x86_64__)
	# rdi: argc
	# rsi: argv
#endif

	#_if argc != 2
	cmp $2, %edi
	je .continue

	# return(1)
	# eax: status
	mov $1, AX		# 1
	ret

.continue:
#ifdef __i386__
	push %edx
	call get_pc_thunk
	add $_GLOBAL_OFFSET_TABLE_, %edx
#endif

	# printf(msg, argv[1])
	# rdi: fmt
	# rsi: args
	push SI
#ifdef __i386__
	push BS(%esi) 	# argv[1]
	lea msg PIC, %ecx
	push %ecx
	mov %edx, %ebx
	call printf@PLT
	pop %ecx
	pop %esi
#elif defined(__x86_64__)
	mov BS(%rsi), %rsi	# argv[1]
	lea msg PIC, %rdi
	call printf@PLT
#endif
	pop SI

#ifdef __i386__
	pop %edx
	pop %esi
	pop %edi
#endif

	# return(0)
	# eax: status
	xor AX, AX		# 0
	ret

#ifdef __i386__
get_pc_thunk:
	mov (%esp), %edx
	ret
#endif
