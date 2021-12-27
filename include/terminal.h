#pragma once
#include <stddef.h>
#include <stdint.h>
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

extern terminal_t mainterm;

void tSetCursorPos(terminal_t* terminal, uint32_t x, uint32_t y);
void tclear(terminal_t* terminal, uint8_t bgColor);
void tinit(terminal_t* terminal,uint8_t termType,uint8_t bgColor, uint32_t sizeX, uint32_t sizeY);
void tputchar(terminal_t* terminal, char c);
void twrite(terminal_t* terminal, char* data, size_t size);
size_t strlen(char* string); //move later
char* itoa(int num, char* outputstring, int base); //move later
void tputs(terminal_t* terminal, char* string);
void tputs_color(terminal_t* terminal, char* string,uint8_t textColor, uint8_t bgColor);
