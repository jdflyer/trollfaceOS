#include "trollfacememory.h"
#include "terminal.h"

void memset(void* addr, uint8_t data, size_t size) {
    while(size--) {
        *(uint8_t*)addr++ = data;
    }
}

alloc_prefix_t *const firstAlloc = (alloc_prefix_t*)HEAP_START;

void malloc_init() {
    firstAlloc->size = 0;
    firstAlloc->next = NULL;
    firstAlloc->prev = NULL;
}

void free(void* ptr) {
    alloc_prefix_t* prefix = (ptr-sizeof(alloc_prefix_t));
    prefix->prev->next = prefix->next;
    prefix->next->prev = prefix->prev;
}

void* malloc(size_t size) {
    /* The goal for the trollfaceOS heap is to use an linked list but the next pointers remain
     * in a constant order forward, so we can tell which memory is free and which isn't*/
    alloc_prefix_t* curr = firstAlloc;
    while (true) {
        if(curr->next!=NULL) {
            size_t free_space = ((uint8_t*)(curr->next))-(((uint8_t*)curr)+sizeof(alloc_prefix_t)+curr->size);
            if (free_space<(size+sizeof(alloc_prefix_t))) {
                //Not enough space, try next allocation
                curr = curr->next;
            }else {
                //There's enough free space in the middle of the heap, allocate here
                alloc_prefix_t* prev = curr;
                alloc_prefix_t* next = curr->next;
                curr = (alloc_prefix_t*)(((uint8_t*)curr)+sizeof(alloc_prefix_t)+curr->size);
                curr->size = size;
                curr->next = next;
                curr->prev = prev;
                next->prev = curr;
                prev->next = curr;
                break;
            }
        }else{
            //Allocate on top of the heap
            alloc_prefix_t* prev = curr;
            curr = (alloc_prefix_t*)(((uint8_t*)curr)+sizeof(alloc_prefix_t)+curr->size);
            curr->size = size;
            curr->prev = prev;
            prev->next = curr;
            curr->next = NULL;
            break;
        }
    }
    return ((uint8_t*)curr)+sizeof(alloc_prefix_t);
}