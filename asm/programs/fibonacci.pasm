# Initialize pointer registers
LPI 0 40
LPI 1 41
LPI 2 42

# Set M(P1) = 1
ACLOADI 1
ACSTORR 1

# Set M(P0) = 0
ACLOADI 0
ACSTORR 0
# Set M(P2) = 0
ACSTORR 2

# Set AC = M(P0)
ACLOADR 0

:loop
# If AC == 19, halt()
CMPEQI 19
BRC :halt

# AC = M(P1)
ACLOADR 1
# AC = AC + M(P2)
ADDMR 2
# R0 = M(P1)
RLOADR 0 1

# M(P1) = AC
# M(P2) = R0
ACSTORR 1
RSTORR 2 0

# count++
ACLOADR 0
ADDI 1
ACSTORR 0

# end for loop
JMP :loop

:halt
DBGBRK

ACLOADI 42 # Push end address
SAR 2
PUSH 2

ACLOADI 41 # Push begin address
SAR 2
PUSH 2

INT 0 # Call print (pops end and begin from stack)

HALT