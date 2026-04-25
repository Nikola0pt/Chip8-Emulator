#ifndef MACHINE
#define MACHINE
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "types.h"
Signal Execute(Chip8*,ParsedInst); // returns a signal for use in main
void SetFonts(Chip8*);
void DrawFrame(Chip8* instance,uint8_t* frame,uint8_t amount,uint8_t x,uint8_t y);



#endif