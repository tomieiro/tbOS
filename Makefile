STICK = /dev/sdb

MBR = bootloader.bin
QEMU=qemu-system-i386
ISO = boot.iso
FLOPPY = floppy.img

all: $(MBR) 

%.bin : %.asm
	nasm -O0 $< -f bin -o $@

iso: $(ISO)

$(ISO) : $(FLOPPY)
	xorriso -as mkisofs -b $< -o $@ -isohybrid-mbr $(MBR) \
	-no-emul-boot -boot-load-size 4 ./

$(FLOPPY) : $(MBR)
	dd if=/dev/zero of=$@ bs=1024 count=1440
	dd if=$< of=$@ seek=0 conv=notrunc

test: clean $(ISO)
	qemu-system-i386 -drive format=raw,file=$(ISO) -net none

stick: $(ISO)
	@if test -z "$(STICK)"; then \
	 echo "*** ATTENTION: Edit Makefile first"; exit 1; fi 
	dd if=$< of=$(STICK)

clean:
	rm -f *.bin *.img *.iso *~ *.o 
