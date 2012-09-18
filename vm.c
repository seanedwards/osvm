#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void (*opcode_t)(vm_t* vm);
opcode_t* vm_opcodes = 0;

void i_noop(vm_t* vm)
{
}

vm_t* vm_init()
{
	vm_t* vm = (vm_t*)malloc(sizeof(vm_t*));
	vm->vm_ram = (uint8_t*)malloc(MAX_MEM);
	vm->vm_r = (uint32_t*)calloc(sizeof(uint32_t), 256);
	memset(vm->vm_r, 0, sizeof(uint32_t) * 256);


	if (vm_opcodes == 0) {
		vm_opcodes = (opcode_t*)calloc(sizeof(opcode_t), 256);
		memset(vm_opcodes, 0, sizeof(opcode_t) * 256);

		vm_opcodes[OP_NOOP] = i_noop;
	}
    
    vm->vm_flags = VM_RUN;

	return vm;
}

void vm_close(vm_t* vm)
{
	free(vm->vm_ram);
	free(vm->vm_r);
	free(vm);
}

uint32_t* vm_r(vm_t* vm, uint8_t n)
{
	return &vm->vm_r[n];
}

instruction_t* vm_i(vm_t* vm, uint8_t n)
{
	return (instruction_t*)&vm->vm_r[n];
}

uint32_t* vm_ram32(vm_t* vm, uint32_t addr)
{
	return (uint32_t*)&vm->vm_ram[addr * sizeof(uint32_t)];
}

uint8_t* vm_ram8(vm_t* vm, uint32_t addr)
{
	return (uint8_t*)&vm->vm_ram[addr * sizeof(uint8_t)];
}

instruction_t* vm_rami(vm_t* vm, uint32_t addr)
{
	return (instruction_t*)&vm->vm_ram[addr * sizeof(instruction_t)];
}

void vm_run(vm_t* vm)
{
	while (*vm_r(vm, REG_RUN)) {
		vm_fetch(vm);
		vm_exec(vm);
	}
}

vm_err vm_fetch(vm_t* vm)
{
	vm->vm_ir = vm_rami(vm, *vm_r(vm, REG_PC));
	*vm_i(vm, REG_IC) = *vm_rami(vm, *vm_r(vm, REG_NI));
	++*vm_r(vm, REG_NI);
	return ERR_NONE;
}

vm_err vm_exec(vm_t* vm)
{
	instruction_t i = *vm_i(vm, REG_IC);
	//printf("Executing %d\n", i.op);
	if (vm_opcodes[vm->vm_ir->opc] == 0)
		return ERR_BADINST;
	(*vm_opcodes[vm->vm_ir->opc])(vm);
	return ERR_NONE;
}

