#include <stddef.h>
#include <stdint.h>

#include "lua_runtime.h"

extern const char _binary_kernel_lua_start[];
extern const char _binary_kernel_lua_end[];

static volatile uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
static size_t cursor_row = 0;
static size_t cursor_col = 0;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;

    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial_init(void) {
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x80);
    outb(0x3F8 + 0, 0x03);
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03);
    outb(0x3F8 + 2, 0xC7);
    outb(0x3F8 + 4, 0x0B);
}

static void serial_write_char(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0) {
    }

    outb(0x3F8, (uint8_t)c);
}

static void put_char(char c) {
    if (c == '\r') {
        cursor_col = 0;
        serial_write_char(c);
        return;
    }

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        serial_write_char(c);
        return;
    }

    if (cursor_row >= 25) {
        cursor_row = 0;
    }

    VGA_MEMORY[cursor_row * 80 + cursor_col] = (uint16_t)0x0F00 | (uint8_t)c;
    cursor_col++;
    serial_write_char(c);

    if (cursor_col >= 80) {
        cursor_col = 0;
        cursor_row++;
    }
}

static void write_text(const char *text) {
    while (*text != '\0') {
        put_char(*text++);
    }
}

static void clear_screen(void) {
    size_t i;

    for (i = 0; i < 80 * 25; ++i) {
        VGA_MEMORY[i] = 0x0720;
    }

    cursor_row = 0;
    cursor_col = 0;
}

static void write_decimal(size_t value) {
    char buffer[21];
    size_t index = 0;

    if (value == 0) {
        put_char('0');
        return;
    }

    while (value > 0) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0) {
        put_char(buffer[--index]);
    }
}

void stage2_main(void) {
    size_t kernel_size = (size_t)(_binary_kernel_lua_end - _binary_kernel_lua_start);
    int runtime_status;

    serial_init();
    clear_screen();
    write_text("tbOS stage2\r\n");
    write_text("ANSI C loader online\r\n");
    write_text("embedded kernel.lua size: ");
    write_decimal(kernel_size);
    write_text(" bytes\r\n");

    runtime_status = lua_runtime_boot(_binary_kernel_lua_start, kernel_size, write_text);
    if (runtime_status != 0) {
        write_text("boot halted before Lua handoff\r\n");
    } else {
        write_text("boot completed\r\n");
    }
}
