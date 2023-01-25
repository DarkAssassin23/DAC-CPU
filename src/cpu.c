#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "memory_system.h"
#include "bit_functions.h"

int registers[16];
int cpsr = 0;
int reg = 0;
int destReg = 0;
int address = 0;
int moveType = 0;

void set_reg(int reg, int value)
{
	registers[reg] = value;
}
int get_reg(int reg)
{
	return registers[reg];
}
int get_cpsr(void)
{
	return cpsr;
}
void show_regs(void)
{
	for(int x=0;x<16;x++)
		printf("R%d: 0x%04x\n",x, registers[x]);
}

void doPrint(void)
{
    int fileDescriptor = registers[0];
    int strAddr = registers[1]+4;
    int size = registers[2];
    FILE *outstream;
    switch (fileDescriptor)
    {
    case 1:
        outstream = stdout;
        break;
    case 2:
        outstream = stderr;
        break;
    default:
        outstream = stdout;
        break;
    }

    for (int x = 0; x <= size; x++)
    {
        // Need to adjsut this to print to the appropriate file descriptor
        fprintf(outstream,"%c", memory_fetch((strAddr + x)));
    }
    printf("\n");
}

int handleSoftwareInterupt(int inst)
{
    switch (registers[7])
    {
    case 0x01:
        if(verbose)
            printf("program exited with code: %d\n", registers[0]);
        return 1;
    case 0x04:
        doPrint();
        return 0;
    default:
        break;
    }
    return 0;
}

void loadImmediate(int inst)
{
    reg = inst >> 16 & 0xff;
    int immediate = inst & 0xffff;
    if (reg > 15)
    {
        printf("Register out of bounds");
        exit(1);
    }
    registers[reg] = immediate;
    if(verbose)
        printf("reg: %d, reg val: 0x%08x, immediate: 0x%04x\n", reg, registers[reg], immediate);
    registers[PC] += 4;
}

void loadRegister(int inst)
{
    reg = inst >> 16 & 0xff;
    address = inst & 0xffff;
    if (address > 1023 || reg > 15)
    {
        printf("Address/Register out of bounds");
        exit(1);
    }
    registers[reg] = memory_fetch_word(address);
    if(verbose)
        printf("reg: %d, reg val: 0x%08x, address: 0x%04x\n", reg, registers[reg], address);
    registers[PC] += 4;
}

void loadAddress(int inst)
{
    destReg = inst >> 16 & 0xf;
    reg = inst >> 8 & 0xff;
    int offset = inst & 0xff;
    if (destReg > 15 || reg > 1023)
    {
        printf("Register out of bounds");
        exit(1);
    }
    registers[destReg] = memory_fetch_word(registers[reg] + offset);
    if(verbose)
        printf("Destination reg: %d, reg val: 0x%08x, base reg: 0x%04x offset: 0x%04x\n", destReg, registers[destReg], reg, offset);
    registers[PC] += 4;
}

void compare(int reg1, int reg2, int byNumber)
{
    if (byNumber)
    {
        if (registers[reg1] == reg2)
        {
            bit_set(&cpsr, Z);
            bit_clear(&cpsr, LT);
            bit_clear(&cpsr, GT);
            if(verbose)
                printf("%d is equal to %d\n", registers[reg1], reg2);
        }
        else if (registers[reg1]>reg2)
        {
            bit_set(&cpsr, GT);
            bit_clear(&cpsr, Z);
            bit_clear(&cpsr, LT);
            if(verbose)
                printf("%d is greater than %d\n", registers[reg1], reg2);
        }
        else
        {
            bit_set(&cpsr, LT);
            bit_clear(&cpsr, Z);
            bit_clear(&cpsr, GT);
            if(verbose)
                printf("%d is less than %d\n", registers[reg1], reg2);
        }
    }
    else
    {
        if (registers[reg1] == registers[reg2])
        {
            bit_set(&cpsr, Z);
            bit_clear(&cpsr, LT);
            bit_clear(&cpsr, GT);
            if(verbose)
                printf("%d is equal to %d\n", registers[reg1], registers[reg2]);
        }
        else if (registers[reg1]>registers[reg2])
        {
            bit_set(&cpsr, GT);
            bit_clear(&cpsr, Z);
            bit_clear(&cpsr, LT);
            if(verbose)
                printf("%d is greater than %d\n", registers[reg1], registers[reg2]);
        }
        else
        {
            bit_set(&cpsr, LT);
            bit_clear(&cpsr, Z);
            bit_clear(&cpsr, GT);
            if(verbose)
                printf("%d is less than %d\n", registers[reg1], registers[reg2]);
        }
    }
}

