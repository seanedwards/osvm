//
//  parser.h
//  vm
//
//  Created by Sean Edwards on 9/19/12.
//  Copyright (c) 2012 Sean Edwards. All rights reserved.
//

#ifndef vm_parser_h
#define vm_parser_h
#include "vm.h"
#include "string.h"
#include <stdio.h>

void parse_pbrain(vm_t* vm, FILE* f, vm_addr_t base_addr);
void parse_asm(vm_t* vm, FILE* f, vm_addr_t base_addr);
void parse_binary(vm_t* vm, FILE* f, vm_addr_t base_addr);

#endif
