#include "interrupts.h"
#include "keyboard.h"
#include "timer.h"
#include "terminal.h"
#include "asm.h"

struct {
    uint16_t size;
    uint32_t offset;
}__attribute__((packed)) idtr;

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

__attribute__((interrupt)) void InterruptStub(interrupt_frame_t* frame) {
    tputs(&mainterm,"Interrupt\n");
    asm("cli;hlt");
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

    setPITClockSpeed(1000);

    asm("sti");
}

void putInterruptIntoIDT(uint32_t interrupt,uint8_t index,uint8_t typeAttributes) {
    idt[index].isrOffset1 = interrupt&0xFFFF;
    idt[index].isrOffset2 = (interrupt>>16)&0xFFFF;
    idt[index].selector = 0x8;
    idt[index].zero = 0;
    idt[index].typeAttributes = typeAttributes;
}