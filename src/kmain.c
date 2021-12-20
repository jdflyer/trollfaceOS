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
#define TBUFFERLOC(term,x,y) ((y*(term->sizeX))+x)

inline void outb(uint16_t port,uint8_t value) {
    asm("outb %b0, %w1"::"a"(value),"d"(port));
}

enum terminalType {
    VGATerm
};

typedef struct {
    uint32_t sizeX;
    uint32_t sizeY;
    uint32_t cursorX;
    uint32_t cursorY;
    uint16_t* buffer;
    uint8_t textColor;
    uint8_t bgColor;
    uint8_t type;
}terminal_t;

terminal_t mainterm;

void tSetCursorPos(terminal_t* terminal, uint32_t x, uint32_t y) {
    terminal->cursorX = x;
    terminal->cursorY = y;
    uint16_t pos = TBUFFERLOC(terminal,x,y);
    outb(0x3d4,0xF);
    outb(0x3d5,(uint8_t)(pos&0xFF));
    outb(0x3d4,0xE);
    outb(0x3d4,(uint8_t)((pos>>8)&0xFF));
}

void tclear(terminal_t* terminal, uint8_t bgColor) {
    terminal->bgColor = bgColor;
    for (size_t x = 0; x<terminal->sizeX; x++) {
        for (size_t y = 0; y<terminal->sizeY; y++) {
            terminal->buffer[TBUFFERLOC(terminal,x,y)] = TTEXT(' ',TCOLOR(terminal->textColor,terminal->bgColor));
        }
    }
    tSetCursorPos(terminal,0,0);
}

void tinit(terminal_t* terminal,uint8_t termType,uint8_t bgColor) {
    tSetCursorPos(terminal,0,0);
    terminal->textColor = VGA_COLOR_WHITE;
    terminal->bgColor = bgColor;
    switch(termType) {
    case VGATerm:
        terminal->sizeX = 80;
        terminal->sizeY = 25;
        terminal->buffer = (uint16_t*)0xB8000;
        break;
    }
    tclear(terminal,bgColor);
}

void tputchar(terminal_t* terminal, char c) {
    switch(c) {
    case '\n':
        tSetCursorPos(terminal,0,terminal->cursorY+1);
        break;
    default:
        terminal->buffer[TBUFFERLOC(terminal,terminal->cursorX,terminal->cursorY)] = TTEXT(c,TCOLOR(terminal->textColor,terminal->bgColor));
        tSetCursorPos(terminal,terminal->cursorX+1,terminal->cursorY);
        break;
    };

    if(terminal->cursorX>terminal->sizeX) {
        tSetCursorPos(terminal,0,terminal->cursorY+1);
        if(terminal->cursorY>terminal->sizeY) {
            for(size_t y=1;y<terminal->sizeY;y++) {
                for(size_t x=0;x<terminal->sizeX;x++) {
                    terminal->buffer[TBUFFERLOC(terminal,x,y-1)] = TBUFFERLOC(terminal,x,y);
                }
            }
            for(size_t x=0;x<terminal->sizeX;x++) {
                terminal->buffer[TBUFFERLOC(terminal,x,terminal->sizeY-1)] = ' ';
            }
            tSetCursorPos(terminal,0,terminal->sizeY-1);
        }
    }
}

void twrite(terminal_t* terminal, char* data, size_t size) {
    for (size_t i=0; i<size;i++){
        tputchar(terminal,data[i]);
    }
}

size_t strlen(char* string) {
    size_t length = 0;
    while (string[length]!=0) {
        length++;
    }
    return length;
}

void tputs(terminal_t* terminal, char* string) {
    twrite(terminal,string,strlen(string));
}

void tputs_color(terminal_t* terminal, char* string,uint8_t textColor, uint8_t bgColor) {
    terminal->textColor = textColor;
    terminal->bgColor = bgColor;
    tputs(terminal,string);
}

void kmain() {
    tinit(&mainterm,VGATerm,VGA_COLOR_BLACK);
    tputs(&mainterm,"TrollfaceOS Starting...\nU mad bro?");
}