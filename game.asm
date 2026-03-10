bits 16
org 0x1000

mov si, message

print:
    mov ah, 0x0E
    lodsb
    cmp al, 0
    je done
    int 0x10
    jmp print

done:
    jmp $

message db "Juego cargado!",0