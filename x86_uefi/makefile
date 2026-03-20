CLANG = clang
LINKER = lld-link
QEMU = qemu-system-x86_64
OVMF = /usr/share/qemu/OVMF.fd

EFI_INC = -I/usr/include/efi -I/usr/include/efi/x86_64
CFLAGS = -target x86_64-pc-win32 $(EFI_INC) -ffreestanding -fshort-wchar -mno-red-zone
LDFLAGS = /subsystem:efi_application /entry:efi_main

ESP_DIR = esp/EFI/BOOT

BOOT_SRC = bootuefi.c
BOOT_OBJ = bootuefi.o
BOOT_EFI = BOOTX64.EFI

GAME_SRC = game.c
GAME_OBJ = game.o
GAME_EFI = game.efi

.PHONY: all run clean copy esp

all: esp

$(BOOT_OBJ): $(BOOT_SRC)
	$(CLANG) $(CFLAGS) -c $(BOOT_SRC) -o $(BOOT_OBJ)

$(BOOT_EFI): $(BOOT_OBJ)
	$(LINKER) $(LDFLAGS) $(BOOT_OBJ) /out:$(BOOT_EFI)

$(GAME_OBJ): $(GAME_SRC)
	$(CLANG) $(CFLAGS) -c $(GAME_SRC) -o $(GAME_OBJ)

$(GAME_EFI): $(GAME_OBJ)
	$(LINKER) $(LDFLAGS) $(GAME_OBJ) /out:$(GAME_EFI)

$(ESP_DIR):
	mkdir -p $(ESP_DIR)

copy: $(BOOT_EFI) $(GAME_EFI) $(ESP_DIR)
	cp $(BOOT_EFI) $(ESP_DIR)/
	cp $(GAME_EFI) $(ESP_DIR)/

esp: copy

run: esp
	$(QEMU) \
		-bios $(OVMF) \
		-drive format=raw,file=fat:rw:esp

clean:
	rm -f $(BOOT_OBJ) $(GAME_OBJ) $(BOOT_EFI) $(GAME_EFI)