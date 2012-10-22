#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef void (*opcode_t)(vm_t* vm);
opcode_t* vm_opcodes = 0;

void print_vm(vm_t* vm) {
    printf("[%02d %02d %02d] -> ", vm->ir.opc, vm->ir.op1, vm->ir.op2);
    
    printf("acc: %04d pc: %02d sp:%02d psw: %04d ", vm->acc.data, vm->pc, vm->sp, vm->psw.data);
    
    printf("Dx: {");
    size_t i = 0;
    for (i = 0; i < DAT_REGS; ++i) printf(" %05d", vm->dreg[i].data);
    printf(" } ");
    
    printf("Px: {");
    for (i = 0; i < PTR_REGS; ++i) printf(" %03d", vm->preg[i]);
    printf(" } ");
    
    printf("Flags: {");
    if (vm->flags & VM_RUN) printf(" RUN");
    if (vm->flags & VM_BADINST) printf(" BADINST");
    if (vm->flags & VM_DBGMODE) printf(" DBGMODE");
    if (vm->flags & VM_STEPMODE) printf(" STEPMODE");
    if (vm->flags & VM_ERROR) printf(" ERROR");
    printf(" }\n");
}

void print_ram(vm_t* vm, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        if (i % 0x0A == 0) printf("[A:%03d]", (uint16_t)i);
        if (i % 0x05 == 0) printf(" ");
        printf("%02X%04X ", (uint8_t)vm->ram[i].unused, vm->ram[i].data);
        if ((i+1) % 0x0A == 0) printf("\n");
    }
}

#define DBG_PRINTF(vm, x) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x)
#define DBG_PRINTF1(vm, x, a1) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1)
#define DBG_PRINTF2(vm, x, a1, a2) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2)
#define DBG_PRINTF3(vm, x, a1, a2, a3) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3)
#define DBG_PRINTF4(vm, x, a1, a2, a3, a4) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3, a4) 

#define PRECONDITION(vm, c, x) if (!(c)) { vm->error = x; longjmp(vm->eh, 1); }

// VM data functions
vm_word_t* vm_dreg(vm_t* vm, uint8_t reg) {
    PRECONDITION(vm, reg < DAT_REGS, "Data register out of range");
    return &vm->dreg[reg];
}
vm_addr_t* vm_preg(vm_t* vm, uint8_t reg) {
    PRECONDITION(vm, reg < PTR_REGS, "Pointer register out of range");
    return &vm->preg[reg];
}
vm_word_t* vm_ram(vm_t* vm, uint16_t addr) {
    PRECONDITION(vm, addr < vm->ram_size, "VM access violation");
    return &vm->ram[addr];
}

vm_word_t* vm_stack(vm_t* vm, size_t offset) {
    PRECONDITION(vm, offset < 50, "Stack offset out of range.");
    return vm_ram(vm, (vm->sp + vm->op) - offset);
}

/***********************************************************
 ************************ Misc Opcodes *********************
 ***********************************************************/
void i_noop(vm_t* vm) { } // OP_NOOP = 98

void i_halt(vm_t* vm) { // OP_HALT = 27
    vm->flags &= ~VM_RUN;
}

void i_syscall(vm_t* vm) { // OP_INT
    switch(vm->ir.op1) {
        case 0:
            print_ram(vm, vm_stack(vm, 1)->data, vm_stack(vm, 2)->data);
            break;
        default:
            PRECONDITION(vm, 0, "Invalid or unimplemented syscall.");
    }
}


/***********************************************************
 ******************* Accumulator Opcodes *******************
 ***********************************************************/
void i_acloadi(vm_t* vm) { // OP_ACLOADI = 3
    vm->acc.data = ((vm_word_t*)&vm->ir)->data;
}

void i_acloadr(vm_t* vm) { // OP_ACLOADR = 4
    vm->acc.data = vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;
}

void i_acloadd(vm_t* vm) { // OP_ACLOADD = 5
    vm->acc.data = vm_ram(vm, vm->ir.op1)->data;
}

void i_acstorr(vm_t* vm) { // OP_ACSTORR = 6
    vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data = vm->acc.data;
}

void i_acstord(vm_t* vm) { // OP_ACSTORD = 7
    vm_ram(vm, vm->ir.op1)->data = vm->acc.data;
}


/***********************************************************
 ****************** Ptr Register Opcodes *******************
 ***********************************************************/
void i_lpi(vm_t* vm) { // OP_LPI = 0
    *vm_preg(vm, vm->ir.op1) = vm->ir.op2;
}

