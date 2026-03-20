[bits 16]
[org 0x7C00]


cli
xor ax, ax
mov ds, ax
mov es, ax
mov ss, ax
mov sp, 0x7C00
sti

mov [BOOT_DRIVE], dl ; BIOS pone el disco en DL


mov bx, 0x1000       ; donde cargar el programa

mov ah, 0x0E     
mov al, 'D'      
int 0x10  


disk_load:
    xor ax, ax
    mov es, ax        
    mov bx, 0x1000

    mov ah, 0x02     ; funcion leer sectores
    mov al, 1        ; cantidad de sectores
    mov ch, 0x00     ; cilindro
    mov dh, 0x00     ; cabeza
    mov cl, 0x02     ; sector (1 es bootloader)
    mov dl, [BOOT_DRIVE]

    int 0x13
    jc disk_error

mov ah, 0x0E     
mov al, 'P'      
int 0x10  


jmp 0x0000:0x1000

;-----------------------

disk_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10

    mov al, ah
    int 0x10

    jmp $

BOOT_DRIVE db 0

times 510-($-$$) db 0
dw 0xAA55