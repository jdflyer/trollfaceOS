#include "kmain.h"
#include "terminal.h"
#include "interrupts.h"
#include "trollfacememory.h"

void kmain() {
    setupIdt();
    malloc_init();
    tinit(&mainterm,VGATerm,VGA_COLOR_BLACK,80,25);
    tputs(&mainterm,"TrollfaceOS Starting...\nU mad bro?\n");
    char* num = malloc(200);
    memset(num,'1',200);
    twrite(&mainterm,num,200);
    itoa(21,num,10);
    tputs(&mainterm,num);
    free(num);
}