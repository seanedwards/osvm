#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void (*opcode_t)(vm_t* vm);
opcode_t* vm_opcodes = 0;

#define DBG_PRINTF(vm, x) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x)
#define DBG_PRINTF1(vm, x, a1) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1)
#define DBG_PRINTF2(vm, x, a1, a2) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2)
#define DBG_PRINTF3(vm, x, a1, a2, a3) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3)
#define DBG_PRINTF4(vm, x, a1, a2, a3, a4) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3, a4)

#define DBG_PRINTOP(vm, desc) DBG_PRINTF4(vm, "PC=%02d: %s %d %d\n", vm->pc, desc, vm->ir.op1, vm->ir.op2)


void i_noop(vm_t* vm) { DBG_PRINTOP(vm, "NOOP"); }

void i_lpi(vm_t* vm) { // OP_LPI = 0
    vm->preg[vm->ir.op1] = vm->ir.op2;
    DBG_PRINTOP(vm, "LPI");
}

void i_api(vm_t* vm) { // OP_API = 1
    vm->preg[vm->ir.op1] += vm->ir.op2;
    DBG_PRINTOP(vm, "API");
}

void i_spi(vm_t* vm) { // OP_SPI = 2
    vm->preg[vm->ir.op1] -= vm->ir.op2;
    DBG_PRINTOP(vm, "SPI");
}


void i_acloadi(vm_t* vm) { // OP_ACLOADI = 3
    vm->acc.data = ((vm_word_t*)&vm->ir)->data;
    DBG_PRINTOP(vm, "ACLOADI");
}

void i_acloadr(vm_t* vm) { // OP_ACLOADR = 4
    vm->acc.data = vm->ram[vm->preg[vm->ir.op1]].data;
    DBG_PRINTOP(vm, "ACLOADR");
}

void i_acloadd(vm_t* vm) { // OP_ACLOADD = 5
    vm->acc.data = vm->ram[vm->ir.op1].data;
    DBG_PRINTOP(vm, "ACLOADD");
}

void i_acstorr(vm_t* vm) { // OP_ACSTORR = 6
    vm->ram[vm->preg[vm->ir.op1]].data = vm->acc.data;
    DBG_PRINTOP(vm, "ACSTORR");
}

void i_acstord(vm_t* vm) { // OP_ACSTORD = 7
    vm->ram[vm->ir.op1].data = vm->acc.data;
    DBG_PRINTOP(vm, "ACSTORD");
}


void i_rstorr(vm_t* vm) { // OP_RSTORR = 8
    vm->ram[vm->preg[vm->ir.op1]].data = vm->dreg[vm->ir.op2].data;
    DBG_PRINTOP(vm, "RSTORR");
}

void i_rstord(vm_t* vm) { // OP_RSTORD = 9
    vm->ram[vm->ir.op1].data = vm->dreg[vm->ir.op2].data;
    DBG_PRINTOP(vm, "RSTORD");
}

void i_rloadr(vm_t* vm) { // OP_RLOADR = 10
    vm->dreg[vm->ir.op1].data = vm->ram[vm->preg[vm->ir.op2]].data;
    DBG_PRINTOP(vm, "RLOADR");
}

void i_rloadd(vm_t* vm) { // OP_RLOADD = 10
    vm->dreg[vm->ir.op1].data = vm->ram[vm->ir.op2].data;
    DBG_PRINTOP(vm, "RLOADD");
}


void i_addi(vm_t* vm) { // OP_ADDI
    vm->acc.data += ((vm_word_t*)&vm->ir)->data;
    DBG_PRINTOP(vm, "ADDI");
}

void i_subi(vm_t* vm) { // OP_SUBI
    vm->acc.data -= ((vm_word_t*)&vm->ir)->data;
    DBG_PRINTOP(vm, "SUBI");
}

void i_addr(vm_t* vm) { // OP_ADDR
    vm->acc.data += vm->dreg[vm->ir.op1].data;
    DBG_PRINTOP(vm, "ADDR");
}

void i_subr(vm_t* vm) { // OP_SUBR
    vm->acc.data -= vm->dreg[vm->ir.op1].data;
    DBG_PRINTOP(vm, "SUBR");
}


void i_addmr(vm_t* vm) { // OP_ADDMR
    vm->acc.data += vm->ram[vm->preg[vm->ir.op1]].data;
    DBG_PRINTOP(vm, "ADDMR");
}

void i_addmd(vm_t* vm) { // OP_ADDMD
    vm->acc.data += vm->ram[vm->ir.op1].data;
    DBG_PRINTOP(vm, "ADDMD");
}

void i_submr(vm_t* vm) { // OP_SUBMR
    vm->acc.data += vm->ram[vm->preg[vm->ir.op1]].data;
    DBG_PRINTOP(vm, "SUBMR");
}

void i_submd(vm_t* vm) { // OP_SUBMD
    vm->acc.data += vm->ram[vm->ir.op1].data;
    DBG_PRINTOP(vm, "SUBMD");
}


void i_cmpeqr(vm_t* vm) { // OP_CMPEQR
    vm->psw.data &= ~0x01;
    if (vm->acc.data == vm->dreg[vm->ir.op1].data)
        vm->psw.data |= 0x01;
    DBG_PRINTOP(vm, "CMPEQR");
}

void i_cmpltr(vm_t* vm) { // OP_CMPLTR
    vm->psw.data &= ~0x01;
    if (vm->acc.data < vm->dreg[vm->ir.op1].data)
        vm->psw.data |= 0x01;
    DBG_PRINTOP(vm, "CMPLTR");
}

