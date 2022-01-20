#include "terminal.h"
#include "asm.h"

terminal_t mainterm;

void tSetCursorPos(terminal_t* terminal, uint32_t x, uint32_t y) {
    terminal->cursorX = x;
    terminal->cursorY = y;
    terminal->buffer[TBUFFERLOC(terminal,x,y)] = TTEXT('_',TCOLOR(terminal->textColor,terminal->bgColor));
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

void tinit(terminal_t* terminal,uint8_t termType,uint8_t bgColor, uint32_t sizeX, uint32_t sizeY) {
    tSetCursorPos(terminal,0,0);
    terminal->textColor = VGA_COLOR_WHITE;
    terminal->bgColor = bgColor;
    terminal->sizeX = sizeX;
    terminal->sizeY = sizeY;
    switch(termType) {
    case VGATerm:
        terminal->buffer = (uint16_t*)0xB8000;
        outb(0x3D4, 0x0A);
	    outb(0x3D5, 0x20);
        break;
    }
    tclear(terminal,bgColor);
}

void tputchar(terminal_t* terminal, char c) {
    switch(c) {
    case '\n':
        CLEARCURSORCHAR(terminal);
        tSetCursorPos(terminal,0,terminal->cursorY+1);
        break;
    case '\b':
        if(terminal->cursorX>0) {
            terminal->buffer[TBUFFERLOC(terminal,terminal->cursorX-1,terminal->cursorY)] = ' ';
            CLEARCURSORCHAR(terminal);
            tSetCursorPos(terminal,terminal->cursorX-1,terminal->cursorY);
        }
        break;
    case '\t':
        tputs(terminal,"     ");
        break;
    default:
        terminal->buffer[TBUFFERLOC(terminal,terminal->cursorX,terminal->cursorY)] = TTEXT(c,TCOLOR(terminal->textColor,terminal->bgColor));
        tSetCursorPos(terminal,terminal->cursorX+1,terminal->cursorY);
        break;
    };

    if(terminal->cursorX>terminal->sizeX) {
        CLEARCURSORCHAR(terminal);
        tSetCursorPos(terminal,0,terminal->cursorY+1);
    }
    if(terminal->cursorY>=terminal->sizeY) {
        for(size_t y=1;y<terminal->sizeY;y++) {
            for(size_t x=0;x<terminal->sizeX;x++) {
                terminal->buffer[TBUFFERLOC(terminal,x,y-1)] = terminal->buffer[TBUFFERLOC(terminal,x,y)];
            }
        }
        for(size_t x=0;x<terminal->sizeX;x++) {
            terminal->buffer[TBUFFERLOC(terminal,x,terminal->sizeY-1)] = ' ';
        }
        tSetCursorPos(terminal,0,terminal->sizeY-1);
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

char* itoa(int num, char* outputstring, int base) {
    if(num==0) {
        outputstring[0] = '0';
        outputstring[1] = 0;
        return outputstring;
    }
    
    bool negative = false;
    char string[20];
    if (num<0&&base==10) {
        negative = true;
        num = -num;
    }

    int i = 0;
    while(num!=0) {
        int32_t remainder = num % base;
        if (remainder>9) {
            string[i++] = (remainder-10)+'a';
        }else{
            string[i++] = remainder+'0';
        }
        num = num/base;
    }

    if(negative) {
        string[i++] = '-';
    }

    size_t j = 0;
    while (i>0) {
        i--;
        outputstring[j] = string[i];
        j++;
    }

    outputstring[j] = 0;

    return outputstring;
}

void tputs(terminal_t* terminal, char* string) {
    twrite(terminal,string,strlen(string));
}

void tputs_color(terminal_t* terminal, char* string,uint8_t textColor, uint8_t bgColor) {
    terminal->textColor = textColor;
    terminal->bgColor = bgColor;
    tputs(terminal,string);
}