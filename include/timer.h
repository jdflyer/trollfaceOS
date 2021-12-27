#pragma once
#include <stddef.h>
#include <stdint.h>
#include "interrupts.h"

void setPITClockSpeed(uint32_t hz);
void pitInterruptHandler(interrupt_frame_t* frame);