void i_cmpeqi(vm_t* vm) { // OP_CMPEQI
    vm->psw.data &= ~0x01;
    if (vm->acc.data == ((vm_word_t*)&vm->ir)->data)
        vm->psw.data |= 0x01;
    DBG_PRINTOP(vm, "CMPEQR");
}

void i_cmplti(vm_t* vm) { // OP_CMPLTI
    vm->psw.data &= ~0x01;
    if (vm->acc.data < ((vm_word_t*)&vm->ir)->data)
        vm->psw.data |= 0x01;
    DBG_PRINTOP(vm, "CMPLTI");
}


void i_brc(vm_t* vm) { // OP_BRC
    if (vm->psw.data & 0x01) {
        vm->pc = vm->ir.op1;
    }
    DBG_PRINTOP(vm, "BRC");
}

void i_bru(vm_t* vm) { // OP_BRU
    vm->pc = vm->ir.op1;
    DBG_PRINTOP(vm, "BRU");
}


void i_halt(vm_t* vm) { // OP_HALT
    vm->flags &= ~VM_RUN;
    DBG_PRINTOP(vm, "HALT");
}


void i_dbgbrk(vm_t* vm)
{
    vm->flags |= VM_STEPMODE;
    DBG_PRINTOP(vm, "DBGBRK");
}

vm_t* vm_init()
{
	vm_t* vm = (vm_t*)malloc(sizeof(vm_t*));
    memset(vm, 0, sizeof(vm_t));
    
    vm_opcodes = (opcode_t*)calloc(sizeof(opcode_t), 100);
    memset(vm_opcodes, 0, sizeof(opcode_t) * 100);
    
    vm_opcodes[OP_LPI] = &i_lpi;
    vm_opcodes[OP_API] = &i_api;
    vm_opcodes[OP_SPI] = &i_spi;
    
    vm_opcodes[OP_ACLOADI] = &i_acloadi;
    vm_opcodes[OP_ACLOADR] = &i_acloadr;
    vm_opcodes[OP_ACLOADD] = &i_acloadd;
    vm_opcodes[OP_ACSTORR] = &i_acstorr;
    vm_opcodes[OP_ACSTORD] = &i_acstord;
    
    vm_opcodes[OP_RSTORR] = &i_rstorr;
    vm_opcodes[OP_RSTORD] = &i_rstord;
    vm_opcodes[OP_RLOADR] = &i_rloadr;
    vm_opcodes[OP_RLOADD] = &i_rloadd;
    
    vm_opcodes[OP_ADDI] = &i_addi;
    vm_opcodes[OP_SUBI] = &i_subi;
    vm_opcodes[OP_ADDR] = &i_addr;
    vm_opcodes[OP_SUBR] = &i_subr;
    
    vm_opcodes[OP_ADDMR] = &i_addmr;
    vm_opcodes[OP_ADDMD] = &i_addmd;
    vm_opcodes[OP_SUBMR] = &i_submr;
    vm_opcodes[OP_SUBMD] = &i_submd;
    
    vm_opcodes[OP_CMPEQR] = &i_cmpeqr;
    vm_opcodes[OP_CMPEQI] = &i_cmpeqi;
    vm_opcodes[OP_CMPLTR] = &i_cmpltr;
    vm_opcodes[OP_CMPLTI] = &i_cmplti;
    
    vm_opcodes[OP_BRC] = &i_brc;
    vm_opcodes[OP_BRU] = &i_bru;
    
    vm_opcodes[OP_HALT] = &i_halt;
    
    vm_opcodes[OP_DBGBRK] = &i_dbgbrk;
    vm_opcodes[OP_NOOP] = &i_noop;
    
	return vm;
}

void vm_close(vm_t* vm)
{
	free(vm);
}


void vm_fetch(vm_t* vm)
{
    if (vm->pc >= MAX_MEM) {
        fprintf(stderr, "Error: Program counter out of range.\n");
        vm->flags |= VM_ERROR;
        return;
    }
	memcpy(&vm->ir, &vm->ram[vm->pc], sizeof(vm_instruction_t));
}

void vm_exec(vm_t* vm)
{
	//printf("Executing %d\n", i.op);
    opcode_t opc = vm_opcodes[vm->ir.opc];
    
	if (opc == 0) {
		fprintf(stderr, "Undefined opcode %d at memory location %d.\n", vm->ir.opc, vm->pc);
        vm->flags |= VM_ERROR;
        return;
    }
    ++vm->pc; // Increment the program counter.
	(*opc)(vm);
}

void vm_run(vm_t* vm)
{
    vm->flags |= VM_RUN; // Reset flags
    
	while ((vm->flags & VM_RUN) != 0) {
        vm_fetch(vm);
        if (vm->flags & VM_ERROR) {
            fprintf(stderr, "Error during fetch step. Cannot continue.\n");
            vm->flags &= ~VM_RUN;
            return;
        }
		vm_exec(vm);
        if (vm->flags & VM_ERROR) {
            fprintf(stderr, "Error during execute step. Cannot continue.\n");
            vm->flags &= ~VM_RUN;
            return;
        }
        
        if ((vm->flags & VM_STEPMODE) != 0) {
        redo_getchar:
            printf("> ");
            switch(getchar()) {
                case 'r':
                case 'R':
                    vm->flags &= ~VM_STEPMODE;
                    break;
                case '?':
                case 'h':
                case 'H':
                case '\n':
                    printf("Debugger usage:\n"
                           "r: Resume normal operation\n"
                           "h: Print this help message\n"
                           "q: Stop VM (equivalent to HALT instruction)\n"
                           );
                    goto redo_getchar;
                    break;
                case 'q':
                case 'Q':
                    vm->flags &= ~VM_RUN;
                    break;
                default:
                    break;
            }
            printf("\n");
        }
	}
}

