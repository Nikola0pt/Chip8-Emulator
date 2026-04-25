#include "machine.h"
void SetFonts(Chip8* instance){
 uint8_t fonts[16*5]={
    0xf0,0x90,0x90,0x90,0xf0, //0
    0x20,0x60,0x20,0x20,0x70, //1
    0xf0,0x10,0xf0,0x80,0xf0, //2
    0xf0,0x10,0xf0,0x10,0xf0, //3
    0x90,0x90,0xf0,0x10,0x10, //4
    0xf0,0x80,0xf0,0x10,0xf0, //5
    0xf0,0x80,0xf0,0x90,0xf0, //6
    0xf0,0x10,0x20,0x40,0x40, //7
    0xf0,0x90,0xf0,0x90,0xf0, //8
    0xf0,0x90,0xf0,0x10,0xf0, //9
    0xf0,0x90,0xf0,0x90,0x90, //A
    0xe0,0x90,0xe0,0x90,0xe0, //B
    0xF0,0x80,0x80,0x80,0xf0, //C
    0xe0,0x90,0x90,0x90,0xe0, //D
    0xf0,0x80,0xf0,0x80,0xf0, //E
    0xf0,0x80,0xf0,0x80,0x80 //F

 };
 memcpy(instance->memory,fonts,16*5);

}
void DrawFrame(Chip8* instance,uint8_t* frame,uint8_t amount,uint8_t x,uint8_t y){
    uint8_t* L0=instance->memory+instance->I;
    instance->V[0xf]=0;
    int bit;
    for(int i=0;i<amount;i++){
        bit=(64*(y+i))+x;
        for(int j=1<<7;j>0;j>>=1){
            uint8_t onoff=!!(L0[i]&j);
            if(bit>=64*32) continue;
            if(frame[bit] && onoff){
                instance->V[0xf]=1;
            }
            frame[bit++]^=onoff;
        }
    }

}
static Signal FUNDEFINED(Chip8* instance,ParsedInst instruction){
    printf("Warning:UNDEFINED instruction: skip for now\n");
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FCLS(Chip8* instance,ParsedInst instruction){
    memset(instance->frame,0,64*32);
    return (Signal){
        .type=SIGNAL_DRAW
    };
}
static Signal FRET(Chip8* instance,ParsedInst instruction){
    if(instance->stackpointer<1) {
        printf("No return adress on stack/n");
        return (Signal){
            .type=SIGNAL_END
        };
    }
    instance->counter=instance->stack[--instance->stackpointer];
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FJP(Chip8* instance,ParsedInst instruction){
    uint16_t jumpadress=0;
    if(instruction.op1.type==REGISTER) jumpadress=(*instruction.op1.value.byte+*instruction.op2.value.adress);
    else jumpadress=(*instruction.op1.value.adress);
    if(jumpadress>4094) {
        printf("Outside memory range\n");
        return (Signal){
            .type=SIGNAL_END
        };
    }
    instance->counter=jumpadress-2;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };

}
static Signal FCALL(Chip8* instance,ParsedInst instruction){
    if(*instruction.op1.value.adress>4094){
        printf("Outside memory range\n");
        return (Signal){
            .type=SIGNAL_END
        };
    }
    instance->stack[instance->stackpointer++]=instance->counter;
    instance->counter=*instruction.op1.value.adress-2;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSE(Chip8* instance,ParsedInst instruction){
    if(*instruction.op1.value.byte==*instruction.op2.value.byte) {
        instance->counter+=2;
    }
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSNE(Chip8* instance,ParsedInst instruction){
    if(*instruction.op1.value.byte!=*instruction.op2.value.byte) {
        instance->counter+=2;
    }
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FLD (Chip8* instance,ParsedInst instruction){
if(instruction.op1.type==REGISTER && (instruction.op2.type!=KEY && instruction.op2.type!=ADRESSREFERENCE)){
    *instruction.op1.value.byte=*instruction.op2.value.byte;
}
else if(instruction.op1.type==ADRESSREG && instruction.op2.type==ADRESS){
    *instruction.op1.value.adress=*instruction.op2.value.adress;
}
else if(instruction.op2.type==KEY){
    return (Signal){
        .type=SIGNAL_PAUSE
    };
}
else if(instruction.op1.type==SPRITE){
    if(*instruction.op2.value.byte>15){
        printf("No supported digit\n");
        return (Signal){
            .type=SIGNAL_END
        };

    }
    *instruction.op1.value.adress=(*instruction.op2.value.byte)*5;
}
else if(instruction.op1.type==BCD){
    if(instance->I+2>4095){
        printf("Outside memory range\n");
        return (Signal){
            .type=SIGNAL_END
        };
    }
    uint8_t firstdigit=*instruction.op2.value.byte/100;
    uint8_t lastdigit=*instruction.op2.value.byte%10;
    uint8_t middledigit=(*instruction.op2.value.byte/10)%10;
    instance->memory[instance->I]=firstdigit;
    instance->memory[instance->I+1]=middledigit;
    instance->memory[instance->I+2]=lastdigit;
}
else if(instruction.op1.type==ADRESSREFERENCE){
    size_t lastregister=instruction.op2.value.byte-&instance->V[0]; //add memory check. too lazy right now
    for(size_t i=0;i<=lastregister;i++){
        instance->memory[instance->I+i]=instance->V[i];
    }
}
else if(instruction.op2.type==ADRESSREFERENCE){
    size_t lastregister=instruction.op1.value.byte-&instance->V[0]; //add a memory check
    for(size_t i=0;i<=lastregister;i++){
        instance->V[i]=instance->memory[instance->I+i];
    }
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
return (Signal){
    .type=SIGNAL_CONTINUE
};
}
static Signal FADD(Chip8* instance,ParsedInst instruction){
    if(instruction.op1.type==ADRESSREG){
        *instruction.op1.value.adress+=*instruction.op2.value.byte;
    }
    else {
        uint16_t carry=*instruction.op1.value.byte+*instruction.op2.value.byte;
        *instruction.op1.value.byte+=*instruction.op2.value.byte;
        if(carry>255){
            instance->V[0xF]=1;
        }
        else {
            instance->V[0xF]=0;
        }
    }
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FOR(Chip8* instance,ParsedInst instruction){
    *instruction.op1.value.byte|=*instruction.op2.value.byte;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FAND(Chip8* instance,ParsedInst instruction){
    *instruction.op1.value.byte&=*instruction.op2.value.byte;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FXOR(Chip8* instance,ParsedInst instruction){
    *instruction.op1.value.byte^=*instruction.op2.value.byte;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSUB(Chip8* instance,ParsedInst instruction){
    uint8_t set;
    if(*instruction.op1.value.byte>*instruction.op2.value.byte){
        set=1;
    }
    else {
        set=0;
    }
    *instruction.op1.value.byte-=*instruction.op2.value.byte;
    instance->V[0xf]=set;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSHR(Chip8* instance,ParsedInst instruction){
    uint8_t set=*instruction.op1.value.byte&0x01;
    *instruction.op1.value.byte>>=1;
    instance->V[0xf]=set;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSUBN(Chip8* instance,ParsedInst instruction){
    uint8_t sub=*instruction.op2.value.byte-*instruction.op1.value.byte;
    *instruction.op1.value.byte=sub;
    if(*instruction.op1.value.byte<*instruction.op2.value.byte){
        instance->V[0xF]=1;
    }
    else {
        instance->V[0xF]=0;
    }
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FSHL(Chip8* instance,ParsedInst instruction){
    uint8_t set=(!!(*instruction.op1.value.byte&0x80));
    *instruction.op1.value.byte<<=1;
    instance->V[0xf]=set;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };
}
static Signal FRND(Chip8* instance,ParsedInst instruction){
    uint8_t random=rand()%256;
    random&=*instruction.op2.value.byte;
    *instruction.op1.value.byte=random;
    return (Signal){
        .type=SIGNAL_CONTINUE
    };

}
static Signal FDRW(Chip8* instance,ParsedInst instruction){
    DrawFrame(instance,instance->frame,*instruction.op3.value.byte,*instruction.op1.value.byte,*instruction.op2.value.byte);
    return (Signal){
        .type=SIGNAL_DRAW
    };
}
static Signal FSKP(Chip8* instance,ParsedInst instruction){
    return (Signal){
        .type=SIGNAL_SKP,
        .key=*instruction.op1.value.byte
    };
}
static Signal FSKNP(Chip8* instance,ParsedInst instruction){
    return (Signal){
        .type=SIGNAL_SKNP,
        .key=*instruction.op1.value.byte
    };
}

Signal Execute(Chip8* instance,ParsedInst instruction){
    Signal (*func[])(Chip8*,ParsedInst)={
    FUNDEFINED, // 0  UNDEFINED
    FCLS,       // 1  CLS
    FRET,       // 2  RET
    FJP,        // 3  JP
    FCALL,      // 4  CALL
    FSE,        // 5  SE
    FSNE,       // 6  SNE
    FLD,        // 7  LD
    FOR, // 8  OR
    FAND,       // 9  AND
    FXOR,       // 10 XOR
    FADD,       // 11 ADD
    FSUB,       // 12 SUB
    FSHR,       // 13 SHR
    FSUBN,      // 14 SUBN
    FSHL,       // 15 SHL
    FRND,       // 16 RND
    FDRW,       // 17 DRW
    FSKP,       // 18 SKP
    FSKNP,      // 19 SKNP
};
    return func[instruction.code](instance,instruction);

}