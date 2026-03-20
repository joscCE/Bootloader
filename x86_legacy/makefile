# Herramientas
ASM = nasm

BOOT_SRC = boot.asm
GAME_SRC = game.asm

BOOT_BIN = boot.bin
GAME_BIN = game.bin
GAME_PAD = game_padded.bin

IMG = os.img

SIZE_MB = 64

all: $(IMG)

$(IMG): $(BOOT_SRC) $(GAME_SRC)
	# 1. Compilar
	$(ASM) -f bin $(BOOT_SRC) -o $(BOOT_BIN)
	$(ASM) -f bin $(GAME_SRC) -o $(GAME_BIN)

	# 2. Alinear game a sectores ( clave)
	dd if=$(GAME_BIN) of=$(GAME_PAD) bs=512 conv=sync

	# 3. Crear disco vacío
	dd if=/dev/zero of=$(IMG) bs=1M count=$(SIZE_MB)

	# 4. Escribir bootloader en sector 1
	dd if=$(BOOT_BIN) of=$(IMG) bs=512 count=1 conv=notrunc

	# 5. Escribir game desde sector 2
	dd if=$(GAME_PAD) of=$(IMG) bs=512 seek=1 conv=notrunc

	qemu-system-x86_64 -drive format=raw,file=os.img

clean:
	rm -f *.bin *.img