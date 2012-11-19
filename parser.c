//
//  parser.c
//  vm
//
//  Created by Sean Edwards on 9/19/12.
//  Copyright (c) 2012 Sean Edwards. All rights reserved.
//

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "vm.h"

const char* mnemonics[100];

typedef struct {
    vm_addr_t offset;
    const char* label;
} label_t;

label_t* labels;

uint8_t parse_2digit(const char* str)
{
    uint8_t ret = 0;
    if (isdigit(str[0])) ret += (str[0] - '0') * 10;
    if (isdigit(str[1])) ret += (str[1] - '0') * 1;
    return ret;
}

uint16_t parse_4digit(const char* str)
{
    uint16_t ret = 0;
    if (isdigit(str[0])) ret += (str[0] - '0') * 1000;
    if (isdigit(str[1])) ret += (str[1] - '0') * 100;
    if (isdigit(str[2])) ret += (str[2] - '0') * 10;
    if (isdigit(str[3])) ret += (str[3] - '0') * 1;
    return ret;
}

void parse_pbrain_instruction(vm_word_t* inst, const char* str)
{
    inst->opc = parse_2digit(str);
    
    switch (inst->opc) {
        // Parse immediate instructions as 4-digit numbers
        case 3: // ACLOADI
        case 12: // ADDI
        case 13: // SUBI
        case 22: // CMPEQI
        case 23: // CMPLTI
            inst->data = parse_4digit(str + 2);
            break;
            
        // Otherwise parse it as two 2-digit numbers
        default:
            inst->op1 = parse_2digit(str + 2);
            inst->op2 = parse_2digit(str + 4);
            break;
    }
}

void parse_pbrain(vm_t* vm, FILE* f, vm_addr_t base_addr)
{
    size_t linenum = 1;
    
    size_t inst = base_addr;
    char buf[512];
    while (fgets(buf, 512, f)) {
        if (strlen(buf) > 6 && buf[0] != '#') {
            parse_pbrain_instruction(&vm->ram[inst], buf);
            ++inst;
        }
        ++linenum;
    }
}

void init_asm() {
    memset(mnemonics, 0, sizeof(const char*) * 100);
    labels = NULL;
    
    // Misc opcodes
    mnemonics[OP_NOOP] = "NOOP";
    mnemonics[OP_HALT] = "HALT";
    mnemonics[OP_INT] = "INT";
    
    // Accumulator opcodes
    mnemonics[OP_ACLOADI] = "ACLOADI";
    mnemonics[OP_ACLOADR] = "ACLOADR";
    mnemonics[OP_ACLOADD] = "ACLOADD";
    mnemonics[OP_ACSTORR] = "ACSTORR";
    mnemonics[OP_ACSTORD] = "ACSTORD";
    
    // Ptr register opcodes
    mnemonics[OP_LPI] = "LPI";
    mnemonics[OP_API] = "API";
    mnemonics[OP_SPI] = "SPI";
    
    // Data register opcodes
    mnemonics[OP_RSTORR] = "RSTORR";
    mnemonics[OP_RSTORD] = "RSTORD";
    mnemonics[OP_RLOADR] = "RLOADR";
    mnemonics[OP_RLOADD] = "RLOADD";
    
    mnemonics[OP_LAR] = "LAR";
    mnemonics[OP_SAR] = "SAR";
    
    // Arithmetic opcodes
    mnemonics[OP_ADDI] = "ADDI";
    mnemonics[OP_ADDR] = "ADDR";
    mnemonics[OP_ADDMR] = "ADDMR";
    mnemonics[OP_ADDMD] = "ADDMD";
    
    mnemonics[OP_SUBI] = "SUBI";
    mnemonics[OP_SUBR] = "SUBR";
    mnemonics[OP_SUBMR] = "SUBMR";
    mnemonics[OP_SUBMD] = "SUBMD";
    
    mnemonics[OP_MULI] = "MULI";
    mnemonics[OP_MULR] = "MULR";
    mnemonics[OP_MULMR] = "MULMR";
    mnemonics[OP_MULMD] = "MULMD";
    
    mnemonics[OP_DIVI] = "DIVI";
    mnemonics[OP_DIVR] = "DIVR";
    mnemonics[OP_DIVMR] = "DIVMR";
    mnemonics[OP_DIVMD] = "DIVMD";
    
    // Comparison opcodes
    mnemonics[OP_CMPEQR] = "CMPEQR";
    mnemonics[OP_CMPEQI] = "CMPEQI";
    mnemonics[OP_CMPLTR] = "CMPLTR";
    mnemonics[OP_CMPLTI] = "CMPLTI";
    
    // Branch opcodes
    mnemonics[OP_BRC] = "BRC";
    mnemonics[OP_BRF] = "BRF";
    mnemonics[OP_BRU] = "BRU";
    
    // Stack opcodes
    mnemonics[OP_PUSH] = "PUSH";
    mnemonics[OP_POP] = "POP";
    
    // Extension opcodes
    mnemonics[OP_PRINTCHR] = "PRINTCHR";
    mnemonics[OP_PRINTNUM] = "PRINTNUM";
    mnemonics[OP_DBGBRK] = "DBGBRK";
}

op parse_opcode(const char* mnemonic) {
    for (int i = 0; i < 100; ++i) {
        if (mnemonics[i] == NULL) continue;
        if (strcmp(mnemonic, mnemonics[i]) == 0) return i;
    }
    return -1;
    // Error.
}

void parse_asm(vm_t* vm, FILE* f, vm_addr_t base_addr)
{
    static const char* tokens = " \t";
    static char initialized = 0;
    if (initialized == 0) {
        init_asm();
        initialized = 1;
    }
    
    char buf[512];
    while (fgets(buf, 512, f)) {
        char* tok = strtok(buf, tokens);
        while (tok) {
            if (tok[0] == '#') break; // Skip everything after comments.
            
            tok = strtok(NULL, tokens);
        }
    }
    
    printf("ASM not yet implemented.");
}


#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
void parse_binary(vm_t* vm, FILE* f, vm_addr_t base_addr)
{
    //memcpy(vm->ram, src, max(len, vm->ram_size * sizeof(vm_word_t)));
}