#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef void (*opcode_t)(vm_t* vm);
opcode_t* vm_opcodes = 0;

#define DBG_PRINTF(vm, x) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x)
#define DBG_PRINTF1(vm, x, a1) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1)
#define DBG_PRINTF2(vm, x, a1, a2) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2)
#define DBG_PRINTF3(vm, x, a1, a2, a3) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3)
#define DBG_PRINTF4(vm, x, a1, a2, a3, a4) if((vm->flags & (VM_DBGMODE|VM_STEPMODE)) != 0) printf(x, a1, a2, a3, a4) 

#define PRECONDITION(vm, c, x) if (!(c)) { vm->error = x; longjmp(vm->eh, 1); }

// VM data functions

vm_process_t* vm_current_process(vm_t* vm) { return vm->current_process; }

uint32_t* vm_ic(vm_t* vm) { return &vm->ic; }

vm_word_t* vm_proc_acc(vm_process_t* proc) { return &proc->reg.acc; }
vm_word_t* vm_proc_psw(vm_process_t* proc) { return &proc->reg.psw; }

vm_addr_t* vm_proc_pc(vm_process_t* proc) { return &proc->reg.pc; }
vm_addr_t* vm_proc_sp(vm_process_t* proc) { return &proc->reg.sp; }

vm_word_t* vm_proc_mr(vm_process_t* proc) { return &proc->reg.mr; }

vm_word_t* vm_proc_dreg(vm_t* vm, vm_process_t* proc, uint8_t reg) {
    PRECONDITION(vm, reg < DAT_REGS, "Data register out of range");
    return &proc->reg.data[reg];
}
vm_addr_t* vm_proc_preg(vm_t* vm, vm_process_t* proc, uint8_t reg) {
    PRECONDITION(vm, reg < PTR_REGS, "Pointer register out of range");
    return &proc->reg.ptr[reg];
}
vm_word_t* vm_proc_ram(vm_t* vm, vm_process_t* proc, uint16_t addr) {
    PRECONDITION(vm, addr < vm_current_process(vm)->addr_size, "VM access violation");
    return &vm->ram[proc->base_addr + addr];
}

vm_word_t* vm_proc_stack(vm_t* vm, vm_process_t* proc, size_t offset) {
    PRECONDITION(vm, (offset + *vm_proc_sp(proc)) < vm_current_process(vm)->addr_size, "Stack offset out of range");
    return vm_proc_ram(vm, proc, *vm_proc_sp(proc) + offset);
}

void vm_proc_push(vm_t* vm, vm_process_t* proc, vm_word_t* data) {
    PRECONDITION(vm, *vm_proc_sp(proc) > 0, "VM stack overflow");
    vm_proc_ram(vm, proc, --(*vm_proc_sp(proc)))->data = data->data;
}

vm_word_t* vm_proc_pop(vm_t* vm, vm_process_t* proc) {
    PRECONDITION(vm, *vm_proc_sp(proc) < vm_current_process(vm)->addr_size, "Pop attempted on empty stack");
    return vm_proc_ram(vm, proc, (*vm_proc_sp(proc))++);
}

vm_word_t* vm_acc(vm_t* vm) { return vm_proc_acc(vm_current_process(vm)); }
vm_word_t* vm_psw(vm_t* vm) { return vm_proc_psw(vm_current_process(vm)); }

vm_addr_t* vm_pc(vm_t* vm) { return vm_proc_pc(vm_current_process(vm)); }
vm_addr_t* vm_sp(vm_t* vm) { return vm_proc_sp(vm_current_process(vm)); }

vm_word_t* vm_mr(vm_t* vm) { return vm_proc_mr(vm_current_process(vm)); }

vm_word_t* vm_dreg(vm_t* vm, uint8_t reg) { return vm_proc_dreg(vm, vm_current_process(vm), reg); }
vm_addr_t* vm_preg(vm_t* vm, uint8_t reg) { return vm_proc_preg(vm, vm_current_process(vm), reg); }
vm_word_t* vm_ram(vm_t* vm, uint16_t addr) { return vm_proc_ram(vm, vm_current_process(vm), addr); }

vm_word_t* vm_stack(vm_t* vm, size_t offset) { return vm_proc_stack(vm, vm_current_process(vm), offset); }

