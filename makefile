all:
	nasm boot.asm -f bin -o boot.bin
	nasm game.asm -f bin -o game.bin
	cat boot.bin game.bin > os.img
	qemu-system-x86_64 -drive format=raw,file=os.img
