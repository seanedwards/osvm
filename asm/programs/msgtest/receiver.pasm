INT GETPID
PUSH 0 # Push our PID

INT RECV_ANY
READMR

ACSTORD 42 # Write the message to memory 42

ACLOADI 42 # Push end address
SAR 2
PUSH 2

ACLOADI 42 # Push begin address
SAR 2
PUSH 2

INT PRINT

HALT