vm_semaphore_t* vm_semaphore(vm_t* vm, size_t idx) {
    PRECONDITION(vm, idx < SEMAPHORE_COUNT, "Semaphore index out of range");
    return &vm->semaphores[idx];
}

void vm_push(vm_t* vm, vm_word_t* data) { vm_proc_push(vm, vm_current_process(vm), data); }

vm_word_t* vm_pop(vm_t* vm) { return vm_proc_pop(vm, vm_current_process(vm)); }


// Output functions

void print_vm(vm_t* vm) {
    printf("[%02d %02d %02d] -> ", vm->ir.opc, vm->ir.op1, vm->ir.op2);
    
    if (vm_current_process(vm) != NULL) {
        printf("acc: %04d pc: %02d sp: %02d psw: %04d pid: %02d ",
               vm_acc(vm)->data,
               *vm_pc(vm),
               *vm_sp(vm),
               vm_psw(vm)->data,
               vm_current_process(vm)->pid);
        
        printf("Dx: {");
        size_t i = 0;
        for (i = 0; i < DAT_REGS; ++i) printf(" %05d", vm_dreg(vm, i)->data);
        printf(" } ");
        
        printf("Px: {");
        for (i = 0; i < PTR_REGS; ++i) printf(" %03d", *vm_preg(vm, i));
        printf(" } ");
    }
    
    vm_flags_t flags = vm->flags;
    printf("VM Flags: {");
    for (int i = 0; i < (vm_current_process(vm) ? 2 : 1); ++i) {
        if (flags & VM_RUN) printf(" RUN");
        if (flags & VM_BADINST) printf(" BADINST");
        if (flags & VM_DBGMODE) printf(" DBGMODE");
        if (flags & VM_STEPMODE) printf(" STEPMODE");
        if (flags & VM_ERROR) printf(" ERROR");
        
        if (i == 1 || vm_current_process(vm) == NULL) break;
        
        printf(" } Proc Flags: {");
        flags = vm_current_process(vm)->flags;
    }
    printf(" } \n");
}

void print_ram(vm_t* vm, size_t start, size_t end) {
    for (size_t i = start; i < end; ++i) {
        if (i % 0x0A == 0) printf("[A:%03d]", (uint16_t)i);
        if (i % 0x05 == 0) printf(" ");
        printf("%02X%04X ", (uint8_t)vm->ram[i].unused, vm->ram[i].data);
        if ((i+1) % 0x0A == 0) printf("\n");
    }
}

vm_process_t* vm_scheduler_pop(vm_t* vm);

/***********************************************************
 ************************ Misc Opcodes *********************
 ***********************************************************/
void i_noop(vm_t* vm) { } // OP_NOOP = 98
void i_halt(vm_t* vm) { /* OP_HALT = 27 */ vm_current_process(vm)->flags &= ~VM_RUN; }

void i_syscall(vm_t* vm) { /* OP_INT */
    opcode_t sysc = vm_opcodes[vm->ir.op1 + 100];
    PRECONDITION(vm, sysc != NULL, "Invalid or unimplemented syscall.");
    sysc(vm);
}


/***********************************************************
 ******************* Accumulator Opcodes *******************
 ***********************************************************/
void i_acloadi(vm_t* vm) { /* OP_ACLOADI = 3 */ vm_acc(vm)->data = vm->ir.data; }
void i_acloadr(vm_t* vm) { /* OP_ACLOADR = 4 */ vm_acc(vm)->data = vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data; }
void i_acloadd(vm_t* vm) { /* OP_ACLOADD = 5 */ vm_acc(vm)->data = vm_ram(vm, vm->ir.op1)->data; }
void i_acstorr(vm_t* vm) { /* OP_ACSTORR = 6 */ vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data = vm_acc(vm)->data; }
void i_acstord(vm_t* vm) { /* OP_ACSTORD = 7 */ vm_ram(vm, vm->ir.op1)->data = vm_acc(vm)->data; }


/***********************************************************
 ****************** Ptr Register Opcodes *******************
 ***********************************************************/
