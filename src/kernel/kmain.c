#include "asm.h"
#include "trollface.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE (SCREEN_WIDTH*SCREEN_HEIGHT)

u8* vram_ptr = (u8*)0xA0000;

void memset(void* addr,u8 val, u32 size) {
    u8* addr_incrementor = addr;
    while (size > 0) {
        *addr_incrementor = val;
        addr_incrementor++; //increment pointer
        size--;
    }
}

void infiniteLoop() {
loop:
    goto loop;
}

void kmain() {
    /*
    u8* y_to_write = vram_ptr;
    for (u32 i =0; i < SCREEN_HEIGHT; i++) {
        y_to_write = y_to_write + SCREEN_WIDTH;
        memset(y_to_write,i,SCREEN_WIDTH);
    }
    */
    //memset(vram_ptr, 0x28, SCREEN_WIDTH*SCREEN_HEIGHT);
    for (u32 i = 0; i < SCREEN_SIZE; i++) {
        *(vram_ptr+i) = trollface[i];
    }
    infiniteLoop();
}