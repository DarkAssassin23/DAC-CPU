#include "cpu.h"
#include "memory_system.h"
#include "bit_functions.h"
#include <stdio.h>
#include <stdlib.h>

int registers[16];
int cpsr = 0;

void set_reg(int reg, int value)
{
	registers[reg] = value;
}
int get_reg(int reg)
{
	return registers[reg];
}
int get_cpsr()
{
	return cpsr;
}
void show_regs()
{
	for(int x=0;x<16;x++)
		printf("R%d: 0x%04x\n",x, registers[x]);
}
void step()
{
	int inst = memory_fetch_word(registers[PC]);
	int opcode = inst >> 24;
	printf("0x%08x\n", inst);
	int reg = 0;
	int destReg  = 0;
	int address = 0;
	switch(opcode)
	{
		case LDR:
			reg = inst >> 16 & 0xff;
			address = inst & 0xffff;
			if(address > 1023 || reg > 15)
			{
				printf("Address/Register out of bounds");
				exit(1);
			}
			registers[reg] = memory_fetch_word(address);
			printf("reg: %d, reg val: 0x%x08x, address: 0x%04x\n", reg, registers[reg], address);
			registers[PC] += 4;
			break;
		case LDI:
			reg = inst >> 16 & 0xff;
			int immediate = inst & 0xffff;
			if(reg > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[reg] = immediate;
			printf("reg: %d, reg val: 0x%x08x, immediate: 0x%04x\n", reg, registers[reg], immediate);
			registers[PC] += 4;
			break;
		case LDX:
			destReg = inst >> 16 & 0xff;
			int offset = inst >> 8 & 0xff;
			reg = inst & 0xff;
			if(destReg > 15 || reg > 1023)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = memory_fetch_word(registers[reg] + offset);
			printf("Destination reg: %d, reg val: 0x%x08x, base reg: 0x%04x offset: 0x%04x\n", destReg, registers[destReg], reg, offset);
			registers[PC] += 4;
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
			printf("reg: %d, store val: 0x%x08x, address: 0x%04x\n", reg, registers[reg], address);
			registers[PC] += 4;
			break;
		case ADD:
			destReg = inst >> 16 & 0xff;
			int reg1 = inst >> 8 & 0xff;
			int reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || reg2 > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = registers[reg1]+registers[reg2]; 
			printf("Destination reg: %d, First reg: %d, Second reg: %d sum: %d\n", destReg, reg1, reg2, registers[destReg]);
			registers[PC] += 4;
			break;
		case SUB:
			destReg = inst >> 16 & 0xff;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || reg2 > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = registers[reg1]-registers[reg2]; 
			printf("Destination reg: %d, First reg: %d, Second reg: %d diffrence: %d\n", destReg, reg1, reg2, registers[destReg]);
			registers[PC] += 4;
			break;
		case MUL:
			destReg = inst >> 16 & 0xff;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || reg2 > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = registers[reg1]*registers[reg2]; 
			printf("Destination reg: %d, First reg: %d, Second reg: %d product: %d\n", destReg, reg1, reg2, registers[destReg]);
			registers[PC] += 4;
			break;
		case DIV:
			destReg = inst >> 16 & 0xff;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || reg2 > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = registers[reg1]/registers[reg2]; 
			printf("Destination reg: %d, First reg: %d, Second reg: %d divdend: %d\n", destReg, reg1, reg2, registers[destReg]);
			registers[PC] += 4;
			break;
		case CMP:
			destReg = inst >> 16 & 0xff;
			reg1 = inst >> 8 & 0xff;
			reg2 = inst & 0xff;
			if(destReg > 15 || reg1 > 15 || reg2 > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			if(registers[reg1]==registers[reg2])
			{
				bit_set(&cpsr, Z);
				bit_clear(&cpsr, LT);
				bit_clear(&cpsr, GT);
				printf("%d is equal to %d\n",registers[reg1],registers[reg2]);
			}
			else if(registers[reg1]>registers[reg2])
			{
				bit_set(&cpsr, GT);
				bit_clear(&cpsr, Z);
				bit_clear(&cpsr, LT);
				printf("%d is greater than %d\n",registers[reg1],registers[reg2]);
			}
			else
			{
				bit_set(&cpsr, LT);
				bit_clear(&cpsr, Z);
				bit_clear(&cpsr, GT);
				printf("%d is less than %d\n",registers[reg1],registers[reg2]);
			}
			//printf("Destination reg: %d, First reg: %d, Second reg: %d divdend: %d\n", destReg, reg1, reg2, registers[destReg]);
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
			destReg = inst >> 16 & 0xff;
			reg = inst & 0xff;
			if(destReg > 15 || reg > 15)
			{
				printf("Register out of bounds");
				exit(1);
			}
			registers[destReg] = registers[reg];
			printf("Destination reg: %d, reg val: 0x%x08x, reg: 0x%04x\n", destReg, registers[destReg], reg);
			if (destReg != PC)
				registers[PC] += 4;
			break;
		case BL:
			registers[LR] = registers[PC] + 4;
			address = inst & 0xffffff;
			registers[PC] = address;
			break;
		
	}
}
void step_n(int n)
{
	
}