void i_lpi(vm_t* vm) { /* OP_LPI = 0 */ *vm_preg(vm, vm->ir.op1) = vm->ir.op2; }
void i_api(vm_t* vm) { /* OP_API = 1 */ *vm_preg(vm, vm->ir.op1) += vm->ir.op2; }
void i_spi(vm_t* vm) { /* OP_SPI = 2 */ *vm_preg(vm, vm->ir.op1) -= vm->ir.op2; }


/***********************************************************
 ***************** Data Register Opcodes *******************
 ***********************************************************/
void i_rstorr(vm_t* vm) { /* OP_RSTORR = 8  */ vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data = vm_dreg(vm, vm->ir.op2)->data; }
void i_rstord(vm_t* vm) { /* OP_RSTORD = 9  */ vm_ram(vm, vm->ir.op1)->data = vm_dreg(vm, vm->ir.op2)->data; }

void i_rloadr(vm_t* vm) { /* OP_RLOADR = 10 */ vm_dreg(vm, vm->ir.op1)->data = vm_ram(vm, *vm_preg(vm, vm->ir.op2))->data; }
void i_rloadd(vm_t* vm) { /* OP_RLOADD = 10 */ vm_dreg(vm, vm->ir.op1)->data = vm_ram(vm, vm->ir.op2)->data; }

void i_lar(vm_t* vm) { /* OP_LAR */ vm_acc(vm)->data = vm_dreg(vm, vm->ir.op1)->data; }
void i_sar(vm_t* vm) { /* OP_SAR */ vm_dreg(vm, vm->ir.op1)->data = vm_acc(vm)->data; }

void i_setmr(vm_t* vm) { /* OP_SETMR */ vm_mr(vm)->data = vm_acc(vm)->data; }
void i_readmr(vm_t* vm) { /* OP_READMR */ vm_acc(vm)->data = vm_mr(vm)->data; }


/***********************************************************
 ******************** Arithmetic Opcodes *******************
 ***********************************************************/
void i_addi(vm_t* vm) { /* OP_ADDI = 12 */ vm_acc(vm)->data += vm->ir.data; }
void i_addr(vm_t* vm) { /* OP_ADDR = 14 */ vm_acc(vm)->data += vm_dreg(vm, vm->ir.op1)->data; }
void i_addmr(vm_t* vm) { /* OP_ADDMR = 16 */ vm_acc(vm)->data += vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data; }
void i_addmd(vm_t* vm) { /* OP_ADDMD = 17 */ vm_acc(vm)->data += vm_ram(vm, vm->ir.op1)->data; }

void i_subi(vm_t* vm) { /* OP_SUBI = 13 */ vm_acc(vm)->data -= vm->ir.data; }
void i_subr(vm_t* vm) { /* OP_SUBR = 15 */ vm_acc(vm)->data -= vm_dreg(vm, vm->ir.op1)->data; }
void i_submr(vm_t* vm) { /* OP_SUBMR = 18 */ vm_acc(vm)->data += vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data; }
void i_submd(vm_t* vm) { /* OP_SUBMD = 19 */ vm_acc(vm)->data += vm_ram(vm, vm->ir.op1)->data; }

void i_muli(vm_t* vm) { /* OP_MULI */ vm_acc(vm)->data *= vm->ir.data; }
void i_mulr(vm_t* vm) { /* OP_MULR */ vm_acc(vm)->data *= vm_dreg(vm, vm->ir.op1)->data; }
void i_mulmr(vm_t* vm) { /* OP_MULMR */ vm_acc(vm)->data *= vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data;}
void i_mulmd(vm_t* vm) { /* OP_MULMD */ vm_acc(vm)->data *= vm_ram(vm, vm->ir.op1)->data; }

void i_divi(vm_t* vm) { /* OP_DIVI */ vm_acc(vm)->data /= vm->ir.data; }
void i_divr(vm_t* vm) { /* OP_DIVR */ vm_acc(vm)->data /= vm_dreg(vm, vm->ir.op1)->data; }
void i_divmr(vm_t* vm) { /* OP_DIVMR */ vm_acc(vm)->data /= vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data; }
void i_divmd(vm_t* vm) { /* OP_DIVMD */ vm_acc(vm)->data /= vm_ram(vm, vm->ir.op1)->data; }

