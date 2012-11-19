#ifndef _VM_H_
#define _VM_H_

#include "opcodes.h"
#include <stdint.h>
#include <setjmp.h>

#define PTR_REGS 4
#define DAT_REGS 3

typedef uint16_t vm_addr_t;

typedef union
{
    struct {
        uint16_t unused;
        uint16_t data;
    };
    
    struct
    {
        uint16_t opc;
        uint8_t op2;
        uint8_t op1;
    };
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
    vm_word_t acc; // accumulator
    vm_word_t psw; // process status word
    
	vm_word_t data[DAT_REGS]; // data registers
    vm_addr_t ptr[PTR_REGS]; // Pointer registers
    
    vm_addr_t pc;  // program counter
    vm_addr_t sp;  // stack pointer (to top of stack!)
} vm_pcb_t;

typedef struct _vm_process_t
{
    vm_flags_t flags; // Process state
    
    uint8_t pid;
    vm_addr_t base_addr;
    
    vm_pcb_t reg;
    
    struct _vm_process_t* next;
} vm_process_t;

typedef struct
{
	vm_word_t ir; // instruction register

    uint32_t ic;

    vm_flags_t flags; // VM state
	vm_word_t* ram;
    uint32_t ram_size;
    uint32_t timeslice_size;
    
    vm_process_t* current_process;
    
    vm_process_t* ready_queue;
    vm_process_t* ready_queue_end;
    
    jmp_buf eh;
    const char* error;
} vm_t;

vm_process_t* vm_spawn(vm_t* vm, vm_addr_t base_addr);

vm_t* vm_init(uint32_t ram_size);
void vm_close(vm_t* vm);

void vm_run(vm_t*);


#endif // _VM_H_

