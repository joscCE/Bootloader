bits 16
org 0x1000

Ramdom:
    mov ah, 00h ;interrupcion para tomar el system time
    int 1AH ; [CX:DX] en ese rango esta la cantidad del ticks desde media noche
    mov ax, DX
    xor dx,dx
    mov cx, 21 ;divisor
    div cx ;el residuo entre 0-20 queda en dx
    add dx, 10 ;ahora queda entre 10-30

mov dl, dl ;columna
mov dh, dl ;row

mov si, message ;un pointer al mensaje

set_cursor:
    mov bh, 0 ;display page
    mov ah, 02h ;poner el cursor en la posicion
    int 0x10 ;interrupcion de video

print:
    mov ah, 0x0E ;la verdad nose, igual no es importante
    lodsb ;agarra un byte de message lo carga y suma 1
    cmp al, 0 ;se terminaron las letras?
    je done ;vaya a done si se terminaron las letras
    int 0x10 ;interrupcion de video
    inc dh ;incrementa el row
    inc dl; ;incrementa el column
    jmp set_cursor ;set el curson con la nueva posicion

done:
    jmp $

message db "uwu",0 ;variable guardada en memoria


