#include <efi.h>

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

/* ---------------------------------
   Textos disponibles
--------------------------------- */
static CHAR16 *prompt = L"Presione enter para empezar";
static CHAR16 *name0  = L"jimmy";
static CHAR16 *name1  = L"jose";

/* ---------------------------------
   Estado global del juego
--------------------------------- */
static CHAR16 *selected_name = NULL;
static UINTN start_col = 0;
static UINTN start_row = 0;

/* Dirección actual de dibujo */
static INTN dir_col = 1;
static INTN dir_row = 0;

/* Estado del generador LCG */
static UINT32 rng_state = 1;

/* ---------------------------------
   Utilidades básicas
--------------------------------- */

/* Function: str_len16
Calcula la longitud de una cadena CHAR16 terminada en nulo.

Params:
- s: const CHAR16* - Cadena de entrada.

Returns:
- UINTN: Cantidad de caracteres de la cadena, sin incluir el terminador nulo.

Restriction:
La cadena debe terminar correctamente en L'\0'.
*/
static UINTN str_len16(const CHAR16 *s) {
    UINTN len = 0;
    while (s[len] != L'\0') { // contar hasta encontrar fin de cadena
        len++;
    }
    return len;
}

/* Function: clear_screen
Limpia completamente la pantalla de la consola UEFI.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- VOID: No retorna valor.

Restriction:
Requiere acceso válido a la consola de salida.
*/
static VOID clear_screen(EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut); 
}

/* Function: set_cursor
Mueve el cursor a una posición específica en la consola.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.
- col: UINTN - Columna destino.
- row: UINTN - Fila destino.

Returns:
- VOID: No retorna valor.

Restriction:
La posición debe estar dentro de los límites visibles de la pantalla.
*/
static VOID set_cursor(EFI_SYSTEM_TABLE *SystemTable, UINTN col, UINTN row) {
    SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, col, row);
}

/* Function: print_at
Imprime un carácter en una posición específica de la pantalla.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.
- col: UINTN - Columna donde se imprimirá el carácter.
- row: UINTN - Fila donde se imprimirá el carácter.
- ch: CHAR16 - Carácter a imprimir.

Returns:
- VOID: No retorna valor.

Restriction:
La posición indicada debe ser válida para la consola.
*/
static VOID print_at(EFI_SYSTEM_TABLE *SystemTable, UINTN col, UINTN row, CHAR16 ch) {
    CHAR16 buf[2];
    buf[0] = ch; // guardar carácter
    buf[1] = L'\0'; // terminar cadena

    set_cursor(SystemTable, col, row);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, buf); // imprimir carácter
}

/* Function: wait_key
Espera hasta que el usuario presione una tecla y retorna esa entrada.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- EFI_INPUT_KEY: Tecla capturada desde la consola.

Restriction:
Requiere que los servicios de entrada estén disponibles.
*/
static EFI_INPUT_KEY wait_key(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_INPUT_KEY key;
    UINTN index;

    while (1) {
        SystemTable->BootServices->WaitForEvent(
            1,
            &SystemTable->ConIn->WaitForKey,
            &index
        ); // esperar hasta que haya una tecla

        if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == EFI_SUCCESS) {
            return key; // devolver tecla 
        }
    }
}

/* ---------------------------------
   Generador pseudoaleatorio
--------------------------------- */

/* Function: rng_seed
Inicializa la semilla del generador pseudoaleatorio usando la hora del sistema.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- VOID: No retorna valor.

Restriction:
Si no se puede obtener la hora, utiliza una semilla fija por defecto.
*/
static VOID rng_seed(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_TIME t;

    if (SystemTable->RuntimeServices->GetTime(&t, NULL) == EFI_SUCCESS) {
        rng_state =
            ((UINT32)t.Nanosecond) ^
            ((UINT32)t.Second << 16) ^
            ((UINT32)t.Minute << 8) ^
            ((UINT32)t.Hour << 24) ^
            ((UINT32)t.Day << 4) ^
            ((UINT32)t.Month); // mezclar componentes de tiempo
    } else {
        rng_state = 12345u; // semilla de respaldo
    }

    if (rng_state == 0) {
        rng_state = 1; // evitar estado cero
    }
}

/* Function: rng_next
Genera el siguiente valor pseudoaleatorio del generador congruencial lineal.

Params:
- No recibe parámetros.

Returns:
- UINT32: Nuevo valor pseudoaleatorio.

Restriction:
Depende de que rng_state haya sido inicializado previamente.
*/
static UINT32 rng_next(void) {
    rng_state = 1664525u * rng_state + 1013904223u; // formular de LCG
    return rng_state;
}

/* Function: random_range
Genera un número pseudoaleatorio en el rango [0, max - 1].

Params:
- max: UINTN - Límite superior exclusivo.

Returns:
- UINTN: Número pseudoaleatorio dentro del rango indicado.

Restriction:
Si max es 0, retorna 0.
*/
static UINTN random_range(UINTN max) {
    if (max == 0) {
        return 0; // evitar modulo por cero
    }

    return (UINTN)((rng_next() >> 16) % max); // usar bits altos del random 
}

/* ---------------------------------
   Lógica del juego
--------------------------------- */

/* Function: random_name
Selecciona aleatoriamente uno de los nombres disponibles.

Params:
- No recibe parámetros.

Returns:
- VOID: No retorna valor.

Restriction:
Depende de que los nombres disponibles estén inicializados.
*/
static VOID random_name(void) {
    if (((rng_next() >> 16) & 1u) == 0) { // usar bits altos del random 
        selected_name = name0;
    } else {
        selected_name = name1;
    }
}

