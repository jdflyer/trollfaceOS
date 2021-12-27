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
    asm("outb %0, %1" : : "a"(value), "Nd"(port));
}

inline uint8_t inb(uint16_t port) {
    uint8_t retval;
    asm("inb %1, %0":"=a"(retval):"Nd"(port));
    return retval;
}

#define IO_WAIT() outb(0x80,0);
#define OUTB_WAIT(port,value) outb(port,value);IO_WAIT();

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

void tinit(terminal_t* terminal,uint8_t termType,uint8_t bgColor, uint32_t sizeX, uint32_t sizeY) {
    tSetCursorPos(terminal,0,0);
    terminal->textColor = VGA_COLOR_WHITE;
    terminal->bgColor = bgColor;
    terminal->sizeX = sizeX;
    terminal->sizeY = sizeY;
    switch(termType) {
    case VGATerm:
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
    case '\b':
        if(terminal->cursorX>0) {
            terminal->buffer[TBUFFERLOC(terminal,terminal->cursorX-1,terminal->cursorY)] = ' ';
            tSetCursorPos(terminal,terminal->cursorX-1,terminal->cursorY);
        }
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

struct {
    uint16_t size;
    uint32_t offset;
}__attribute__((packed)) idtr;

typedef struct {
    uint16_t isrOffset1;
    uint16_t selector;
    uint8_t zero;
    uint8_t typeAttributes;
    uint16_t isrOffset2;
}__attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
idt_entry_t idt[256];

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1+1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2+1)

#define PIC_EOI 0x20

void PICSendEOI(uint8_t irq) {
    if(irq>=8) {
        outb(PIC2_COMMAND,PIC_EOI);
    }
    outb(PIC1_COMMAND,PIC_EOI);
}

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10
 
#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

void remapPIC(uint8_t offset1, uint8_t offset2) {
    uint8_t mask1,mask2;
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    OUTB_WAIT(PIC1_COMMAND,ICW1_INIT|ICW1_ICW4);
    OUTB_WAIT(PIC2_COMMAND,ICW1_INIT|ICW1_ICW4);
    OUTB_WAIT(PIC1_DATA,offset1);
    OUTB_WAIT(PIC2_DATA,offset2);
    OUTB_WAIT(PIC1_DATA,4);
    OUTB_WAIT(PIC2_DATA,2);

    OUTB_WAIT(PIC1_DATA,ICW4_8086);
    OUTB_WAIT(PIC2_DATA,ICW4_8086);

    outb(PIC1_DATA,mask1);
    outb(PIC2_DATA,mask2);
}

void setIRQMask(uint8_t line) {
    uint16_t port;
    if (line<8) {
        port=PIC1_DATA;
    }else{
        port=PIC2_DATA;
        line = line - 8;
    }
    uint8_t value = (inb(port) | (1<<line));
    outb(port,value);
}

void clearIRQMask(uint8_t line) {
    uint16_t port;
    if (line<8) {
        port=PIC1_DATA;
    }else{
        port=PIC2_DATA;
        line = line - 8;
    }
    uint8_t value = (inb(port) | (~(1<<line)));
    outb(port,value);
}

typedef struct{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} interrupt_frame_t;

uint8_t keyboardScancodesTable[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};		

__attribute__((interrupt)) void keyboardInterruptHandler(interrupt_frame_t* frame) {
    uint8_t scancode = inb(0x60);
    if (keyboardScancodesTable[scancode]!=0) {
        tputchar(&mainterm,keyboardScancodesTable[scancode]);
    }
    PICSendEOI(1);
}

__attribute__((interrupt)) void InterruptStub(interrupt_frame_t* frame) {
    tputs(&mainterm,"Interrupt\n");
    asm("cli;hlt");
}

__attribute__((interrupt)) void pitInterruptHandler(interrupt_frame_t* frame) {
    tputs(&mainterm,"timer");
    PICSendEOI(0);
}

void putInterruptIntoIDT(uint32_t interrupt,uint8_t index,uint8_t typeAttributes) {
    idt[index].isrOffset1 = interrupt&0xFFFF;
    idt[index].isrOffset2 = (interrupt>>16)&0xFFFF;
    idt[index].selector = 0x8;
    idt[index].zero = 0;
    idt[index].typeAttributes = typeAttributes;
}

#define PITCHANNEL0PORT 0x40
#define PITCHANNEL1PORT 0x41
#define PITCHANNEL2PORT 0x42
#define PITCONTROLLERPORT 0x43

void setPITClockSpeed(uint32_t hz)
{
    int divisor = 1193180 / hz;
    outb(PITCONTROLLERPORT, 0b00110110);
    outb(PITCHANNEL0PORT, divisor & 0xFF);
    outb(PITCHANNEL0PORT, divisor >> 8);
}

void setupIdt() {
    asm("cli");
    idtr.offset = (uint32_t)&idt;
    idtr.size = sizeof(idt)-1;
    asm("lidt %0" : : "m"(idtr));

    remapPIC(0x20,0x28);

    for (size_t i=0; i<sizeof(idt)/sizeof(idt_entry_t);i++) {
        putInterruptIntoIDT((uint32_t)&InterruptStub,i,0b10001110);
    }
    putInterruptIntoIDT((uint32_t)&keyboardInterruptHandler,0x21,0b10001110);
    putInterruptIntoIDT((uint32_t)&pitInterruptHandler,0x20,0b10001110);

    //setPITClockSpeed(1);
    //clearIRQMask(0);
    clearIRQMask(1);
    

    asm("sti");
}

void kmain() {
    setupIdt();
    tinit(&mainterm,VGATerm,VGA_COLOR_BLACK,80,25);
    tputs(&mainterm,"TrollfaceOS Starting...\nU mad bro?\n");
    char num[20];
    itoa(21,num,10);
    tputs(&mainterm,num);
}