#include "decoding.h"
void SeperateNibbles(RawInst* CurInst){
CurInst->nibble1=CurInst->instruction[0]>>4;
CurInst->nibble2=(CurInst->nibble1<<4)^CurInst->instruction[0];
CurInst->nibble3=CurInst->instruction[1]>>4;
CurInst->nibble4=(CurInst->nibble3<<4)^CurInst->instruction[1];
}
static ParsedInst SetDefault(){ //returns a ParsedInst default value, for use in the Decoding
    ParsedInst ret;
    ret.code=UNDEFINED;
    ret.op1.type=NOTHING;
    ret.op1.value.adress=NULL;
    ret.op2.type=NOTHING;
    ret.op2.value.adress=NULL;
    ret.op3.type=NOTHING;
    ret.op3.value.adress=NULL;
    return ret;
}
static uint16_t ReadAdress(RawInst* curInst){
    uint16_t adress=0;
    adress+=curInst->nibble2<<8; 
    adress+=curInst->nibble3<<4;
    adress+=curInst->nibble4;
    return adress;
}
static uint8_t ReadByte(RawInst* curInst){
    uint8_t byte=0;
    byte+=curInst->nibble3<<4;
    byte+=curInst->nibble4;
    return byte;
}
static void ReadRegisterByte(RawInst* curInst,ParsedInst* PInst,Chip8* instance){
    instance->byteconst=ReadByte(curInst);
    PInst->op2.value.byte=&instance->byteconst;
    PInst->op1.value.byte=&instance->V[curInst->nibble2];
    PInst->op1.type=REGISTER;
    PInst->op2.type=CONSTBYTE;
}
static void Read2Register(RawInst* curInst, ParsedInst* PInst,Chip8* instance){
    PInst->op1.value.byte=&instance->V[curInst->nibble2];
    PInst->op1.type=REGISTER;
    PInst->op2.value.byte=&instance->V[curInst->nibble3];
    PInst->op2.type=REGISTER;
}
static ParsedInst Case0(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    if(curInst->nibble3==0xE && curInst->nibble4==0){
        PInst.code=CLS;
    }
    if(curInst->nibble3==0xE && curInst->nibble4==0xE){
        PInst.code=RET;
    }
    return PInst;

}
static ParsedInst Case1(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    instance->adressconst=ReadAdress(curInst);
    PInst.op1.value.adress=&instance->adressconst;
    PInst.code=JP;
    PInst.op1.type=ADRESS;
    return PInst;
    
}
static ParsedInst Case2(RawInst* curInst, Chip8* instance){
    ParsedInst Pinst=SetDefault();
    instance->adressconst=ReadAdress(curInst);
    Pinst.op1.value.adress=&instance->adressconst;
    Pinst.code=CALL;
    Pinst.op1.type=ADRESS;
    return Pinst;
}
static ParsedInst Case3(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    ReadRegisterByte(curInst,&Pinst,instance);
    Pinst.code=SE;
    return Pinst;

}
static ParsedInst Case4(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    Pinst.code=SNE;
    ReadRegisterByte(curInst,&Pinst,instance);
    return Pinst;

}
static ParsedInst Case5(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    Pinst.code=SE;
    Read2Register(curInst,&Pinst,instance);
    return Pinst;

}
static ParsedInst Case6(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    ReadRegisterByte(curInst,&Pinst,instance);
    Pinst.code=LD;
    return Pinst;

}
static ParsedInst Case7(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    ReadRegisterByte(curInst,&Pinst,instance);
    Pinst.code=ADD;
    return Pinst;

}
static ParsedInst Case8(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    Read2Register(curInst,&Pinst,instance);
    Pinst.code=LD+curInst->nibble4; //all of the instructions in the 8 family are all in order
    if(curInst->nibble4==0xE) Pinst.code=SHL;
    if(curInst->nibble4>7 && curInst->nibble4<0xE) Pinst.code=UNDEFINED;
    return Pinst;

}
static ParsedInst Case9(RawInst* curInst,Chip8* instance){
    ParsedInst Pinst=SetDefault();
    Pinst.code=SNE;
    Read2Register(curInst,&Pinst,instance);
    return Pinst;

}
static ParsedInst CaseA(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    instance->adressconst=ReadAdress(curInst);
    PInst.op2.value.adress=&instance->adressconst;
    PInst.code=LD;
    PInst.op2.type=ADRESS;
    PInst.op1.value.adress=&instance->I;
    PInst.op1.type=ADRESSREG;
    return PInst;
    
}
static ParsedInst CaseB(RawInst* curInst, Chip8* instance){
     ParsedInst PInst=SetDefault();
    instance->adressconst=ReadAdress(curInst);
    PInst.op2.value.adress=&instance->adressconst;
    PInst.code=JP;
    PInst.op2.type=ADRESS;
    PInst.op1.value.byte=&instance->V[0];
    PInst.op1.type=REGISTER;
    return PInst;
    
}
static ParsedInst CaseC(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    ReadRegisterByte(curInst,&PInst,instance);
    PInst.code=RND;
    return PInst;
}
static ParsedInst CaseD(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    Read2Register(curInst,&PInst,instance);
    PInst.code=DRW;
    instance->byteconst=curInst->nibble4;
    PInst.op3.value.byte=&instance->byteconst;
    PInst.op3.type=CONSTNIBBLE;
    return PInst;
}
static ParsedInst CaseE(RawInst* curInst, Chip8* instance){
    ParsedInst PInst=SetDefault();
    PInst.op1.value.byte=&instance->V[curInst->nibble2];
    PInst.op1.type=REGISTER;
    uint8_t temp=0;
    temp+=curInst->nibble3<<4;
    temp+=curInst->nibble4;
    if(temp==0x9E) PInst.code=SKP;
    else if(temp==0xA1) PInst.code=SKNP;
    else PInst.code=UNDEFINED;
    return PInst;
}
static ParsedInst CaseF(RawInst* curInst, Chip8* instance){
    uint8_t temp=0;
    ParsedInst PInst=SetDefault();
    temp+=curInst->nibble3<<4;
    temp+=curInst->nibble4;
    switch(temp){
        case 0x07:
        PInst.code=LD;
        PInst.op1.value.byte=&instance->V[curInst->nibble2];
        PInst.op1.type=REGISTER;
        PInst.op2.value.byte=&instance->delaytimer;
        PInst.op2.type=DELAYTIMER;
        break;
        case 0x0A:
        PInst.code=LD;
        PInst.op1.value.byte=&instance->V[curInst->nibble2];
        PInst.op1.type=REGISTER;
        PInst.op2.type=KEY; //TODO Figure out how to do inputs (which phase)
        break;
        case 0x15:
        PInst.code=LD;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        PInst.op1.value.byte=&instance->delaytimer;
        PInst.op1.type=DELAYTIMER;
        break;
        case 0x18:
        PInst.code=LD;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        PInst.op1.value.byte=&instance->soundtimer;
        PInst.op1.type=SOUND;
        break;
        case 0x1E:
        PInst.code=ADD;
        PInst.op1.value.adress=&instance->I;
        PInst.op1.type=ADRESSREG;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        break;
        case 0x29:
        PInst.code=LD;
        PInst.op1.value.adress=&instance->I;
        PInst.op1.type=SPRITE;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        break;
        case 0x33:
        PInst.code=LD;
        PInst.op1.value.adress=&instance->I;
        PInst.op1.type=BCD;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        break;
        case 0x55:
        PInst.code=LD;
        PInst.op1.value.adress=&instance->I;
        PInst.op1.type=ADRESSREFERENCE;
        PInst.op2.value.byte=&instance->V[curInst->nibble2];
        PInst.op2.type=REGISTER;
        break;
        case 0x65:
        PInst.code=LD;
        PInst.op1.value.byte=&instance->V[curInst->nibble2];
        PInst.op1.type=REGISTER;
        PInst.op2.value.adress=&instance->I;
        PInst.op2.type=ADRESSREFERENCE;
        break;

    }
    return PInst;
}


