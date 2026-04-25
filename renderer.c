#include "renderer.h"
static SDL_Window* window=NULL;
static SDL_Renderer* renderer=NULL;
int InitSDL(int width,int height){
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        fprintf(stderr,"Couldnt initialize SDL:%s",SDL_GetError());
        return 1;
    }
    if(!SDL_CreateWindowAndRenderer("CHIP-8",width,height,SDL_WINDOW_RESIZABLE,&window,&renderer)){
        fprintf(stderr,"Couldnt create window/renderer:%s",SDL_GetError());
        return 1;
    }
    if(!SDL_SetRenderLogicalPresentation(renderer,64,32,SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)){
        fprintf(stderr,"Cannot set resolution:%s",SDL_GetError());
        return 1;
    }
    return 0;
}
int CheckEvents(){
    SDL_Event curEvent={0};
    while(SDL_PollEvent(&curEvent)){
        if(curEvent.type==SDL_EVENT_QUIT){
            return 1;
        }
    }
    return 0;
}
static int FillArrayPoints(SDL_FPoint* display,const uint8_t* frame){
    int count=0;
    for(int i=0;i<2048;i++){
        if(frame[i]){
            display[count].x=i%64;
            display[count].y=i/64;
            count++;
        }
    }
    return count;
}
uint8_t IsKeyPressed(uint8_t key){
    const bool* const keyboard=SDL_GetKeyboardState(NULL);
    if(keyboard[key+4]){
        return 1;
    }
    return 0;
}
uint8_t KeyPressed(){
    const bool* const keyboard=SDL_GetKeyboardState(NULL);
    for(int i=4;i<20;i++){
        if(keyboard[i]){
            return i-4;
        }
    }
    return 0;
}
int Render(const uint8_t* frame){
    static uint8_t prevframe[64*32]={0}; //previousframe
    static uint8_t firstframe=1; //a check for if its the first frame  
    SDL_FPoint display[64*32]={0};
    int numofPoints=FillArrayPoints(display,frame);
    SDL_SetRenderDrawColor(renderer,255,255,255,SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
    SDL_RenderPoints(renderer,(const SDL_FPoint*)display,numofPoints);
    SDL_RenderPresent(renderer);
    memcpy(prevframe,frame,2048);
    firstframe=0;
    return 0;
    
    

}