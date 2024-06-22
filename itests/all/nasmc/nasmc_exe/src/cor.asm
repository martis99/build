section .data
    msg db "ASM", 10, 0  ; "ASM\n"

section .text
    extern printf

global main

main:
    call get_pc_thunk
    mov edx, eax  ; Move the result of get_pc_thunk into edx

    lea ecx, [msg - $$ + edx]  ; Calculate the address of msg relative to edx
    push ecx                    ; Push address of msg
    call printf                 ; Call printf
    add esp, 4                  ; Clean up stack after call

    xor eax, eax  ; Set return value to 0
    ret           ; Return from main

get_pc_thunk:
    mov eax, [esp]  ; Move return address (top of stack) into eax
    ret             ; Return with eax containing return address