ParsedInst SwitchOpcode(RawInst* curInst, Chip8* instance ){
    ParsedInst (*func[])(RawInst*, Chip8* instance )={Case0,Case1,Case2,Case3,Case4,Case5,Case6,Case7,
    Case8,Case9,CaseA,CaseB,CaseC,CaseD,CaseE,CaseF};
    return func[curInst->nibble1](curInst,instance);
}
static void PrintRegister(Operand curOP,Chip8* instance,FILE* file){
    const uint8_t* const first=&instance->V[0];
    switch(curOP.type){
        case REGISTER:
        fprintf(file,"V%tX ",(curOP.value.byte-first)/sizeof(*curOP.value.byte));
        break;
        case ADRESS:
        fprintf(file,"0x%03X ",*curOP.value.adress);
        break;
        case CONSTBYTE:
        fprintf(file,"0x%02X ",*curOP.value.byte);
        break;
        case CONSTNIBBLE:
        fprintf(file,"0x%X ",*curOP.value.byte);
        break;
        case ADRESSREG:
        fprintf(file,"I ");
        break;
        case DELAYTIMER:
        fprintf(file,"DT ");
        break;
        case KEY:
        fprintf(file,"K ");
        break;
        case SOUND:
        fprintf(file,"ST ");
        break;
        case BCD:
        fprintf(file,"B ");
        break;
        case ADRESSREFERENCE:
        fprintf(file,"[I] ");
        break;
        case SPRITE:
        fprintf(file,"F ");
        break;
        case NOTHING:
    }
}
void PrintInstruction(ParsedInst curInst,Chip8* instance,FILE* curFile){
    static const char* instructions[]={"UNDEFINED",
    "CLS",
    "RET",
    "JP",
    "CALL",
    "SE",
    "SNE",
    "LD",
    "OR",
    "AND",
    "XOR",
    "ADD",
    "SUB",
    "SHR",
    "SUBN",
    "SHL",
    "RND",
    "DRW",
    "SKP",
    "SKNP",};
    fprintf(curFile,"%s ",instructions[curInst.code]); //print the instruction
    if(curInst.code==UNDEFINED){ 
        putc('\n',curFile);
        return;
    }
    PrintRegister(curInst.op1,instance,curFile);
    PrintRegister(curInst.op2,instance,curFile);
    PrintRegister(curInst.op3,instance,curFile);
    putc('\n',curFile);
}