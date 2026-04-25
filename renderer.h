#ifndef RENDER
#define RENDER
#include <SDL3/SDL.h>
#include <stdio.h>
int InitSDL(int width,int height); //Initialize SDL window and renderer. Returns 0 on success,prints to stderr and returns 1 on error
int CheckEvents(); //checks if user has requested exit of program, if so return 1, otherwise return 0 and continue
int Render(const uint8_t*); //returns 0 (for now), ill consider making it a void
uint8_t IsKeyPressed(uint8_t key);
uint8_t KeyPressed();



#endif