void i_api(vm_t* vm) { // OP_API = 1
    *vm_preg(vm, vm->ir.op1) += vm->ir.op2;
}

void i_spi(vm_t* vm) { // OP_SPI = 2
    *vm_preg(vm, vm->ir.op1) -= vm->ir.op2;
}


/***********************************************************
 ***************** Data Register Opcodes *******************
 ***********************************************************/
void i_rstorr(vm_t* vm) { // OP_RSTORR = 8
    vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data = vm_dreg(vm, vm->ir.op2)->data;
}

void i_rstord(vm_t* vm) { // OP_RSTORD = 9
    vm_ram(vm, vm->ir.op1)->data = vm_dreg(vm, vm->ir.op2)->data;
}

void i_rloadr(vm_t* vm) { // OP_RLOADR = 10
    vm_dreg(vm, vm->ir.op1)->data = vm_ram(vm, *vm_preg(vm, vm->ir.op2))->data;
}

void i_rloadd(vm_t* vm) { // OP_RLOADD = 10
    vm_dreg(vm, vm->ir.op1)->data = vm_ram(vm, vm->ir.op2)->data;
}

void i_ramove(vm_t* vm) { // OP_RAMOVE
    vm->acc.data = vm_dreg(vm, vm->ir.op1)->data;
}

void i_armove(vm_t* vm) { // OP_ARMOVE
    vm_dreg(vm, vm->ir.op1)->data = vm->acc.data;
}

void i_lar(vm_t* vm) { // OP_LAR
    vm->acc.data = vm_dreg(vm, vm->ir.op1)->data;
}

void i_sar(vm_t* vm) { // OP_SAR
    vm_dreg(vm, vm->ir.op1)->data = vm->acc.data;
}


/***********************************************************
 ******************** Arithmetic Opcodes *******************
 ***********************************************************/
void i_addi(vm_t* vm) { // OP_ADDI = 12
    vm->acc.data += ((vm_word_t*)&vm->ir)->data;
}

void i_addr(vm_t* vm) { // OP_ADDR = 14
    vm->acc.data += vm_dreg(vm, vm->ir.op1)->data;
}

void i_addmr(vm_t* vm) { // OP_ADDMR = 16
    vm->acc.data += vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;
}

void i_addmd(vm_t* vm) { // OP_ADDMD = 17
    vm->acc.data += vm_ram(vm, vm->ir.op1)->data;
}


void i_subi(vm_t* vm) { // OP_SUBI = 13
    vm->acc.data -= ((vm_word_t*)&vm->ir)->data;
}

void i_subr(vm_t* vm) { // OP_SUBR = 15
    vm->acc.data -= vm_dreg(vm, vm->ir.op1)->data;
}

void i_submr(vm_t* vm) { // OP_SUBMR = 18
    vm->acc.data += vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;
}

void i_submd(vm_t* vm) { // OP_SUBMD = 19
    vm->acc.data += vm_ram(vm, vm->ir.op1)->data;
}


void i_muli(vm_t* vm) { // OP_MULI
    vm->acc.data *= ((vm_word_t*)&vm->ir)->data;
}

void i_mulr(vm_t* vm) { // OP_MULR
    vm->acc.data *= vm_dreg(vm, vm->ir.op1)->data;
}

void i_mulmr(vm_t* vm) { // OP_MULMR
    vm->acc.data *= vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;
}

void i_mulmd(vm_t* vm) { // OP_MULMD
    vm->acc.data *= vm_ram(vm, vm->ir.op1)->data;
}


void i_divi(vm_t* vm) { // OP_DIVI
    vm->acc.data /= ((vm_word_t*)&vm->ir)->data;
}

void i_divr(vm_t* vm) { // OP_DIVR
    vm->acc.data /= vm_dreg(vm, vm->ir.op1)->data;
}

void i_divmr(vm_t* vm) { // OP_DIVMR
    vm->acc.data /= vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;
}

void i_divmd(vm_t* vm) { // OP_DIVMD
    vm->acc.data /= vm_ram(vm, vm->ir.op1)->data;
}



/***********************************************************
 ******************** Comparison Opcodes *******************
 ***********************************************************/
void i_cmpeqr(vm_t* vm) { // OP_CMPEQR = 20
    vm->psw.data &= ~0x01;
    if (vm->acc.data == vm_dreg(vm, vm->ir.op1)->data)
        vm->psw.data |= 0x01;
}

void i_cmpltr(vm_t* vm) { // OP_CMPLTR = 21
    vm->psw.data &= ~0x01;
    if (vm->acc.data < vm_dreg(vm, vm->ir.op1)->data)
        vm->psw.data |= 0x01;
}

