BUILD_DIR := build
LUA_DIR := third_party/lua-5.4.8/src
CC := gcc
LD := ld
OBJCOPY := objcopy
COMMON_CFLAGS := -m32 -march=i386 -mno-mmx -mno-sse -mno-sse2 -std=c11 -ffreestanding -fno-pie -fno-pic -fno-stack-protector -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-exceptions -ffunction-sections -fdata-sections
PROJECT_CFLAGS := $(COMMON_CFLAGS) -Wall -Wextra -Werror
LUA_CFLAGS := $(COMMON_CFLAGS) -I$(LUA_DIR) -I. -DLUA_USER_H='"tb_lua_port.h"' -DLUA_32BITS=1 -DLUA_USE_CTYPE=0
LDFLAGS := -m elf_i386 --gc-sections
LUA_CORE_SRCS := lapi.c lcode.c lctype.c ldebug.c ldo.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c
LUA_CORE_OBJS := $(addprefix $(BUILD_DIR)/lua/,$(LUA_CORE_SRCS:.c=.o))

all: iso
	@printf "Criando iso...\n"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/lua:
	@mkdir -p $(BUILD_DIR)/lua

$(BUILD_DIR)/stage2_sectors.inc: $(BUILD_DIR)/stage2.bin | $(BUILD_DIR)
	@python -c 'from pathlib import Path; size = Path("build/stage2.bin").stat().st_size; print(f"%define STAGE2_SECTORS {(size + 511) // 512}")' > $@

$(BUILD_DIR)/bootloader.bin: bootloader.asm $(BUILD_DIR)/stage2_sectors.inc | $(BUILD_DIR)
	@nasm -O0 -f bin $< -o $@

$(BUILD_DIR)/stage2_entry.o: stage2_entry.asm | $(BUILD_DIR)
	@nasm -O0 -f elf32 $< -o $@

$(BUILD_DIR)/tb_jump.o: tb_jump.asm | $(BUILD_DIR)
	@nasm -O0 -f elf32 $< -o $@

$(BUILD_DIR)/stage2.o: stage2.c lua_runtime.h | $(BUILD_DIR)
	@$(CC) $(PROJECT_CFLAGS) -c stage2.c -o $@

$(BUILD_DIR)/lua_runtime.o: lua_runtime.c lua_runtime.h | $(BUILD_DIR)
	@$(CC) $(PROJECT_CFLAGS) -I$(LUA_DIR) -c lua_runtime.c -o $@

$(BUILD_DIR)/tb_libc.o: tb_libc.c tb_libc.h | $(BUILD_DIR)
	@$(CC) $(PROJECT_CFLAGS) -c tb_libc.c -o $@

$(BUILD_DIR)/tb_heap.o: tb_heap.c tb_heap.h tb_libc.h | $(BUILD_DIR)
	@$(CC) $(PROJECT_CFLAGS) -c tb_heap.c -o $@

$(BUILD_DIR)/lua/%.o: $(LUA_DIR)/%.c tb_lua_port.h tb_libc.h | $(BUILD_DIR)/lua
	@$(CC) $(LUA_CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel_lua.o: kernel.lua | $(BUILD_DIR)
	@$(OBJCOPY) -I binary -O elf32-i386 -B i386 $< $@

$(BUILD_DIR)/stage2.elf: linker.ld $(BUILD_DIR)/stage2_entry.o $(BUILD_DIR)/tb_jump.o $(BUILD_DIR)/tb_libc.o $(BUILD_DIR)/tb_heap.o $(BUILD_DIR)/stage2.o $(BUILD_DIR)/lua_runtime.o $(LUA_CORE_OBJS) $(BUILD_DIR)/kernel_lua.o
	@$(LD) $(LDFLAGS) -T linker.ld -o $@ $(BUILD_DIR)/stage2_entry.o $(BUILD_DIR)/tb_jump.o $(BUILD_DIR)/tb_libc.o $(BUILD_DIR)/tb_heap.o $(BUILD_DIR)/stage2.o $(BUILD_DIR)/lua_runtime.o $(LUA_CORE_OBJS) $(BUILD_DIR)/kernel_lua.o

$(BUILD_DIR)/stage2.bin: $(BUILD_DIR)/stage2.elf
	@$(OBJCOPY) -O binary $< $@

compile: $(BUILD_DIR)/stage2.bin $(BUILD_DIR)/bootloader.bin

img: compile
	@dd if=/dev/zero of=tbOS.img bs=1024 count=1440 status=none
	@dd if=$(BUILD_DIR)/bootloader.bin of=tbOS.img conv=notrunc status=none
	@dd if=$(BUILD_DIR)/stage2.bin of=tbOS.img bs=512 seek=1 conv=notrunc status=none

iso: clean img
	@xorriso -as mkisofs -b tbOS.img -o tbOS.iso -no-emul-boot -boot-load-size 4 ./

run: img
	@qemu-system-i386 -drive format=raw,file=tbOS.img

run-headless: img
	@qemu-system-i386 -drive format=raw,file=tbOS.img -display none -serial stdio

clean:
	@rm -rf $(BUILD_DIR) *.bin *.img *.iso *~ *.o
