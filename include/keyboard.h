#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "interrupts.h"

extern bool shiftPressed;
extern bool capsLock;

void keyboardInterruptHandler(interrupt_frame_t* frame);
