section .data
    msg db "NASM", 10, 0

section .text
    global main
    extern printf

main:
    push ebp
    mov ebp, esp

    ; Call get_pc_thunk to get current PC
    call get_pc_thunk
    add edx, msg wrt ..got

    ; Load address of msg into ecx
    mov ecx, dword [edx]

    ; Call printf
    push ecx
    call printf wrt ..plt
    add esp, 4

    mov esp, ebp
    pop ebp

    xor eax, eax
    ret

get_pc_thunk:
    mov edx, [esp]
    ret