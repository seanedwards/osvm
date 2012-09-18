#ifndef _VM_H_
#define _VM_H_

#include "opcodes.h"
#include <stdint.h>

#define MAX_MEM 150 // (1 << (8 * 3))

typedef struct
{
	uint8_t opc;
	uint8_t op1;
	uint8_t op2;
} instruction_t;

typedef enum
{
	REG_ACC = 0,
	REG_R0 = 1,
	REG_R2 = 2,
	REG_R3 = 3,
	REG_COUNT32 = 4,

	REG_PR0 = 0,
	REG_PR1 = 1,
	REG_PR2 = 2,
	REG_PR3 = 3,
	REG_PC = 4,
	REG_SP = 5,
	REG_PSW = 6,
	REG_COUNT16 = 7
} vm_reg;

typedef enum
{
	ERR_NONE = 0,
	ERR_BADINST = 1
} vm_err;

typedef enum
{
    VM_NONE = 0,
    VM_RUN = 1
} vm_flags;

typedef struct
{
	uint8_t* vm_ram;
	uint32_t* vm_r; // registers
	uint16_t* vm_pr; // pointer registers
	instruction_t* vm_ir; // instruction register
    
    char vm_flags;
} vm_t;

uint32_t* vm_r32(vm_t*, uint8_t); // Fetch a value from a 32-bit register
uint16_t* vm_r16(vm_t*, uint8_t); // Fetch a value from a 16-bit register

uint32_t* vm_ram32(vm_t*, uint16_t); // Fetch a 32-bit value from memory.
uint8_t* vm_ram8(vm_t*, uint16_t); // Fetch an 8-bit value from memory.
instruction_t* vm_rami(vm_t*, uint16_t); // Fetch a 24-bit instruction from memory.

vm_t* vm_init();
void vm_close(vm_t* vm);

void vm_run(vm_t*);

vm_err vm_fetch(vm_t*);
vm_err vm_exec(vm_t*);


#endif // _VM_H_

