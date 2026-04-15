#ifndef TB_HEAP_H
#define TB_HEAP_H

#include <stddef.h>

void tb_heap_init(void);
void *tb_heap_alloc(size_t size);
void tb_heap_free(void *pointer);
void *tb_heap_realloc(void *pointer, size_t size);
void *tb_heap_lua_alloc(void *ud, void *pointer, size_t old_size, size_t new_size);

#endif
