#include "tb_heap.h"

#include "tb_libc.h"

#define TB_HEAP_BYTES (1024u * 1024u)
#define TB_HEAP_ALIGNMENT 8u

typedef struct tb_heap_block {
    size_t size;
    int free;
    struct tb_heap_block *next;
} tb_heap_block;

static unsigned char tb_heap_area[TB_HEAP_BYTES];
static tb_heap_block *tb_heap_root = NULL;

static size_t align_size(size_t size) {
    size_t mask = TB_HEAP_ALIGNMENT - 1u;

    return (size + mask) & ~mask;
}

static void split_block(tb_heap_block *block, size_t size) {
    size_t remaining = block->size - size;
    tb_heap_block *old_next = block->next;

    if (remaining <= sizeof(tb_heap_block) + TB_HEAP_ALIGNMENT) {
        return;
    }

    block->next = (tb_heap_block *)((unsigned char *)(block + 1) + size);
    block->next->size = remaining - sizeof(tb_heap_block);
    block->next->free = 1;
    block->next->next = old_next;
    block->size = size;
}

static void coalesce_blocks(void) {
    tb_heap_block *block = tb_heap_root;

    while (block != NULL && block->next != NULL) {
        if (block->free && block->next->free) {
            block->size += sizeof(tb_heap_block) + block->next->size;
            block->next = block->next->next;
        } else {
            block = block->next;
        }
    }
}

void tb_heap_init(void) {
    tb_heap_root = (tb_heap_block *)tb_heap_area;
    tb_heap_root->size = TB_HEAP_BYTES - sizeof(tb_heap_block);
    tb_heap_root->free = 1;
    tb_heap_root->next = NULL;
}

void *tb_heap_alloc(size_t size) {
    tb_heap_block *block;

    if (size == 0) {
        return NULL;
    }

    size = align_size(size);
    block = tb_heap_root;
    while (block != NULL) {
        if (block->free && block->size >= size) {
            split_block(block, size);
            block->free = 0;
            return (void *)(block + 1);
        }

        block = block->next;
    }

    return NULL;
}

void tb_heap_free(void *pointer) {
    tb_heap_block *block;

    if (pointer == NULL) {
        return;
    }

    block = ((tb_heap_block *)pointer) - 1;
    block->free = 1;
    coalesce_blocks();
}

void *tb_heap_realloc(void *pointer, size_t size) {
    tb_heap_block *block;
    void *new_pointer;
    size_t bytes_to_copy;

    if (pointer == NULL) {
        return tb_heap_alloc(size);
    }

    if (size == 0) {
        tb_heap_free(pointer);
        return NULL;
    }

    size = align_size(size);
    block = ((tb_heap_block *)pointer) - 1;
    if (block->size >= size) {
        return pointer;
    }

    new_pointer = tb_heap_alloc(size);
    if (new_pointer == NULL) {
        return NULL;
    }

    bytes_to_copy = block->size < size ? block->size : size;
    memcpy(new_pointer, pointer, bytes_to_copy);
    tb_heap_free(pointer);
    return new_pointer;
}

void *tb_heap_lua_alloc(void *ud, void *pointer, size_t old_size, size_t new_size) {
    (void)ud;
    (void)old_size;

    if (new_size == 0) {
        tb_heap_free(pointer);
        return NULL;
    }

    return tb_heap_realloc(pointer, new_size);
}
