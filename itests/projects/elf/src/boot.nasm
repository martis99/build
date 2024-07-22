mov ah, 0x0e
mov al, 'H'
int 0x10
mov al, 'e'
int 0x10
mov al, 'l'
int 0x10
mov al, 'l'
int 0x10
mov al, 'o'
int 0x10
mov al, 13
int 0x10
mov al, 10
int 0x10
mov al, 0
int 0x10

mov dx, 0xf4
mov al, 0x00
out dx, al

mov al, '.'
int 0x10

times 510 - ($-$$) db 0
dw 0xaa55
