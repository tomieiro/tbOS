all: iso
	@printf "Criando iso...\n"

compile:
	@nasm	-O0	bootloader.asm	-f	bin	-o	bootloader.bin
	@nasm	-O0	kernel.asm	-f	bin	-o	kernel.bin
	@cat	bootloader.bin	kernel.bin	>	tbOS.bin

img:	compile
	@dd	if=/dev/zero	of=tbOS.img	bs=1024	count=1440
	@dd	if=tbOS.bin	of=tbOS.img	seek=0	conv=notrunc

iso:	clean	img
	@xorriso	-as	mkisofs	-b	tbOS.img	-o	tbOS.iso	-isohybrid-mbr	tbOS.bin	-no-emul-boot	-boot-load-size	4 ./

run:
	@qemu-system-i386	-drive	format=raw,file=tbOS.iso

clean:
	@rm	-f	*.bin	*.img	*.iso	*~	*.o