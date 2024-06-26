jmp short main
nop

bdb_oem:                   db "MSWIN4.1"    ; 8 bytes
bdb_bytes_per_sector:      dw 512
bdb_sectors_per_cluster:   db 1
bdb_reserved_sectors:      dw 1
bdb_fat_count:             db 2
bdb_dir_entries_count:     dw 0xE0
bdb_total_sectors:         dw 2880          ; 2880 * 512 = 1.44 MB
bdb_media_descriptor_type: db 0XF0          ; F0 = 3.5 inch disk
bdb_sectors_per_fat:       dw 9
bdb_sectors_per_track:     dw 18
bdb_heads:                 dw 2
bdb_hidden_sectors:        dd 0
bdb_large_sector_count:    dd 0

; extended boot record
ebr_drive_number:          db 0             ; 0x00 = floppy, 0x80 = hdd
                           db 0             ; reserved
ebr_signature:             db 0x29
ebr_volume_id:             db 0x12, 0x34, 0x56, 0x78
ebr_volume_label:          db "OSFS VOLUME" ; 11 bytes
ebr_system_id:             db "FAT12   "    ; 8 bytes

section .text
	global main

main:
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

times 510 - ($-$$) db 0
dw 0xaa55