void i_modi(vm_t* vm) { /* OP_MODI */ vm_acc(vm)->data %= vm->ir.data; }
void i_modr(vm_t* vm) { /* OP_MODR */ vm_acc(vm)->data %= vm_dreg(vm, vm->ir.op1)->data; }
void i_modmr(vm_t* vm) { /* OP_MODMR */ vm_acc(vm)->data %= vm_ram(vm, *vm_preg(vm, vm->ir.op1))->data; }
void i_modmd(vm_t* vm) { /* OP_MODMD */ vm_acc(vm)->data %= vm_ram(vm, vm->ir.op1)->data; }



/***********************************************************
 ******************** Comparison Opcodes *******************
 ***********************************************************/
void i_cmpeqr(vm_t* vm) { // OP_CMPEQR = 20
    vm_psw(vm)->data &= ~0x01;
    if (vm_acc(vm)->data == vm_dreg(vm, vm->ir.op1)->data) vm_psw(vm)->data |= 0x01;
}

void i_cmpltr(vm_t* vm) { // OP_CMPLTR = 21
    vm_psw(vm)->data &= ~0x01;
    if (vm_acc(vm)->data < vm_dreg(vm, vm->ir.op1)->data) vm_psw(vm)->data |= 0x01;
}

void i_cmpeqi(vm_t* vm) { // OP_CMPEQI = 22
    vm_psw(vm)->data &= ~0x01;
    if (vm_acc(vm)->data == vm->ir.data) vm_psw(vm)->data |= 0x01;
}

void i_cmplti(vm_t* vm) { // OP_CMPLTI = 23
    vm_psw(vm)->data &= ~0x01;
    if (vm_acc(vm)->data < vm->ir.data) vm_psw(vm)->data |= 0x01;
}



/***********************************************************
 ********************** Branch Opcodes *********************
 ***********************************************************/
void i_brc(vm_t* vm) { /* OP_BRC = 24 */ if ((vm_psw(vm)->data & 0x01) != 0) *vm_pc(vm) = vm->ir.op1; }
void i_brf(vm_t* vm) { /* OP_BRC = 25 */ if ((vm_psw(vm)->data & 0x01) == 0) *vm_pc(vm) = vm->ir.op1; }
void i_bru(vm_t* vm) { /* OP_BRU = 26 */ *vm_pc(vm) = vm->ir.op1; }


/***********************************************************
 *********************** Stack Opcodes *********************
 ***********************************************************/
void i_push(vm_t* vm) { /* OP_PUSH */ vm_push(vm, vm_dreg(vm, vm->ir.op1)); }
void i_pop(vm_t* vm) { /* OP_POP */ vm_dreg(vm, vm->ir.op1)->data = vm_pop(vm)->data; }


/***********************************************************
 ********************* Extension Opcodes *******************
 ***********************************************************/
void i_printchr(vm_t* vm) { /* OP_PRINTCHR */ printf("%c", (uint8_t)vm_acc(vm)->data); }
void i_printnum(vm_t* vm) { /* OP_PRINTNUM */ printf("%d", vm_acc(vm)->data); }

void i_dbgbrk(vm_t* vm) { /* OP_DBGBRK */
    vm->flags |= VM_STEPMODE;
    printf("Breakpoint hit. Type '?' for debugger help.\n");
}


/***********************************************************
 *********************** Syscall Opcodes *******************
 ***********************************************************/

void i_int_print(vm_t* vm) {
    printf("Process %d:\n", vm_current_process(vm)->pid);
    for (int i = vm_stack(vm, 0)->data; i <= vm_stack(vm, 1)->data; ++i) {
        printf("[%03d] %02X%04X\n", i, vm_ram(vm, i)->unused, vm_ram(vm, i)->data);
    }
    
    vm_pop(vm); // Pop begin address
    vm_pop(vm); // Pop end address
    
}

void i_int_getpid(vm_t* vm) { vm_dreg(vm, 0)->data = vm_current_process(vm)->pid; }

