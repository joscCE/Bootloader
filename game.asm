bits 16
org 0x1000

;==========================
; Inicio de programa
;==========================
start:
    mov si, prompt         ; mensaje de bienvenida

; print del mensaje de bienvenida
print_string:             
    lodsb
    cmp al, 0              ; si ya termino
    je wait_enter          ; esperar tecla enter

    mov ah, 0x0E           
    int 0x10               ; interrupcion de video
    jmp print_string       

;==========================
; Tecla de inicio
;==========================
wait_enter:
    mov ah, 00h
    int 16h                 ; espera una tecla

    cmp al, 0Dh             ; tecla enter
    jne wait_enter
                            
    call clear_screen       ; limpiar ventana

;==========================
; Generar random pos
;==========================
random_pos:
    ;random columna 
    mov ah, 00h             ; interrupcion para tomar el system time
    int 1Ah                 ; [CX:DX] = ticks desde media noche
    mov ax, dx
    xor dx, dx
    mov cx, 68              ; divisor
    div cx                  ; residuo entre 0..67 queda en DX
    add dl, 6               ; ahora queda entre 6..73
    mov [start_col], dl     ; guardar columna inicial
    ;random fila
    mov ah, 00h             ; interrupcion para tomar el system time
    int 1Ah
    mov ax, dx
    xor dx, dx
    mov cx, 13              ; divisor
    div cx                  ; residuo entre 0..12 queda en DX
    add dl, 6               ; ahora queda entre 6..18
    ;guardar pos inicial
    mov dh, dl              ; fila
    mov dl, [start_col]     ; columna
    mov [start_row], dh     ; guardar fila inicial

    mov si, message         ; puntero al mensaje
    jmp print_up

;==============================
; Esperar tecla, loop principal
;==============================
wait_key:
    mov ah, 00h
    int 16h                 ; interrupcion de tecla

    cmp al, 1Bh             ; tecla esc
    je start

    cmp ah, 4Bh             ; flecha izquierda
    je print_left               

    cmp ah, 4Dh             ; flecha derecha
    je print_right

    cmp ah, 48h             ; flecha arriba
    je print_up
    
    cmp ah, 50h             ; flecha abajo
    je print_down

    jmp wait_key            ; esperar otra tecla

;==========================
; Aux para prints
;==========================
set_cursor:
    mov bh, 0               ; display page
    mov ah, 02h             ; poner el cursor en la posicion
    int 0x10                ; interrupcion de video
    ret
start_print:   
    call clear_screen       ; limpiar pantalla
    mov si, message         ; puntero al mensaje
    mov dl, [start_col]     ; cargar col inicial
    mov dh, [start_row]     ; cargar row inicial
    ret     

;==========================
; Prints
;==========================

;180 grados hacia arriba sobre el eje horizontal
print_up:
    call start_print
pu_loop:
    call set_cursor
    mov ah, 0x0E            ; la verdad nose, igual no es importante uwu
    lodsb                   ; agarra un byte de message lo carga y suma 1
    cmp al, 0               ; se terminaron las letras?
    je wait_key             ; vaya a done si se terminaron las letras
    int 0x10                ; interrupcion de video
    inc dl                  ; incrementa el col para imprimir normal
    jmp pu_loop             

;180 grados hacia abajo sobre el eje horizontal
print_down:
    call start_print
pd_loop:
    call set_cursor
    mov ah, 0x0E
    lodsb                   ; cargar el siguiente byte del mensaje
    cmp al, 0               ; si se termino el mensaje
    je wait_key             ; esperar otra tecla
    mov ah, 0x0E            ; imprimir el caracter
    int 0x10                ; int video
    dec dl                  ; decrementa el col para imprimir hacia atras
    jmp pd_loop             

;90 grados hacia derecha sobre el eje vertical
print_right:
    call start_print
pr_loop:
    call set_cursor
    mov ah, 0x0E
    lodsb                   ; cargar el siguiente byte del mensaje
    cmp al, 0               ; si se termino el mensaje
    je wait_key             ; esperar otra tecla
    mov ah, 0x0E            ; imprimir el caracter
    int 0x10                ; int video
    inc dh                  ; incrementa el row para imprimir hacia abajo
    jmp pr_loop  

;90 grados hacia izquierda sobre el eje vertical
print_left:
    call start_print
pl_loop:
    call set_cursor
    mov ah, 0x0E
    lodsb                   ; cargar el siguiente byte del mensaje
    cmp al, 0               ; si se termino el mensaje
    je wait_key             ; esperar otra tecla
    mov ah, 0x0E            ; imprimir el caracter
    int 0x10                ; int video
    dec dh                  ; decrementa el row para imprimir hacia arriba
    jmp pl_loop

;limpiar todo el screen
clear_screen:
    mov ax, 0600h           ; scroll up toda la pantalla
    mov bh, 07h             ; color de fondo y texto
    mov cx, 0000h           ; esquina superior izquierda
    mov dx, 184Fh           ; esquina inferior derecha (24 filas, 80 columnas)
    int 10h                 ; int video
    ret  

;loop infinito
done:
    jmp $


;variables guardadas en memoria
prompt db "dar enter para empezar", 0
message db "onichan",0      
start_col db 0
start_row db 0