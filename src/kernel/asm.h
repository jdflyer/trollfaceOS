#ifndef ASM_H
#define ASM_H
#include "typedefs.h"
#ifndef asm
#define asm __asm__ volatile
#endif

static inline u8 inportb(u16 port) {
    u8 r;
    asm("inb %1, %0" : "=a" (r) : "dN" (port));
    return r;
}
#endif