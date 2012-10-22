#ifndef _VM_H_
#define _VM_H_

#include "opcodes.h"
#include <stdint.h>
#include <setjmp.h>

#define PTR_REGS 4
#define DAT_REGS 3

typedef uint8_t vm_addr_t;

typedef struct
{
	uint16_t opc;
	uint8_t op2;
	uint8_t op1;
} vm_instruction_t;

typedef struct
{
    uint16_t unused;
    uint16_t data;
} vm_word_t;

typedef enum
{
    VM_NONE = 0,
    VM_RUN = 1,
    VM_BADINST = 2,
    VM_DBGMODE = 4,
    VM_STEPMODE = 8,
    
    VM_ERROR = 16,
} vm_flags_t;

typedef struct
{
	vm_instruction_t ir; // instruction register
    vm_word_t acc; // accumulator
    vm_word_t psw; // process status word

    vm_flags_t flags; // VM state
	vm_word_t* ram;
    uint32_t ram_size;
    
	vm_word_t dreg[DAT_REGS]; // data registers
    vm_addr_t preg[PTR_REGS]; // Pointer registers
    
    vm_addr_t pc;  // program counter
    vm_addr_t sp;  // stack pointer
    vm_addr_t op;  // offset register
    
    jmp_buf eh;
    const char* error;
} vm_t;


vm_t* vm_init(uint32_t ram_size);
void vm_close(vm_t* vm);

void vm_run(vm_t*);


#endif // _VM_H_

