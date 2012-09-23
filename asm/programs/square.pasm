ACLOADI 20

LPI 0 20
LPI 1 21

ACSTORR 0
SUBI 1    # loop from 19 to 0
ACSTORR 1 # store i

RLOADD 0 20

:loop
CMPEQI 0  # if i == 0
BRC :halt # break

ACLOADR 0 # load x
ADDR 0    # x += R0
ACSTORR 0 # store x

ACLOADR 1 # load i
SUBI 1    # i -= 1
ACSTORR 1
JMP :loop

:halt
HALT

