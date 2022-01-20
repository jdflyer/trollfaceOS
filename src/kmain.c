#include "kmain.h"
#include "terminal.h"
#include "interrupts.h"

void kmain() {
    setupIdt();
    tinit(&mainterm,VGATerm,VGA_COLOR_BLACK,80,25);
    tputs(&mainterm,"TrollfaceOS Starting...\nU mad bro?\n");
    char num[20];
    itoa(21,num,10);
    tputs(&mainterm,num);
}