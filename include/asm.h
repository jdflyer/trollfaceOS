#include <stdint.h>

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