void i_cmpeqi(vm_t* vm) { // OP_CMPEQI = 22
    vm->psw.data &= ~0x01;
    if (vm->acc.data == ((vm_word_t*)&vm->ir)->data)
        vm->psw.data |= 0x01;
}

void i_cmplti(vm_t* vm) { // OP_CMPLTI = 23
    vm->psw.data &= ~0x01;
    if (vm->acc.data < ((vm_word_t*)&vm->ir)->data)
        vm->psw.data |= 0x01;
}



/***********************************************************
 ********************** Branch Opcodes *********************
 ***********************************************************/
void i_brc(vm_t* vm) { // OP_BRC = 24
    if ((vm->psw.data & 0x01) != 0) {
        vm->pc = vm->ir.op1;
    }
}

void i_brf(vm_t* vm) { // OP_BRC = 25
    if ((vm->psw.data & 0x01) == 0) {
        vm->pc = vm->ir.op1;
    }
}

void i_bru(vm_t* vm) { // OP_BRU = 26
    vm->pc = vm->ir.op1;
}


/***********************************************************
 *********************** Stack Opcodes *********************
 ***********************************************************/
void i_push(vm_t* vm) { // OP_PUSH
    vm_ram(vm, vm->sp + vm->op++)->data = vm_dreg(vm, vm->ir.op1)->data;
}

void i_pop(vm_t* vm) { // OP_POP
    vm_dreg(vm, vm->ir.op1)->data = vm_ram(vm, vm->sp + vm->op--)->data;
}


/***********************************************************
 ********************* Extension Opcodes *******************
 ***********************************************************/
void i_printchr(vm_t* vm) { // OP_PRINTCHR
    printf("%c", (uint8_t)vm->acc.data);
}

void i_printnum(vm_t* vm) { // OP_PRINTNUM
    printf("%d", vm->acc.data);
}

void i_dbgbrk(vm_t* vm) { // OP_DBGBRK
    vm->flags |= VM_STEPMODE;
    printf("Breakpoint hit. Type '?' for debugger help.\n");
}




void vm_setup_opcodes() {
    static char initialized = 0;
    if (initialized) return;
    
    vm_opcodes = (opcode_t*)calloc(sizeof(opcode_t), 100);
    memset(vm_opcodes, 0, sizeof(opcode_t) * 100);
    
    
    // Misc opcodes
    vm_opcodes[OP_NOOP] = &i_noop;
    vm_opcodes[OP_HALT] = &i_halt;
    vm_opcodes[OP_INT] = &i_syscall;
    
    // Accumulator opcodes
    vm_opcodes[OP_ACLOADI] = &i_acloadi;
    vm_opcodes[OP_ACLOADR] = &i_acloadr;
    vm_opcodes[OP_ACLOADD] = &i_acloadd;
    vm_opcodes[OP_ACSTORR] = &i_acstorr;
    vm_opcodes[OP_ACSTORD] = &i_acstord;
    
    // Ptr register opcodes
    vm_opcodes[OP_LPI] = &i_lpi;
    vm_opcodes[OP_API] = &i_api;
    vm_opcodes[OP_SPI] = &i_spi;
    
    // Data register opcodes
    vm_opcodes[OP_RSTORR] = &i_rstorr;
    vm_opcodes[OP_RSTORD] = &i_rstord;
    vm_opcodes[OP_RLOADR] = &i_rloadr;
    vm_opcodes[OP_RLOADD] = &i_rloadd;
    
    vm_opcodes[OP_RAMOVE] = &i_ramove;
    vm_opcodes[OP_ARMOVE] = &i_armove;
    
    vm_opcodes[OP_LAR] = &i_lar;
    vm_opcodes[OP_SAR] = &i_sar;
    
    // Arithmetic opcodes
    vm_opcodes[OP_ADDI] = &i_addi;
    vm_opcodes[OP_ADDR] = &i_addr;
    vm_opcodes[OP_ADDMR] = &i_addmr;
    vm_opcodes[OP_ADDMD] = &i_addmd;
    
    vm_opcodes[OP_SUBI] = &i_subi;
    vm_opcodes[OP_SUBR] = &i_subr;
    vm_opcodes[OP_SUBMR] = &i_submr;
    vm_opcodes[OP_SUBMD] = &i_submd;
    
    vm_opcodes[OP_MULI] = &i_muli;
    vm_opcodes[OP_MULR] = &i_mulr;
    vm_opcodes[OP_MULMR] = &i_mulmr;
    vm_opcodes[OP_MULMD] = &i_mulmd;
    
    vm_opcodes[OP_DIVI] = &i_divi;
    vm_opcodes[OP_DIVR] = &i_divr;
    vm_opcodes[OP_DIVMR] = &i_divmr;
    vm_opcodes[OP_DIVMD] = &i_divmd;
    
    // Comparison opcodes
    vm_opcodes[OP_CMPEQR] = &i_cmpeqr;
    vm_opcodes[OP_CMPEQI] = &i_cmpeqi;
    vm_opcodes[OP_CMPLTR] = &i_cmpltr;
    vm_opcodes[OP_CMPLTI] = &i_cmplti;
    
    // Branch opcodes
    vm_opcodes[OP_BRC] = &i_brc;
    vm_opcodes[OP_BRF] = &i_brf;
    vm_opcodes[OP_BRU] = &i_bru;
    
    // Stack opcodes
    vm_opcodes[OP_PUSH] = &i_push;
    vm_opcodes[OP_POP] = &i_pop;
    
    // Extension opcodes
    vm_opcodes[OP_PRINTCHR] = &i_printchr;
    vm_opcodes[OP_PRINTNUM] = &i_printnum;
    vm_opcodes[OP_DBGBRK] = &i_dbgbrk;
    
    initialized = 1;
}