void i_int_send_msg(vm_t* vm) {
    uint8_t receiver_pid = (uint8_t)vm_stack(vm, 0)->data;
    vm_process_list_iterator_t i = vm_pl_search(vm->receive_queue, receiver_pid);
    
    if (i == vm_pl_end(vm->receive_queue) || // Receiver hasn't posted receive yet or...
        (vm_proc_stack(vm, vm_pl_iter_val(i), 0)->data != vm_current_process(vm)->pid && // Is already receiving from another proc (RECV_MSG) and...
         vm_proc_stack(vm, vm_pl_iter_val(i), 0)->data != vm_pl_iter_val(i)->pid // Is not receiving from any proc (RECV_ALL)
         ))
    {
        DBG_PRINTF3(vm, "Process %d blocked sending message (%d) to PID %d\n", vm_current_process(vm)->pid, vm_mr(vm)->data, receiver_pid);
        vm_pl_append(vm->send_queue, vm_current_process(vm));
        vm->current_process = vm_scheduler_pop(vm);
            
    } else { // Receiver will accept a message from us:
        uint8_t from_sender_pid = vm_proc_stack(vm, vm_pl_iter_val(i), 0)->data;
        uint16_t message = vm_mr(vm)->data;
        
        DBG_PRINTF3(vm, "Process %d receiving message (%d) from PID %d\n", receiver_pid, message, from_sender_pid);

        vm_proc_mr(vm_pl_iter_val(i))->data = message;
        
        if (from_sender_pid != receiver_pid) // Receiver called recv_msg
            vm_proc_pop(vm, vm_pl_iter_val(i)); // ...so we have to pop the sender's PID
        
        vm_proc_pop(vm, vm_pl_iter_val(i)); // Pop the receiver's PID
        
        vm_pl_append(vm->ready_queue, vm_pl_iter_val(i)); // Put the receiver back on the ready queue.
        vm_pl_remove(vm->receive_queue, i); // And remove it from the receive queue.

        vm_pop(vm); // pop receiver PID
        vm_pop(vm); // pop sender PID.
    }
}

void i_int_recv_msg(vm_t* vm) {
    uint8_t sender_pid = (uint8_t)vm_stack(vm, 0)->data;
    vm_process_list_iterator_t i = vm_pl_search(vm->send_queue, sender_pid);
    
    if (i == vm_pl_end(vm->send_queue) || // Sender hasn't sent the message yet or...
        vm_proc_stack(vm, vm_pl_iter_val(i), 0)->data != vm_current_process(vm)->pid // Is sending to another process
        ) {
        DBG_PRINTF2(vm, "Process %d blocked receiving message from PID %d\n", vm_current_process(vm)->pid, sender_pid);
        vm_pl_append(vm->receive_queue, vm_current_process(vm));
        vm->current_process = vm_scheduler_pop(vm);
        
    } else { // Sender already posted a message.
        vm_mr(vm)->data = vm_proc_mr(vm_pl_iter_val(i))->data;
        
        DBG_PRINTF3(vm, "Process %d received message (%d) from PID %d\n", vm_current_process(vm)->pid, vm_mr(vm)->data, sender_pid);
        
        vm_proc_pop(vm, vm_pl_iter_val(i)); // Pop sender's receiver PID
        vm_proc_pop(vm, vm_pl_iter_val(i)); // Pop sender's sender PID
        
        vm_pl_append(vm->ready_queue, vm_pl_iter_val(i)); // Put the sender back on the ready queue.
        vm_pl_remove(vm->send_queue, i); // And remove it from the send queue.
        
        vm_pop(vm); // Pop sender PID
        vm_pop(vm); // Pop receiver PID
    }
}

void i_int_recv_any(vm_t* vm) {
    vm_process_list_iterator_t i;
    
    for (i = vm_pl_begin(vm->send_queue); i != vm_pl_end(vm->send_queue); i = vm_pl_next(i)) {
        if (vm_proc_stack(vm, vm_pl_iter_val(i), 0)->data == vm_current_process(vm)->pid) { // Sender is sending to us.
            vm_mr(vm)->data = vm_proc_mr(vm_pl_iter_val(i))->data;
            
            DBG_PRINTF3(vm, "Process %d received message (%d) from PID %d\n", vm_current_process(vm)->pid, vm_mr(vm)->data, vm_pl_iter_val(i)->pid);
            
            vm_proc_pop(vm, vm_pl_iter_val(i)); // Pop sender's receiver PID
            vm_proc_pop(vm, vm_pl_iter_val(i)); // Pop sender's sender PID
            
            vm_pop(vm); // Pop receiver PID
            break;
        }
    }
    
    if (i == vm_pl_end(vm->send_queue)) { // No process was sending to us.
        DBG_PRINTF1(vm, "Process %d blocked receiving message from any sender\n", vm_current_process(vm)->pid);
        vm_pl_append(vm->receive_queue, vm_current_process(vm));
        vm->current_process = vm_scheduler_pop(vm);
    }
}

