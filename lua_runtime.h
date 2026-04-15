#ifndef LUA_RUNTIME_H
#define LUA_RUNTIME_H

#include <stddef.h>

typedef void (*console_write_fn)(const char *text);

int lua_runtime_boot(const char *source, size_t source_size, console_write_fn write_text);

#endif
