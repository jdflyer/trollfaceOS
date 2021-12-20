#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

#define TCOLOR(fg,bg) (fg|(bg<<4))
#define TTEXT(char,color) (char|(color<<8))
#define TBUFFERLOC(x,y) ((y*terminal->sizeX)+x)

typedef struct {
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t cursorX;
    uint32_t cursorY;
    uint16_t* buffer;
    uint8_t color;
}terminal_t;

terminal_t mainterm = {80,25,0,0,0,0};

void tinit(terminal_t* terminal) {
    terminal->cursorX = 0;
    terminal->cursorY = 0;
    terminal->color = TCOLOR(VGA_COLOR_WHITE,VGA_COLOR_BLACK);
    terminal->buffer = (uint16_t*)0xB8000;
    for (size_t x = 0; x<terminal->sizeX; x++) {
        for (size_t y = 0; y<terminal->sizeY; y++) {
            terminal->buffer[TBUFFERLOC(x,y)] = TTEXT(' ',terminal->color);
        }
    }
}

void tputchar(terminal_t* terminal, char c) {
    switch(c) {
    case '\n':
        terminal->cursorY++;
        terminal->cursorX = 0;
        break;
    default:
        terminal->buffer[TBUFFERLOC(terminal->cursorX,terminal->cursorY)] = TTEXT(c,terminal->color);
        terminal->cursorX++;
    };

    if(terminal->cursorX>terminal->sizeX) {
        terminal->cursorX = 0;
        terminal->cursorY++;
        if(terminal->cursorY>terminal->sizeY) {
            for(size_t y=1;y<terminal->sizeY;y++) {
                for(size_t x=0;x<terminal->sizeX;x++) {
                    terminal->buffer[TBUFFERLOC(x,y-1)] = TBUFFERLOC(x,y);
                }
            }
        }
    }
}

void twrite(terminal_t* terminal, char* data, size_t size) {
    for (size_t i=0; i<size;i++){
        tputchar(terminal,data[i]);
    }
}

size_t strlen(char* string) {
    size_t length;
    while (string[length]!=0) {
        length++;
    }
    return length;
}

void tputs(terminal_t* terminal, char* string) {
    twrite(terminal,string,strlen(string));
}

void tputs_color(terminal_t* terminal, char* string,uint8_t color) {
    terminal->color = color;
    tputs(terminal,string);
}

void kmain() {
    tinit(&mainterm);
    tputs(&mainterm,"TrollfaceOS Starting...\nU mad bro?");
}