#include "asm.h"
#include "images.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE (SCREEN_WIDTH*SCREEN_HEIGHT)

u8* vram_ptr = (u8*)0xA0000;

void memset(void* addr,u8 val, u32 size) {
    u8* addr_incrementor = addr;
    while (size > 0) {
        *addr_incrementor = val;
        addr_incrementor++;
        size--;
    }
}

void memcpy(void* dest, void* origin, u32 size) {
    u8* dest_cast = (u8*)dest;
    u8* origin_cast = (u8*)origin;
    while (size > 0) {
        *dest_cast = *origin_cast;
        dest_cast++;
        origin_cast++;
        size--;
    }
}

void infiniteLoop() {
loop:
    goto loop;
}

void fill_screen(u8 color) {
    memset(vram_ptr,color,SCREEN_SIZE);
}

void print_char(char character,u16 x, u16 y) {
    if (character >= 32 && character <= 126) {
        character--; //Fix for the font lol
        //Character is in "printable range"
        //Font sheet is 16 chars per row
        u8 row = character/16;
        u8 collumn = character-(row*16);
        //Each font character is 8x8; but the font sheet is linear (128x64)
        for (u8 char_row=0;char_row<8;char_row++){
            memcpy(vram_ptr+(SCREEN_WIDTH*(char_row+y))+x,fontSRC+(row*128*8)+((char_row-1)*128)+(collumn*8),8);
        }
    }
}

void print_string(const char* string,u16 x,u16 y) {
    u16 transformed_x = x;
    while (*string != 0)
    {
        print_char(*string,transformed_x,y);
        transformed_x=transformed_x+8;
        if(*string=='\n'){
            y=y+8;
            transformed_x=x;
        }
        string++;
    }
}

void kmain() {
    /*
    u8* y_to_write = vram_ptr;
    for (u32 i =0; i < SCREEN_HEIGHT; i++) {
        y_to_write = y_to_write + SCREEN_WIDTH;
        memset(y_to_write,i,SCREEN_WIDTH);
    }
    */
    memcpy(vram_ptr,splashscreen,SCREEN_SIZE);
    print_string("U MAD,\nWORLD?",100,50);
    infiniteLoop();
}