INT GETPID
PUSH 0 # Push our PID

LAR 0
ADDI 1
SAR 0
PUSH 0  # PID of receiver

ACLOADI 1024
SETMR
INT SEND_MSG # Send the message
HALT

