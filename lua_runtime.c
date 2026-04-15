#include <stddef.h>

#include "lua.h"

#include "lua_runtime.h"
#include "tb_heap.h"

static console_write_fn g_console_write = NULL;

static void write_text(const char *text) {
    if (g_console_write != NULL) {
        g_console_write(text);
    }
}

static void write_lua_string(const char *text, size_t length) {
    char buffer[128];
    size_t offset = 0;

    while (offset < length) {
        size_t chunk = length - offset;
        size_t i;

        if (chunk >= sizeof(buffer)) {
            chunk = sizeof(buffer) - 1;
        }

        for (i = 0; i < chunk; ++i) {
            buffer[i] = text[offset + i];
        }

        buffer[chunk] = '\0';
        write_text(buffer);
        offset += chunk;
    }
}

static int lua_console_write(lua_State *L) {
    size_t length = 0;
    const char *text = lua_tolstring(L, 1, &length);

    if (text == NULL) {
        write_text("(nil)");
    } else {
        write_lua_string(text, length);
    }

    write_text("\r\n");
    return 0;
}

static int lua_console_halt(lua_State *L) {
    (void)L;

    write_text("tb.halt() invoked\r\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }

    return 0;
}

static int lua_panic(lua_State *L) {
    size_t length = 0;
    const char *message = lua_tolstring(L, -1, &length);

    write_text("lua panic: ");
    if (message != NULL) {
        write_lua_string(message, length);
    } else {
        write_text("(no message)");
    }
    write_text("\r\n");

    return 0;
}

static void register_kernel_api(lua_State *L) {
    lua_pushglobaltable(L);

    lua_pushcfunction(L, lua_console_write);
    lua_setfield(L, -2, "print");

    lua_newtable(L);
    lua_pushcfunction(L, lua_console_write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, lua_console_halt);
    lua_setfield(L, -2, "halt");
    lua_setfield(L, -2, "tb");

    lua_pop(L, 1);
}

static int call_boot(lua_State *L) {
    int type = lua_type(L, -1);

    if (type == LUA_TFUNCTION) {
        if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
            return -1;
        }

        return 0;
    }

    if (type != LUA_TTABLE) {
        write_text("kernel chunk returned unexpected value\r\n");
        return -1;
    }

    lua_getfield(L, -1, "boot");
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        write_text("kernel.boot not found\r\n");
        lua_pop(L, 1);
        return -1;
    }

    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        return -1;
    }

    return 0;
}

static void print_status_field(lua_State *L, const char *field_name) {
    size_t length = 0;
    const char *value;

    lua_getfield(L, -1, field_name);
    value = lua_tolstring(L, -1, &length);

    write_text(field_name);
    write_text(": ");
    if (value != NULL) {
        write_lua_string(value, length);
    } else {
        write_text("(non-string)");
    }
    write_text("\r\n");

    lua_pop(L, 1);
}

typedef struct {
    const char *data;
    size_t size;
    int consumed;
} lua_source_reader;

static const char *read_kernel_source(lua_State *L, void *ud, size_t *size) {
    lua_source_reader *reader = (lua_source_reader *)ud;

    (void)L;

    if (reader->consumed) {
        *size = 0;
        return NULL;
    }

    reader->consumed = 1;
    *size = reader->size;
    return reader->data;
}

int lua_runtime_boot(const char *source, size_t source_size, console_write_fn write_text_fn) {
    lua_State *L;
    lua_source_reader reader;

    g_console_write = write_text_fn;

    write_text("lua runtime init\r\n");
    tb_heap_init();
    write_text("heap ready\r\n");
    L = lua_newstate(tb_heap_lua_alloc, NULL);
    if (L == NULL) {
        write_text("lua_newstate failed\r\n");
        return -1;
    }

    write_text("lua state created\r\n");
    lua_atpanic(L, lua_panic);
    register_kernel_api(L);
    write_text("kernel api registered\r\n");

    reader.data = source;
    reader.size = source_size;
    reader.consumed = 0;

    write_text("loading kernel.lua\r\n");
    if (lua_load(L, read_kernel_source, &reader, "kernel.lua", NULL) != LUA_OK) {
        write_text("lua_load failed: ");
        write_text(lua_tostring(L, -1));
        write_text("\r\n");
        lua_close(L);
        return -1;
    }

    write_text("executing kernel chunk\r\n");
    if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        write_text("kernel chunk failed: ");
        write_text(lua_tostring(L, -1));
        write_text("\r\n");
        lua_close(L);
        return -1;
    }

    write_text("calling kernel.boot\r\n");
    if (call_boot(L) != 0) {
        write_text("kernel boot failed: ");
        write_text(lua_tostring(L, -1));
        write_text("\r\n");
        lua_close(L);
        return -1;
    }

    if (lua_type(L, -1) == LUA_TTABLE) {
        print_status_field(L, "name");
        print_status_field(L, "version");
        print_status_field(L, "status");
    }

    lua_close(L);
    write_text("Lua kernel boot finished\r\n");
    return 0;
}