void i_int_sem_wait(vm_t* vm) {
    uint16_t sem_idx = vm_stack(vm, 0)->data;
    vm_semaphore_t* sem = vm_semaphore(vm, sem_idx);
    
    vm_pop(vm); // pop the semaphore index
    vm_pop(vm); // pop the PID, which we had to pass for who knows what reason.
    
    if (sem->val < 0) {
        DBG_PRINTF2(vm, "Process %d waiting on semaphore %d\n", vm_current_process(vm)->pid, sem_idx);
        vm_pl_append(sem->blocked, vm_current_process(vm));
        vm->current_process = vm_scheduler_pop(vm);
    }
    --sem->val;
    
    if (sem->val < 0)
        assert(-sem->val == vm_pl_size(sem->blocked) + 1);
}

void i_int_sem_signal(vm_t* vm) {
    uint16_t sem_idx = vm_stack(vm, 0)->data;
    vm_semaphore_t* sem = vm_semaphore(vm, sem_idx);
    
    vm_pop(vm); // pop the semaphore index
    vm_pop(vm); // pop the PID, which we had to pass for who knows what reason.
    
    ++sem->val;
    
    if (sem->val < 0) {
        DBG_PRINTF3(vm, "Process %d signaled by PID %d on semaphore %d\n", vm_pl_first(sem->blocked)->pid, vm_current_process(vm)->pid, sem_idx);
        vm_pl_append(vm->ready_queue, vm_pl_first(sem->blocked));
        vm_pl_pop_first(sem->blocked);
    }
    
    if (sem->val < 0)
        assert(-sem->val == vm_pl_size(sem->blocked) + 1);
}



void vm_setup_opcodes();

vm_t* vm_init(uint32_t ram_size)
{
    vm_setup_opcodes();
    
	vm_t* vm = (vm_t*)malloc(sizeof(vm_t*));
    memset(vm, 0, sizeof(vm_t));
    
    vm->ram_size = ram_size;
    vm->ram = (vm_word_t*)calloc(sizeof(vm_word_t), vm->ram_size);
    memset(vm->ram, 0, vm->ram_size);
    
    vm->ready_queue = vm_pl_new();
    vm->receive_queue = vm_pl_new();
    vm->send_queue = vm_pl_new();
    
    vm->semaphores = (vm_semaphore_t*)calloc(sizeof(vm_semaphore_t), SEMAPHORE_COUNT);
    for (uint8_t i = 0; i < SEMAPHORE_COUNT; ++i) {
        vm->semaphores->val = 0;
        vm->semaphores[i].blocked = vm_pl_new();
    }
    
	return vm;
}

void vm_close(vm_t* vm)
{
    for (uint8_t i = 0; i < SEMAPHORE_COUNT; ++i) {
        vm_pl_delete(vm->semaphores[i].blocked);
    }
    free(vm->semaphores);
    
    vm_pl_delete(vm->ready_queue);
    vm_pl_delete(vm->receive_queue);
    vm_pl_delete(vm->send_queue);
    free(vm->ram);
	free(vm);
}

void vm_fetch(vm_t* vm)
{
    vm->ir = *vm_ram(vm, *vm_pc(vm));
}

void vm_exec(vm_t* vm)
{
	//printf("Executing %d\n", i.op);
    opcode_t opc = vm_opcodes[vm->ir.opc];
    
    PRECONDITION(vm, opc != 0, "Undefined opcode.");
    ++*vm_pc(vm); // Increment the program counter.
	(*opc)(vm);
}

vm_process_t* vm_scheduler_pop(vm_t* vm)
{
    vm_process_t* ret = vm_pl_first(vm->ready_queue);
    vm_pl_pop_first(vm->ready_queue);
    *vm_ic(vm) = 0;
    return ret;
}