vm_t* vm_init(uint32_t ram_size)
{
    vm_setup_opcodes();
    
	vm_t* vm = (vm_t*)malloc(sizeof(vm_t*));
    memset(vm, 0, sizeof(vm_t));
    
    vm->ram_size = ram_size;
    vm->ram = (vm_word_t*)calloc(sizeof(vm_word_t), vm->ram_size);
    memset(vm->ram, 0, vm->ram_size);
    
    vm->sp = vm->ram_size - 50;
    
	return vm;
}

void vm_close(vm_t* vm)
{
    free(vm->ram);
	free(vm);
}


void vm_fetch(vm_t* vm)
{
    *((vm_word_t*)&vm->ir) = *vm_ram(vm, vm->pc);
}

void vm_exec(vm_t* vm)
{
	//printf("Executing %d\n", i.op);
    opcode_t opc = vm_opcodes[vm->ir.opc];
    
    PRECONDITION(vm, opc != 0, "Undefined opcode.");
    ++vm->pc; // Increment the program counter.
	(*opc)(vm);
}

void vm_run(vm_t* vm)
{
    vm->flags |= VM_RUN; // Reset flags
    
	while ((vm->flags & VM_RUN) != 0) {
        if (!setjmp(vm->eh)) {
            vm_fetch(vm);
        }
        else {
            fprintf(stderr, "Error during fetch step: %s. Cannot continue.\n", vm->error);
            vm->flags |= VM_STEPMODE | VM_ERROR;
            vm->flags &= ~VM_RUN;
        }
        
        if (!setjmp(vm->eh)) {
            vm_exec(vm);
        }
        else {
            fprintf(stderr, "Error during execute step: %s. Cannot continue.\n", vm->error);
            vm->flags |= VM_STEPMODE | VM_ERROR;
            vm->flags &= ~VM_RUN;
        }
        
        if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) {
            print_vm(vm);
        }
        
        if ((vm->flags & VM_STEPMODE) != 0) {
            char inPrompt = 1;
            while (inPrompt) {
                int chr = -1;
                printf("dbg> ");
                chr = getchar();
                if (chr == '\n') continue;
                
                switch(chr) {
                    case 'q':
                    case 'Q':
                        vm->flags &= ~VM_RUN;
                    case 'r':
                    case 'R':
                        vm->flags &= ~VM_STEPMODE;
                    case 's':
                    case 'S':
                        inPrompt = 0;
                        break;
                    case 'd':
                    case 'D':
                        printf("VM State:\n");
                        print_vm(vm);
                        printf("Memdump: \n");
                        print_ram(vm, 0, vm->ram_size);
                        break;
                        
                    default:
                        printf("Unknown command %c", chr);
                    case '?':
                    case 'h':
                    case 'H':
                        printf("Debugger usage:\n"
                               "r: Resume normal operation\n"
                               "s: Step to next instruction\n"
                               "h: Print this help message\n"
                               "q: Stop VM (equivalent to HALT instruction)\n"
                               "d: Dump RAM to standard output\n"
                               );
                        break;
                }
                
                while (getchar() != '\n');
            }
        }
	}
    
    if ((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) {
        printf("Execution complete. Final VM state: \n");
        print_vm(vm);
        print_ram(vm, 0, vm->ram_size);
    }
}

