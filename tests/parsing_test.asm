wow: LOAD r1, -2(r7)
STORE r2, -3(r5)
BEQ r7, r2, -5
RET
CALL wow
ADD r1, r2, r3
ADDI r1, r2, -5
NAND r1, r2, r5
MUL r5, r2, r7