void vm_scheduler_push(vm_t* vm, vm_process_t* proc)
{
    vm_pl_append(vm->ready_queue, proc);
}

vm_process_t* vm_spawn(vm_t* vm, vm_addr_t base_addr, vm_addr_t addr_size)
{
    static int pid = 0;
    
    vm_process_t* proc = (vm_process_t*)malloc(sizeof(vm_process_t));
    memset(proc, 0, sizeof(vm_process_t));
    
    proc->reg.sp = 150;
    proc->flags |= VM_RUN;
    proc->pid = ++pid;
    proc->base_addr = base_addr;
    proc->addr_size = addr_size;
    
    vm_scheduler_push(vm, proc);
    
    return proc;
}

char vm_flags_test(vm_t* vm, vm_flags_t flags)
{
    return (vm->flags & flags) != 0 ||
        (vm_current_process(vm) &&
            (vm_current_process(vm)->flags & flags) != 0);
}


void vm_run(vm_t* vm)
{
    vm->flags |= VM_RUN; // Reset flags
    
	while ((vm->flags & VM_RUN) != 0) {
        if (*vm_ic(vm) > vm->timeslice_size || (vm->current_process == NULL && vm->ready_queue != NULL)) {
            // If it's still running, schedule it again.
            if (vm->current_process != NULL && vm_flags_test(vm, VM_RUN))
                vm_scheduler_push(vm, vm->current_process);
            
            vm->current_process = vm_scheduler_pop(vm);
            
            if (vm_flags_test(vm, VM_DBGMODE) && vm_current_process(vm) != NULL) {
                printf("Time slice complete. Switching to process %d.\n", vm->current_process->pid);
            }
        }
        
        // If we're out of processes to run, shut down the machine.
        if (vm_current_process(vm) == NULL)
        {
            vm->flags &= ~VM_RUN;
            continue;
        }
        
        if (!setjmp(vm->eh)) {
            vm_fetch(vm);
        }
        else {
            fprintf(stderr, "Error during fetch step: %s. Cannot continue.\n", vm->error);
            vm_current_process(vm)->flags |= VM_STEPMODE | VM_ERROR;
            vm_current_process(vm)->flags &= ~VM_RUN;
        }
        
        if (!setjmp(vm->eh)) {
            vm_exec(vm);
            ++(*vm_ic(vm)); // Increment the instruction counter.
        }
        else {
            fprintf(stderr, "Error during execute step: %s. Cannot continue.\n", vm->error);
            vm_current_process(vm)->flags |= VM_STEPMODE | VM_ERROR;
            vm_current_process(vm)->flags &= ~VM_RUN;
        }
        
        if ((vm_current_process(vm)->flags & VM_RUN) == 0) {
            if (vm_flags_test(vm, VM_STEPMODE | VM_DBGMODE))
                printf("PID %d halted.\n", vm_current_process(vm)->pid);
            
            vm_process_t* proc = vm_current_process(vm);
            vm->current_process = vm_scheduler_pop(vm);
            free(proc);
        }
        
        if(vm_flags_test(vm, VM_STEPMODE | VM_DBGMODE)) {
            print_vm(vm);
        }
        
        if (vm_flags_test(vm, VM_STEPMODE)) {
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
                        if (vm_current_process(vm) && (vm_current_process(vm)->flags & VM_STEPMODE) != 0)
                            vm_current_process(vm)->flags &= ~VM_STEPMODE;
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



/* Process list functions */

vm_process_list_t* vm_pl_new() {
    vm_process_list_t* list = (vm_process_list_t*)malloc(sizeof(vm_process_list_t));
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
    return list;
}

void vm_pl_delete(vm_process_list_t* list) {
    while (vm_pl_size(list) != 0) vm_pl_pop_first(list);
    free(list);
}

uint32_t vm_pl_size(vm_process_list_t* list) {
    return list->count;
}

void vm_pl_push(vm_process_list_t* list, vm_process_t* proc) {
    vm_process_list_node_t* node = (vm_process_list_node_t*)malloc(sizeof(vm_process_list_node_t));
    node->process = proc;
    node->previous = NULL;
    
    node->next = list->first;
    if (list->first != NULL)
        list->first->previous = node;
    
    list->first = node;
    if (list->last == NULL)
        list->last = node;
    ++list->count;
}
void vm_pl_append(vm_process_list_t* list, vm_process_t* proc) {
    vm_process_list_node_t* node = (vm_process_list_node_t*)malloc(sizeof(vm_process_list_node_t));
    node->process = proc;
    node->next = NULL;
    
    if (list->last != NULL)
        list->last->next = node;
    node->previous = list->last;
    
    list->last = node;
    if (list->first == NULL)
        list->first = node;
    ++list->count;
}

vm_process_t* vm_pl_first(vm_process_list_t* list) {
    if (vm_pl_size(list) == 0) return NULL;
    return list->first->process;
}
vm_process_t* vm_pl_last(vm_process_list_t* list) {
    if (vm_pl_size(list) == 0) return NULL;
    return list->last->process;
}

void vm_pl_pop_first(vm_process_list_t* list) {
    if (vm_pl_size(list) == 0) return;
    vm_process_list_node_t* oldfirst = list->first;
    list->first = list->first->next;
    if (list->first != NULL)
        list->first->previous = NULL;
    --list->count;
    free(oldfirst);
}
void vm_pl_pop_last(vm_process_list_t* list) {
    if (vm_pl_size(list) == 0) return;
    vm_process_list_node_t* oldlast = list->last;
    list->last = list->last->previous;
    if (list->last != NULL)
        list->last->next = NULL;
    --list->count;
    free(oldlast);
}

void vm_pl_remove(vm_process_list_t* list, vm_process_list_iterator_t iter) {
    if (iter->next) iter->next->previous = iter->previous;
    if (iter->previous) iter->previous->next = iter->next;
    --list->count;
    free(iter);
}

vm_process_list_iterator_t vm_pl_search(vm_process_list_t* list, uint8_t pid) {
    vm_process_list_node_t* node = list->first;
    while (node != NULL && node->process->pid != pid) node = node->next;
    return node;
}

vm_process_list_iterator_t vm_pl_begin(vm_process_list_t* list) {
    return list->first;
}

vm_process_list_iterator_t vm_pl_end(vm_process_list_t* list) {
    return NULL;
}

vm_process_list_iterator_t vm_pl_next(vm_process_list_iterator_t iter) {
    if (iter == NULL) return NULL;
    return iter->next;
}

vm_process_t* vm_pl_iter_val(vm_process_list_iterator_t iter) {
    if (iter == NULL) return NULL;
    return iter->process;
}


#define MAX_OPCODES 200

/* Set up opcodes */
void vm_setup_opcodes() {
    static char initialized = 0;
    if (initialized) return;
    
    vm_opcodes = (opcode_t*)calloc(sizeof(opcode_t), MAX_OPCODES);
    memset(vm_opcodes, 0, sizeof(opcode_t) * MAX_OPCODES);
    
    
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
    
    vm_opcodes[OP_LAR] = &i_lar;
    vm_opcodes[OP_SAR] = &i_sar;
    
    vm_opcodes[OP_SETMR] = &i_setmr;
    vm_opcodes[OP_READMR] = &i_readmr;
    
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
    
    vm_opcodes[OP_MODI] = &i_modi;
    vm_opcodes[OP_MODR] = &i_modr;
    vm_opcodes[OP_MODMR] = &i_modmr;
    vm_opcodes[OP_MODMD] = &i_modmd;
    
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
    
    // Syscall opcodes
    vm_opcodes[OP_INT_PRINT] = &i_int_print;
    vm_opcodes[OP_INT_GETPID] = &i_int_getpid;
    vm_opcodes[OP_INT_SEND_MSG] = &i_int_send_msg;
    vm_opcodes[OP_INT_RECV_MSG] = &i_int_recv_msg;
    vm_opcodes[OP_INT_RECV_ANY] = &i_int_recv_any;
    vm_opcodes[OP_INT_SEM_WAIT] = &i_int_sem_wait;
    vm_opcodes[OP_INT_SEM_SIGNAL] = &i_int_sem_signal;
    
    initialized = 1;
}

