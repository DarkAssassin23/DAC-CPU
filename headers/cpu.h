#pragma once

#define LDR 1
#define LDI 2
#define LDX 3
#define STR 4
#define ADD 5
#define SUB 6
#define MUL 7
#define DIV 8
#define CMP 9
#define B   10
#define BEQ 11
#define BNE 12
#define BLT 13
#define BGT 14
#define MOV 15
#define BL  16
#define SWI 17

#define Z 1
#define LT 2
#define GT 3

#define SR 13
#define LR 14
#define PC 15
#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15

static int verbose = 0;

void set_reg(int reg, int value);
int get_reg(int reg);
int get_cpsr(void);
void show_regs(void);
int step(void);
int opcodeToHex(char *opcode);
char *hexToOpcode(int hex);
