#ifndef _OPCODES_H_
#define _OPCODES_H_

typedef enum
{
	OP_LPI = 0, // Load pointer immediate: Pn <- XX
	OP_API = 1, // Add pointer immediate: Pn <- Pn + XX
	OP_SPI = 2, // Subtract pointer immediate: Pn <- Pn - XX

	OP_ACLOADI = 3, // Load accumulator immediate: AC <- XXXX
	OP_ACLOADR = 4, // Load accumulator register addressing: AC <- M(Pn)
	OP_ACLOADD = 5, // Load accumulator direct addressing: AC <- M(XX)
	OP_ACSTORR = 6, // Store accumulator register addressing: M(Pn) <- AC
	OP_ACSTORD = 7, // Store accumulator direct addressing: M(XX) <- AC

	OP_RSTORR = 8, // Store register to memory register addressing: M(Pn) <- Rn
	OP_RSTORD = 9, // Store register to memory direct addressing: M(XX) <- Rn
	OP_RLOADR = 10, // Load register from memory register addressing: Rn <- M(Pn)
	OP_RLOADD = 11, // Load register from memory direct addressing: Rn <- M(XX)

	OP_ADDI = 12, // Add accumulator immediate: AC <- AC + XXXX
	OP_ADDR = 14, // Add contents of register to accumulator: AC <- AC + Rn
	OP_ADDMR = 16, // Add accumulator register addressing: AC <- AC + M(Pn)
	OP_ADDMD = 17, // Add accumulator direct addressing: AC <- AC + M(XX)

	OP_SUBI = 13, // Subtract accumulator immediate: AC <- AC - XXXX
	OP_SUBR = 15, // Subtract contents of register from accumulator: AC <- AC - Rn	
	OP_SUBMR = 18, // Subtract from accumulator register addressing: AC <- AC - M(Pn)
	OP_SUBMD = 19, // Subtract from accumulator direct addressing: AC <- AC - M(XX)
    
    OP_MULI = 50, // Multiply accumulator immediate: AC <- AC * XXXX
	OP_MULR = 51, // Multiply contents of register with accumulator: AC <- AC * Rn
    OP_MULMR = 52, // Multiply accumulator immediate: AC <- AC * M(Px)
	OP_MULMD = 53, // Multiply contents of register with accumulator: AC <- AC * M(XX)
    
	OP_DIVI = 54, // Divide accumulator immediate: AC <- AC / XXXX
	OP_DIVR = 55, // Divide contents of register from accumulator: AC <- AC / Rn
	OP_DIVMR = 56, // Divide accumulator immediate: AC <- AC / M(Px)
	OP_DIVMD = 57, // Divide contents of register from accumulator: AC <- AC / M(XX)
    
    OP_MODI = 35, // AC % XXXX
    OP_MODR = 36, // AC <- AC % Rn
    OP_MODMR = 37, // AC <- AC / M(Px)
    OP_MODMD = 38, // AC <- AC % M(XX)

	OP_CMPEQR = 20, // Compare equal register: if AC == Rn then PSW[0] = 1 else PSW[0] = 0
	OP_CMPLTR = 21, // Compare less register : if AC <  Rn then PSW[0] = 1 else PSW[0] = 0
	OP_CMPEQI = 22, // Compare equal immediate: if AC == XXXX then PSW[0] = 1 else PSW[0] = 0
	OP_CMPLTI = 23, // Compare less immediate: if AC <  XXXX then PSW[0] = 1 else PSW[0] = 0

	OP_BRC = 24, // Branch conditional: if PSW[0] = T then PC = XX
    OP_BRF = 25, // Branch false conditional: if PSW[0] = F then PC = XX
	OP_BRU = 26, // Branch unconditional: PC = XX

    OP_HALT = 98,
	OP_INT = 27, // syscall(XX)
    
    OP_PUSH = 28, // M(SP++) <- R(XX)
    OP_POP = 29, // R(XX) <- M(SP--)
    
    OP_LAR = 30, // AC <- R(XX)
    OP_SAR = 31, // R(XX) <- AC
    
    OP_SETMR = 60, // MR <- AC
    OP_READMR = 61, // AC <- MR

	// Extensions
	OP_DBGBRK = 90,
    OP_PRINTCHR = 91, // Print character contents of accumulator
    OP_PRINTNUM = 92, // Print numeric contents of accumulator
    OP_NOOP = 99,
    
    // Syscalls (only accessible via OP_INT)
    OP_INT_PRINT = 100,
    OP_INT_GETPID = 101,
    OP_INT_SEND_MSG = 102,
    OP_INT_RECV_MSG = 103,
    OP_INT_RECV_ANY = 104,
    
    OP_INT_SEM_WAIT = 106,
    OP_INT_SEM_SIGNAL = 107,
} op;

typedef enum {
    INT_PRINT = 0,
    INT_GETPID = 1,
    INT_SEND_MSG = 2,
    INT_RECV_MSG = 3,
    INT_RECV_ANY = 4
} vm_syscall;

#endif // _OPCODES_H_

