#ifndef DECODE
#define DECODE
#include <stdint.h>
#include <stdio.h>
#include "types.h"
void SeperateNibbles(RawInst* CurInst); //Seperate all nibbles within the instruction and fill out the nibble variables in the RawInst parameter
ParsedInst SwitchOpcode(RawInst*, Chip8*);//A switch function that redirects towards functions that read the nibbles properly
void PrintInstruction(ParsedInst,Chip8*,FILE*);
#endif