#include "timer.h"
#include "asm.h"

uint64_t systemTicks = 0;

__attribute__((interrupt)) void pitInterruptHandler(interrupt_frame_t* frame) {
    systemTicks++;
    PICSendEOI(0);
}

#define PITCHANNEL0PORT 0x40
#define PITCHANNEL1PORT 0x41
#define PITCHANNEL2PORT 0x42
#define PITCONTROLLERPORT 0x43

void setPITClockSpeed(uint32_t hz) {
    uint16_t divisor = 1193180 / hz;
    outb(PITCONTROLLERPORT, 0b00110110);
    outb(PITCHANNEL0PORT, divisor & 0xFF);
    outb(PITCHANNEL0PORT, divisor >> 8);
}