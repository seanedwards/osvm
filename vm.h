#ifndef _VM_H_
#define _VM_H_

#include "opcodes.h"
#include <stdint.h>
#include <setjmp.h>

#define PTR_REGS 4
#define DAT_REGS 3
#define SEMAPHORE_COUNT 10

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
    
    vm_word_t mr; // message register
    
    vm_addr_t pc;  // program counter
    vm_addr_t sp;  // stack pointer (to top of stack!)
} vm_pcb_t;

typedef struct _vm_process_t
{
    vm_flags_t flags; // Process state
    
    uint8_t pid;
    vm_addr_t base_addr;
    vm_addr_t addr_size;
    
    vm_pcb_t reg;
} vm_process_t;

typedef struct _vm_process_list_node_t
{
    vm_process_t* process;
    
    struct _vm_process_list_node_t* next;
    struct _vm_process_list_node_t* previous;
} vm_process_list_node_t;

typedef vm_process_list_node_t* vm_process_list_iterator_t;

typedef struct
{
    vm_process_list_node_t* first;
    vm_process_list_node_t* last;
    
    uint32_t count;
} vm_process_list_t;

typedef struct
{
    int64_t val;
    vm_process_list_t* blocked;
} vm_semaphore_t;

typedef struct
{
	vm_word_t ir; // instruction register

    uint32_t ic;

    vm_flags_t flags; // VM state
	vm_word_t* ram;
    uint32_t ram_size;
    uint32_t timeslice_size;
    
    vm_process_t* current_process;
    
    vm_process_list_t* ready_queue;
    vm_process_list_t* receive_queue;
    vm_process_list_t* send_queue;
    
    vm_semaphore_t* semaphores;
    
    jmp_buf eh;
    const char* error;
} vm_t;

vm_process_t* vm_spawn(vm_t* vm, vm_addr_t base_addr, vm_addr_t addr_size);

vm_t* vm_init(uint32_t ram_size);
void vm_close(vm_t* vm);

void vm_run(vm_t*);

// Process list functions:
vm_process_list_t* vm_pl_new();
void vm_pl_delete(vm_process_list_t* list);

uint32_t vm_pl_size(vm_process_list_t* list);

void vm_pl_push(vm_process_list_t* list, vm_process_t* proc);
void vm_pl_append(vm_process_list_t* list, vm_process_t* proc);

vm_process_t* vm_pl_first(vm_process_list_t* list);
vm_process_t* vm_pl_last(vm_process_list_t* list);

void vm_pl_pop_first(vm_process_list_t* list);
void vm_pl_pop_last(vm_process_list_t* list);

vm_process_list_iterator_t vm_pl_search(vm_process_list_t* list, uint8_t pid);

void vm_pl_remove(vm_process_list_t* list, vm_process_list_iterator_t iter);

vm_process_list_iterator_t vm_pl_begin(vm_process_list_t* list);
vm_process_list_iterator_t vm_pl_end(vm_process_list_t* list);
vm_process_list_iterator_t vm_pl_next(vm_process_list_iterator_t iter);
vm_process_t* vm_pl_iter_val(vm_process_list_iterator_t iter);


#endif // _VM_H_

