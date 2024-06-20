#ifdef __i386__
#define BS 4
#define AX %eax
#define BX %ebx
#define CX %ecx
#define SP %esp
#define PIC @GOTOFF(%edx)
#elif defined(__x86_64__)
#define BS 8
#define AX %rax
#define BX %rbx
#define CX %rcx
#define SP %rsp
#define PIC (%rip)
#endif

.data
	msg: .asciz "ASM: "
	nl: .asciz "\n"

.text
	.global _start

_start:
	# esp[0]: argc
	# esp[BS]: argv
	mov (SP), AX
	lea BS(SP), BX

	#_if argc != 2
	cmp $2, AX
	je .continue

	# exit(1)
	# eax: number
	# ebx: status
	mov $1, AX		# exit
	mov $1, BX		# 1
	int $0x80

.continue:
#ifdef __i386__
	call get_pc_thunk
	add $_GLOBAL_OFFSET_TABLE_, %edx
#endif

	# print_string(msg)
	lea msg PIC, CX
	call print_string

	# print_string(argv[1])
	mov BS(BX), CX
	call print_string

	# print_string(nl)
	lea nl PIC, CX
	call print_string

	# exit(0)
	# eax: number
	# ebx: status
	mov $1, %eax		# exit
	xor %ebx, %ebx		# 0
	int $0x80

#ifdef __i386__
print_string:
	# ecx: buf

	push %eax
	push %ebx
	push %ecx
	push %edx

	mov %ecx, %edx
.find_null:
	cmpb $0, (%edx)
	je .end_find_null
	inc %edx
	jmp .find_null
.end_find_null:
	sub %ecx, %edx

	# write(stdout, buf, length)
	# eax: number
	# ebx: fd
	# ecx: buf
	# edx: length
	mov $4, %eax		# write
	mov $1, %ebx		# stdout
	int $0x80

	pop %edx
	pop %ecx
	pop %ebx
	pop %eax

	ret
#elif defined(__x86_64__)
print_string:
	# rcx: buf

	push %rax
	push %rdi
	push %rsi
	push %rdx

	mov %rcx, %rsi
	mov %rsi, %rdx
.find_null:
	cmpb $0, (%rdx)
	je .end_find_null
	inc %rdx
	jmp .find_null
.end_find_null:
	sub %rsi, %rdx

	# syscall(sys_write, stdout, buf, length)
	# rax: number
	# rdi: fd
	# rsi: buf
	# rdx: length
	mov $1, %rax		# sys_write
	mov $1, %rdi		# stdout
	syscall

	pop %rdx
	pop %rsi
	pop %rdi
	pop %rax

	ret
#endif

#ifdef __i386__
get_pc_thunk:
	mov (%esp), %edx
	ret
#endif