void handleMove(int inst)
{
    switch (moveType)
    {
    case 0:
        reg = inst & 0xff;
        if (reg > 15)
        {
            printf("Source register is out of bounds\n");
            exit(1);
        }
        registers[destReg] = registers[reg];
        if(verbose)
            printf("Destination reg: %d, reg val: 0x%08x, reg: %d\n", destReg, registers[destReg], reg);
        if (destReg != PC)
            registers[PC] += 4;
        break;
    case 1:
        reg = inst >> 16 & 0xf;
        int immediate = inst & 0xffff;
        if (reg > 15)
        {
            printf("Register out of bounds");
            exit(1);
        }
        registers[reg] = immediate;
        if(verbose)
            printf("reg: %d, reg val: 0x%08x, immediate: 0x%04x\n", reg, registers[reg], immediate);
        if (destReg != PC)
            registers[PC] += 4;
        break;
    case 2:
        loadAddress(inst);
        break;
    default:
        break;

    }

}

int step(void)
{
    int byNumber = 0;
	int inst = memory_fetch_word(registers[PC]);
	int opcode = inst >> 24;
    if(verbose)
	    printf("0x%08x\n", inst);

	switch(opcode)
	{
		case LDR:
            loadRegister(inst);
			break;
		case LDI:
            loadImmediate(inst);
			break;
		case LDX:
            loadAddress(inst);
			break;
		case STR:
			reg = inst >> 16 & 0xff;
			address = inst & 0xffff;
			if(address > 1023 || reg > 15)
			{
				printf("Address/Register out of bounds");
				exit(1);
			}
			memory_store_word(address, registers[reg]);
            if(verbose)
			    printf("reg: %d, store val: 0x%08x, address: 0x%04x\n", reg, registers[reg], address);
			registers[PC] += 4;
			break;
		case ADD:
            byNumber = inst >> 20 & 0xf;
			destReg = inst >> 16 & 0xf;
			int reg1 = inst >> 8 & 0xff;
			int reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || (reg2 > 15 && !byNumber))
			{
				printf("Register out of bounds");
				exit(1);
			}
            if (byNumber)
            {
                registers[destReg] = registers[reg1] + reg2;
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, number: %d sum: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            else
            {
                registers[destReg] = registers[reg1] + registers[reg2];
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, Second reg: %d sum: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
    		registers[PC] += 4;
			break;
		case SUB:
            byNumber = inst >> 20 & 0xf;
			destReg = inst >> 16 & 0xf;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
            if (destReg > 15 || reg1 > 15 || (reg2 > 15 && !byNumber))
			{
				printf("Register out of bounds");
				exit(1);
			}
            if (byNumber)
            {
                registers[destReg] = registers[reg1] - reg2;
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, number: %d difference: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            else
            {
                registers[destReg] = registers[reg1] - registers[reg2];
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, Second reg: %d difference: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            registers[PC] += 4;
			break;
		case MUL:
            byNumber = inst >> 20 & 0xf;
			destReg = inst >> 16 & 0xf;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
            if (destReg > 15 || reg1 > 15 || (reg2 > 15 && !byNumber))
			{
				printf("Register out of bounds");
				exit(1);
			}
            if (byNumber)
            {
                registers[destReg] = registers[reg1] * reg2;
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, number: %d product: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            else
            {
                registers[destReg] = registers[reg1] * registers[reg2];
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, Second reg: %d product: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            registers[PC] += 4;
			break;
		case DIV:
            byNumber = inst >> 20 & 0xf;
			destReg = inst >> 16 & 0xf;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
            if (destReg > 15 || reg1 > 15 || (reg2 > 15 && !byNumber))
			{
				printf("Register out of bounds");
				exit(1);
			}
            if (byNumber)
            {
                registers[destReg] = registers[reg1] / reg2;
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, number: %d quotient: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            else
            {
                registers[destReg] = registers[reg1] / registers[reg2];
                if(verbose)
                    printf("Destination reg: %d, First reg: %d, Second reg: %d quotient: %d\n", destReg, reg1, reg2, registers[destReg]);
            }
            registers[PC] += 4;
			break;
		case CMP:
            byNumber = inst >> 20 & 0xf;
			reg1 = inst >> 16 & 0xf;
            if (byNumber)
                reg2 = inst & 0xffff;
            else
                reg2 = inst >> 8 & 0xff;
            if (reg1 > 15 || (reg2 > 15 && !byNumber))
			{
				printf("Register out of bounds");
				exit(1);
			}
            compare(reg1, reg2, byNumber);
			
			registers[PC] += 4;
			break;
		case B:
			address = inst & 0xffffff;
			registers[PC] = address;
			break;
		case BEQ:
			if(bit_test(cpsr,Z))
			{
				address = inst & 0xffffff;
				registers[PC] = address;
			}
			else
				registers[PC] += 4;
			break;
		case BNE:
			if(bit_test(cpsr,Z)==0)
			{
				address = inst & 0xffffff;
				registers[PC] = address;
			}
			else
				registers[PC] += 4;
			break;
		case BLT:
			if(bit_test(cpsr,LT))
			{
				address = inst & 0xffffff;
				registers[PC] = address;
			}
			else
				registers[PC] += 4;
			break;
		case BGT:
			if(bit_test(cpsr, GT))
			{
				address = inst & 0xffffff;
				registers[PC] = address;
			}
			else
				registers[PC] += 4;
			break;
		case MOV:
            moveType = inst >> 20 & 0xf;
            destReg = inst >> 16 & 0xf;
            handleMove(inst);
			break;
		case BL:
			registers[LR] = registers[PC] + 4;
            if(verbose)
                printf("Link Register: 0x%x, Program Counter: 0x%x\n", registers[LR], registers[PC]);
			address = inst & 0xffffff;
			registers[PC] = address;
			break;

        case SWI:
            //int immediate = inst & 0xffffff;
            if (handleSoftwareInterupt(inst))
                return 1;
            registers[PC] += 4;
            break;
        default:
            registers[PC] += 4;

		
	}
    return 0;
}
int opcodeToHex(char *opcode)
{
    if (strcmp(opcode, "ldr") == 0)
        return LDR;
    else if (strcmp(opcode, "ldi") == 0)
        return LDI;
    else if (strcmp(opcode, "ldx") == 0)
        return LDX;
    else if (strcmp(opcode, "str") == 0)
        return STR;
    else if (strcmp(opcode, "add") == 0)
        return ADD;
    else if (strcmp(opcode, "sub") == 0)
        return SUB;
    else if (strcmp(opcode, "mul") == 0)
        return MUL;
    else if (strcmp(opcode, "div") == 0)
        return DIV;
    else if (strcmp(opcode, "cmp") == 0)
        return CMP;
    else if (strcmp(opcode, "b") == 0)
        return B;
    else if (strcmp(opcode, "beq") == 0)
        return BEQ;
    else if (strcmp(opcode, "bne") == 0)
        return BNE;
    else if (strcmp(opcode, "blt") == 0)
        return BLT;
    else if (strcmp(opcode, "bgt") == 0)
        return BGT;
    else if (strcmp(opcode, "mov") == 0)
        return MOV;
    else if (strcmp(opcode, "bl") == 0)
        return BL;
    else if (strcmp(opcode, "swi") == 0)
        return SWI;

    return 0;
}

char *hexToOpcode(int hex)
{
    switch (hex)
    {
    case LDR:
        return "ldr";
    case LDI:
        return "ldi";
    case LDX:
        return "ldx";
    case STR:
        return "str";
    case ADD:
        return "add";
    case SUB:
        return "sub";
    case MUL:
        return "mul";
    case DIV:
        return "div";
    case CMP:
        return "cmp";
    case B:
        return "b";
    case BEQ:
        return "beq";
    case BNE:
        return "bne";
    case BLT:
        return "blt";
    case BGT:
        return "bgt";
    case MOV:
        return "mov";
    case BL:
        return "bl";
    case SWI:
        return "swi";
    default:
        return "n/a";
    }
}

