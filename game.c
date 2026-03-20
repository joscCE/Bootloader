#include <efi.h>

#define SCREEN_COLS 80
#define SCREEN_ROWS 25

static CHAR16 *prompt = L"Presione enter para empezar";
static CHAR16 *name0  = L"jimmy";
static CHAR16 *name1  = L"jose";

static CHAR16 *selected_name = NULL;
static UINTN start_col = 0;
static UINTN start_row = 0;

/* Estado del generador LCG */
static UINT32 rng_state = 1;

/* ---------------------------------
   Utilidades básicas
--------------------------------- */

static UINTN str_len16(const CHAR16 *s) {
    UINTN len = 0;
    while (s[len] != L'\0') {
        len++;
    }
    return len;
}

static VOID clear_screen(EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
}

static VOID set_cursor(EFI_SYSTEM_TABLE *SystemTable, UINTN col, UINTN row) {
    SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, col, row);
}

static VOID print_at(EFI_SYSTEM_TABLE *SystemTable, UINTN col, UINTN row, CHAR16 ch) {
    CHAR16 buf[2];
    buf[0] = ch;
    buf[1] = L'\0';

    set_cursor(SystemTable, col, row);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, buf);
}

static EFI_INPUT_KEY wait_key(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_INPUT_KEY key;
    UINTN index;

    while (1) {
        SystemTable->BootServices->WaitForEvent(
            1,
            &SystemTable->ConIn->WaitForKey,
            &index
        );

        if (SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) == EFI_SUCCESS) {
            return key;
        }
    }
}

/* ---------------------------------
   Pseudo random number generator
   Linear Congruential Generator
--------------------------------- */

static VOID rng_seed(EFI_SYSTEM_TABLE *SystemTable) {
    EFI_TIME t;

    if (SystemTable->RuntimeServices->GetTime(&t, NULL) == EFI_SUCCESS) {
        rng_state =
            ((UINT32)t.Nanosecond) ^
            ((UINT32)t.Second << 16) ^
            ((UINT32)t.Minute << 8) ^
            ((UINT32)t.Hour << 24) ^
            ((UINT32)t.Day << 4) ^
            ((UINT32)t.Month);
    } else {
        rng_state = 12345u;
    }

    if (rng_state == 0) {
        rng_state = 1;
    }
}

static UINT32 rng_next(void) {
    rng_state = 1664525u * rng_state + 1013904223u;
    return rng_state;
}

static UINTN random_range(UINTN max) {
    if (max == 0) {
        return 0;
    }

    return (UINTN)(rng_next() % max);
}

/* ---------------------------------
   Lógica del juego
--------------------------------- */

static VOID random_name(void) {
    if (random_range(2) == 0) {
        selected_name = name0;
    } else {
        selected_name = name1;
    }
}

static VOID random_pos(void) {
    UINTN len = str_len16(selected_name);

    UINTN max_col_start = SCREEN_COLS - len;

    if (max_col_start < 4) {
        start_col = 0;
    } else {
        UINTN range = max_col_start - 4 + 1;
        start_col = 4 + random_range(range);
    }

    start_row = 4 + random_range(15);
}

static VOID start_print(EFI_SYSTEM_TABLE *SystemTable) {
    clear_screen(SystemTable);
}

static VOID print_up(EFI_SYSTEM_TABLE *SystemTable) {
    start_print(SystemTable);

    UINTN col = start_col;
    UINTN row = start_row;

    for (UINTN i = 0; selected_name[i] != L'\0'; i++) {
        print_at(SystemTable, col, row, selected_name[i]);
        col++;
    }
}

static VOID print_down(EFI_SYSTEM_TABLE *SystemTable) {
    start_print(SystemTable);

    UINTN col = start_col;
    UINTN row = start_row;

    for (UINTN i = 0; selected_name[i] != L'\0'; i++) {
        print_at(SystemTable, col, row, selected_name[i]);
        if (col > 0) {
            col--;
        }
    }
}

static VOID print_right(EFI_SYSTEM_TABLE *SystemTable) {
    start_print(SystemTable);

    UINTN col = start_col;
    UINTN row = start_row;

    for (UINTN i = 0; selected_name[i] != L'\0'; i++) {
        print_at(SystemTable, col, row, selected_name[i]);
        row++;
    }
}

static VOID print_left(EFI_SYSTEM_TABLE *SystemTable) {
    start_print(SystemTable);

    UINTN col = start_col;
    UINTN row = start_row;

    for (UINTN i = 0; selected_name[i] != L'\0'; i++) {
        print_at(SystemTable, col, row, selected_name[i]);
        if (row > 0) {
            row--;
        }
    }
}

static VOID show_prompt(EFI_SYSTEM_TABLE *SystemTable) {
    clear_screen(SystemTable);
    set_cursor(SystemTable, 0, 0);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, prompt);
}

static VOID reset_game(EFI_SYSTEM_TABLE *SystemTable) {
    show_prompt(SystemTable);
}

/* ---------------------------------
   Entrada principal UEFI
--------------------------------- */

EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    (void)ImageHandle;

    rng_seed(SystemTable);

start:
    reset_game(SystemTable);

    while (1) {
        EFI_INPUT_KEY key = wait_key(SystemTable);

        if (key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
            break;
        }
    }

    random_name();
    random_pos();
    print_up(SystemTable);

    while (1) {
        EFI_INPUT_KEY key = wait_key(SystemTable);

        if (key.ScanCode == SCAN_ESC) {
            clear_screen(SystemTable);
            return EFI_SUCCESS;
        }

        if (key.UnicodeChar == CHAR_BACKSPACE) {
            goto start;
        }

        if (key.ScanCode == SCAN_LEFT) {
            print_left(SystemTable);
        } else if (key.ScanCode == SCAN_RIGHT) {
            print_right(SystemTable);
        } else if (key.ScanCode == SCAN_UP) {
            print_up(SystemTable);
        } else if (key.ScanCode == SCAN_DOWN) {
            print_down(SystemTable);
        }
    }

    return EFI_SUCCESS;
}