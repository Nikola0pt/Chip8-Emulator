#ifndef TYPES
#define TYPES
#include <stdint.h>
typedef struct {
    uint8_t instruction[2];
    uint8_t nibble1;
    uint8_t nibble2;
    uint8_t nibble3;
    uint8_t nibble4;
}RawInst;
typedef enum {
    UNDEFINED,
    CLS,
    RET,
    JP,
    CALL,
    SE,
    SNE,
    LD,
    OR,
    AND,
    XOR,
    ADD,
    SUB,
    SHR,
    SUBN,
    SHL,
    RND,
    DRW,
    SKP,
    SKNP,
}Opcode; //simple opcode enum. Every opcode has been added so the need for new ones are nonexistent, but for future reference. Dont forget to change the PrintInstruction array
typedef struct{
    union {
        uint16_t* adress;
        uint8_t* byte;
    }value;
    enum {
        ADRESS,REGISTER,CONSTBYTE,NOTHING,ADRESSREG,CONSTNIBBLE,DELAYTIMER,KEY,SOUND,BCD,ADRESSREFERENCE,SPRITE
    }type;

}Operand; //A operand, that supports any operand, yet functions the same on every type. Plus can handle variants of the same insruction
typedef struct{
    Opcode code;
    Operand op1;
    Operand op2;
    Operand op3;
}ParsedInst; //executable instructions
typedef struct {
    uint16_t counter; //current position of program execution
    uint16_t programsize; //size of program
    uint8_t memory[4096]; //RAM of Chip8
    uint8_t V[16]; // 16 V registers- V1-VF
    uint16_t I; //adress register
    uint16_t stack[16];
    uint8_t stackpointer;
    uint8_t byteconst;
    uint16_t adressconst;
    uint8_t delaytimer;
    uint8_t soundtimer;
    uint8_t frame[64*32];
    uint64_t seed;
} Chip8;
typedef struct {
   enum{ SIGNAL_END,
    SIGNAL_CONTINUE,
    SIGNAL_DRAW,
    SIGNAL_PAUSE,
    SIGNAL_SKP,
    SIGNAL_SKNP} type;
    uint8_t key;
}Signal;
 //for use in Signal for no key specified
#endif