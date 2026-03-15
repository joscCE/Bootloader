bits 16
org 0x7C00

mov [BOOT_DRIVE], dl ; BIOS pone el disco en DL

mov bx, 0x1000       ; donde cargar el programa
mov dh, 1            ; cuantos sectores leer

call disk_load

jmp 0x1000           ; saltar al game

;-----------------------
disk_load:
    mov ah, 0x02     ; funcion leer sectores
    mov al, dh       ; cantidad de sectores
    mov ch, 0x00     ; cilindro
    mov dh, 0x00     ; cabeza
    mov cl, 0x02     ; sector (1 es bootloader)
    mov dl, [BOOT_DRIVE]

    int 0x13
    jc disk_error
    ret

disk_error:
    jmp $

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xAA55