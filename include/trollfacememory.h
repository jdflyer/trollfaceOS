#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define HEAP_START 0x200000

typedef struct alloc_prefix_t alloc_prefix_t;

struct alloc_prefix_t {
    alloc_prefix_t* next;
    alloc_prefix_t* prev;
    size_t size;
};

extern alloc_prefix_t *const firstAlloc;

void memset(void* addr, uint8_t data, size_t size);
void malloc_init();
void free(void* ptr);
void* malloc(size_t size);
