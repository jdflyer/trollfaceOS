#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t isrOffset1;
    uint16_t selector;
    uint8_t zero;
    uint8_t typeAttributes;
    uint16_t isrOffset2;
}__attribute__((packed)) idt_entry_t;

typedef struct{
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t sp;
    uint32_t ss;
} interrupt_frame_t;

void PICSendEOI(uint8_t irq);
void remapPIC(uint8_t offset1, uint8_t offset2);
void setIRQMask(uint8_t line);
void clearIRQMask(uint8_t line);
void InterruptStub(interrupt_frame_t* frame);
void setupIdt();
void putInterruptIntoIDT(uint32_t interrupt,uint8_t index,uint8_t typeAttributes);
