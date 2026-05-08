#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "machine.h"
#include "decoding.h"
#include "renderer.h"
#include <time.h>

uint16_t ReadFile(uint8_t* memory,FILE* rom){
    fseek(rom,0,SEEK_END);
    int count=ftell(rom);
    rewind(rom);
    if(count>0xFFF-0x200) return 0;
    fread((memory+0x200),sizeof(uint8_t),count,rom);
    return count;
    

}
void DumpState(FILE* output,ParsedInst curInst,Chip8* instance){
    fprintf(output,"Currrent Instruction:");
    PrintInstruction(curInst,instance,output);
    if(curInst.code==JP) return;
    fprintf(output,"Status of registers:");
    for(int i=0;i<16;i++){
        fprintf(output,"V%X=%d\n",i,instance->V[i]);
    }
    fprintf(output,"Memory Register:0x%03X\n",instance->I);
    fprintf(output,"Current Frame:\n");
    for(int i=0;i<32;i++){
        for(int j=0;j<64;j++){
            fprintf(output,"%d",instance->frame[i*64+j]);
        }
        fprintf(output,"\n");
    }


}
struct ThreadData{
    Chip8* machine;
    SDL_AudioSpec* spec
};

int SDLCALL TimerThread(void* data){ //unfinished
    struct ThreadData* curdata=(struct ThreadData*)data;
    Chip8* instance=curdata->machine;
    SDL_AudioSpec* spec=curdata->spec;
    while(1){
           if(instance->soundtimer!=0) {
            
            instance->soundtimer--;}
           if(instance->delaytimer!=0) instance->delaytimer--;
            SDL_Delay(16);
        }
    return 0;
}

int main (int argc,char* argv[]){
    Chip8 machine={0};
 /*   SDL_Thread* thread; //unfinished
    SDL_AudioSpec spec={0};
    spec.freq=44100;
    spec.format=SDL_AUDIO_F32; 
    spec.channels=1;
    struct ThreadData data={
        .machine=&machine,
        .spec=&spec,
    }; */
    char filename[100]="default.ch8";
    for(int i=1;i<argc;i++){ //argument check for rom
        strncpy(filename,argv[i],99);
        filename[99]=0; 

    }
    FILE* rom=fopen(filename,"rb");
    FILE* output=fopen("output.txt","w");
    if(rom==NULL){
        printf("Could not open file\n");
        return 1;
    }
    machine.programsize=ReadFile(machine.memory,rom);
    machine.counter=0x200;
    if(InitSDL(640,320)) return 1;
    int i=0;
    SetFonts(&machine);
  /*  while(1){ //renderer test
        if(CheckEvents()) break;
        DrawFrame(&machine,machine.frame,(uint8_t)i);
        i++;
        if(i>=16) i=0;
        SDL_Delay(600);
        Execute(&machine,test);
        SDL_Delay(600);
}*/
  //  thread=SDL_CreateThread(TimerThread,"Test",(void*)&data);
  uint64_t ic=0;
    while (1){
        if(CheckEvents()) break;
        RawInst curInst={0};
        curInst.instruction[0]=machine.memory[machine.counter];
        curInst.instruction[1]=machine.memory[machine.counter+1];
        SeperateNibbles(&curInst);
        ParsedInst Inst=SwitchOpcode(&curInst,&machine);
        //fprintf(stdout,"0x%03X:",machine.counter);
        //PrintInstruction(Inst,&machine,stdout);
        Signal signal=Execute(&machine,Inst);
        ic++;
       // DumpState(output,Inst,&machine);
        if(signal.type==SIGNAL_DRAW){
            Render(machine.frame);
            machine.counter+=2;
        }
        else if(signal.type==SIGNAL_CONTINUE){
            machine.counter+=2;
        }
        else if(signal.type==SIGNAL_PAUSE){
            int8_t key=KeyPressed();
            if(key==-1) continue;
            else {
                *Inst.op1.value.byte=key;
                machine.counter+=2;
            }
        }
        else if(signal.type==SIGNAL_SKP){
            if(IsKeyPressed(signal.key)){
                machine.counter+=2;
            }
            machine.counter+=2;
        }
        else if(signal.type==SIGNAL_SKNP){
            if(!IsKeyPressed(signal.key)){
                machine.counter+=2;
            }
            machine.counter+=2;
        }
    
    }
    printf("Total time elapsed:%f\nInstruction count:%llu",(float)clock()/CLOCKS_PER_SEC,ic);
    return 0;
}