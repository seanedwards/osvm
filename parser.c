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

uint8_t parse_2digit(const char* str)
{
    if (isalpha(str[0])) return str[1] - '0';
    
    return ((str[0] - '0') * 10) + (str[1] - '0');
}

void parse_pbrain_instruction(vm_instruction_t* inst, const char* str)
{
    inst->opc = parse_2digit(str);
    inst->op1 = parse_2digit(str + 2);
    inst->op2 = parse_2digit(str + 4);
}

void parse_pbrain(vm_t* vm, const char* src, size_t len)
{
    size_t linenum = 1;
    
    size_t inst = 0;
    for (size_t i = 0; i < len; ++i) {
        if (src[i] != '#') { // Parse opcode.
            parse_pbrain_instruction((vm_instruction_t*)&vm->ram[inst], &src[i]);
            i += 6;
            ++inst;
        }
        
        while (i < len && src[i] != '\n') ++i; // Fast forward over the rest of the line.
        ++linenum;
    }
}

void parse_asm(const char* src, size_t len, void* ram)
{
    printf("ASM not yet implemented.");
}


#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif
void parse_binary(vm_t* vm, const char* src, size_t len)
{
    memcpy(vm->ram, src, max(len, MAX_MEM * sizeof(vm_word_t)));
}