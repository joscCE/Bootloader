bits 16
org 0x1000

mov dl, 23 ;columna
mov dh, 12 ;row

mov si, message

set_cursor:
    mov bh, 0 ;display page
    mov ah, 02h ;poner el cursor en la posicion
    int 0x10

print:
    mov ah, 0x0E
    lodsb
    cmp al, 0
    je done
    int 0x10
    inc dh
    jmp set_cursor

done:
    jmp $

message db "Juego cargado!",0