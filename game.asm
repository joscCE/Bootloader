bits 16
org 0x1000

start:
    mov si, prompt         ; mensaje de bienvenida

print_string:              ; print del mensaje de bienvenida
    lodsb
    cmp al, 0              ; si ya termino
    je wait_enter          ; esperar tecla enter

    mov ah, 0x0E           
    int 0x10               ; interrupcion de video
    jmp print_string       

wait_enter:
    mov ah, 00h
    int 16h                 ; espera una tecla

    cmp al, 0Dh             ; tecla enter
    jne wait_enter

    call clear_screen       ; limpiar ventana

random_pos:

    ; random para columna: 6..73
    mov ah, 00h             ;interrupcion para tomar el system time
    int 1AH                 ;[CX:DX] en ese rango esta la cantidad del ticks desde media noche
    
    mov ax, dx              ;dx como semilla
    xor dx, dx
    mov cx, 68              ;0..67 
    div cx                  ;residuo

    xchg ax, dx             ;mover residuo a ax
    mov dl, al              ;usar al como columna base
    add dl, 6               ;ajustar rango 6..73

    mov [start_col], dl     ;guardar col inicial

    ; random para fila: 6..18
    mov ah, 00h          
    int 1AH

    mov ax, dx
    xor dx, dx
    mov cx, 13              ;0..12
    div cx                  ;residuo

    xchg ax, dx             
    mov dh, al              ;usar al como fila base
    add dh, 6               ;ajustar rango 6..18

    mov [start_row], dh     ;guardar row inicial

    mov si, message         ;un pointer al mensaje
    jmp set_cursor

set_cursor:
    mov bh, 0               ;display page
    mov ah, 02h             ;poner el cursor en la posicion
    int 0x10                ;interrupcion de video

print_horizontal:
    mov ah, 0x0E            ;la verdad nose, igual no es importante uwu
    lodsb                   ;agarra un byte de message lo carga y suma 1
    cmp al, 0               ;se terminaron las letras?
    je wait_key             ;vaya a done si se terminaron las letras
    int 0x10                ;interrupcion de video
    ;inc dh                 ;incrementa el row (comentado por ahora para q sea horizontal)
    inc dl                  ;incrementa el column
    jmp set_cursor          ;set el curson con la nueva posicion


; esperar tecla
wait_key:
    mov ah, 00h
    int 16h                 ;interrupcion de tecla

    cmp al, 1Bh             ;tecla esc
    je done

    cmp ah, 4Bh             ;flecha izquierda: rota 90 grados hacia izquierda sobre eje y
    ;je print_right               

    cmp ah, 4Dh             ;flecha derecha: rota 90 grados hacia derecha sobre eje y
    je print_right

    cmp ah, 48h             ;flecha arriba: rota 180 grados hacia abajo sobre eje x
    ;je print
    
    cmp ah, 50h             ;flecha abajo: rota 180 grados hacia arriba sobre eje c
    ;je print

    jmp wait_key

print_right:
    call clear_screen       ;limpiar todo
    mov si, message         ;puntero al mensaje
    mov dl, [start_col]     ;cargar col inicial
    mov dh, [start_row]     ;cargar row inicial

pr_loop:
    mov bh, 0               ;display page
    mov ah, 02h             ;cursor en pos
    int 0x10                ;int video

    lodsb                   ;cargar el siguiente byte del mensaje
    cmp al, 0               ;si se termino el mensaje
    je wait_key             ;esperar otra tecla

    mov ah, 0Eh             ;imprimir el caracter
    int 0x10                ;int video

    inc dh                  ;incrementa el row para imprimir hacia abajo
    jmp pr_loop            

done:
    jmp $


clear_screen:
    mov ax, 0600h           ;scroll up toda la pantalla
    mov bh, 07h             ;color de fondo y texto
    mov cx, 0000h           ;esquina superior izquierda
    mov dx, 184Fh           ;esquina inferior derecha (24 filas, 80 columnas)
    int 10h                 ;int video
    ret

;variables guardadas en memoria
prompt    db "dar enter para empezar", 0
message db "onichan",0          
start_col db 0
start_row db 0
step_col  db 0
step_row  db 0