/* Function: set_random_position
Genera una posición aleatoria inicial para dibujar el nombre en pantalla.

Params:
- No recibe parámetros.

Returns:
- VOID: No retorna valor.

Restriction:
Requiere que selected_name ya haya sido asignado.
*/
static VOID set_random_position(void) {
    UINTN len = str_len16(selected_name); // largo del nombre

    UINTN max_col_start = SCREEN_COLS - len; // ultima columna segura

    if (max_col_start < 4) {
        start_col = 0; 
    } else {
        UINTN range = max_col_start - 4 + 1;
        start_col = 4 + random_range(range); // margen izquierdo minimo de 4
    }

    start_row = 4 + random_range(15); // fila random entre 4 y 18
}

/* Function: set_direction
Configura la orientación actual del texto mediante incrementos de columna y fila.

Params:
- dcol: INTN - Cambio en columna por cada carácter.
- drow: INTN - Cambio en fila por cada carácter.

Returns:
- VOID: No retorna valor.

Restriction:
La combinación de dirección debe representar un desplazamiento válido para el texto.
*/
static VOID set_direction(INTN dcol, INTN drow) {
    dir_col = dcol; // movimiento horizontal
    dir_row = drow; // movimiento vertical
}

/* Function: draw_name
Dibuja el nombre seleccionado en pantalla usando la posición inicial y la dirección actual.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- VOID: No retorna valor.

Restriction:
Requiere que selected_name, start_col y start_row hayan sido definidos previamente.
*/
static VOID draw_name(EFI_SYSTEM_TABLE *SystemTable) {
    UINTN i;
    INTN col = (INTN)start_col; // col inicial
    INTN row = (INTN)start_row; // row inicial

    clear_screen(SystemTable); // limpiar antes de redibujar

    for (i = 0; selected_name[i] != L'\0'; i++) {
        if (col >= 0 && col < (INTN)SCREEN_COLS &&
            row >= 0 && row < (INTN)SCREEN_ROWS) {
            print_at(SystemTable, (UINTN)col, (UINTN)row, selected_name[i]);
        }
        
        // avanzar segun orientacion
        col += dir_col; 
        row += dir_row;
    }
}

/* Function: show_prompt
Muestra el mensaje inicial en la pantalla.

Params:
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- VOID: No retorna valor.

Restriction:
Requiere acceso válido a la consola de salida.
*/
static VOID show_prompt(EFI_SYSTEM_TABLE *SystemTable) {
    clear_screen(SystemTable);
    set_cursor(SystemTable, 0, 0); // ir a la esquina superior izquierda
    SystemTable->ConOut->OutputString(SystemTable->ConOut, prompt); // mostrar mensaje
}

/* Function: setup_game
Inicializa una nueva ronda del juego seleccionando nombre, posición y orientación inicial.

Params:
- No recibe parámetros.

Returns:
- VOID: No retorna valor.

Restriction:
Debe ejecutarse antes de dibujar el nombre por primera vez.
*/
static VOID setup_game(void) {
    random_name(); // escoger nombre
    set_random_position(); // generar pos
    set_direction(1, 0); // orientacion inicial horizontal a la derecha
}

/* ---------------------------------
   Entrada principal UEFI
--------------------------------- */

/* Function: efi_main
Punto de entrada principal del programa UEFI. Muestra un mensaje de inicio, espera Enter,
selecciona un nombre aleatorio y una posición aleatoria, lo dibuja en pantalla y permite
cambiar su orientación mediante las flechas del teclado.

Params:
- ImageHandle: EFI_HANDLE - Identificador de la imagen UEFI cargada.
- SystemTable: EFI_SYSTEM_TABLE* - Tabla de sistema UEFI.

Returns:
- EFI_STATUS: Retorna EFI_SUCCESS al finalizar correctamente.

Restriction:
Esc termina el programa, Backspace reinicia la ejecución, y las flechas cambian la orientación del texto.
*/
EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;

    rng_seed(SystemTable); // iniciar semilla

start:
    show_prompt(SystemTable); // mostrar mensaje de bienvenida

    while (1) {
        EFI_INPUT_KEY key = wait_key(SystemTable); // esperar tecla

        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            break; // Enter inicia el juego
        }
    }

    setup_game(); 
    draw_name(SystemTable); 

    while (1) {
        EFI_INPUT_KEY key = wait_key(SystemTable); // leer tecla del juego

        if (key.ScanCode == SCAN_ESC) { // salir 
            clear_screen(SystemTable); 
            return EFI_SUCCESS;
        }

        if (key.UnicodeChar == CHAR_BACKSPACE) { // reiniciar
            goto start;
        }

        if (key.ScanCode == SCAN_LEFT) { // horizontal izquierda
            set_direction(-1, 0);
            draw_name(SystemTable);
        } else if (key.ScanCode == SCAN_RIGHT) { // horizontal derecha
            set_direction(1, 0);
            draw_name(SystemTable);
        } else if (key.ScanCode == SCAN_UP) { // verticial arriba
            set_direction(0, -1);
            draw_name(SystemTable);
        } else if (key.ScanCode == SCAN_DOWN) { // vertical abajo
            set_direction(0, 1);
            draw_name(SystemTable);
        }
    }

    return EFI_SUCCESS;
}