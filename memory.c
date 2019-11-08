#include <stdio.h>
#include "memory.h"
#include "bit_functions.h"

unsigned char mem[32][32];

unsigned char mem_get(unsigned int row_selected, unsigned int col_selected)
{
	return mem[bit_find(row_selected)][bit_find(col_selected)];
}
void mem_put(unsigned int row_selected, unsigned int col_selected, unsigned char b)
{
	mem[bit_find(row_selected)][bit_find(col_selected)] = b;
}
void mem_dump()
{
	for(int x=0;x<32;x++)
	{
		for(int i=0;i<32;i+=8)
		{
			printf("0x%04x (0d%04d) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			x*32 + i, x*32 + i, mem[x][i],mem[x][i+1],mem[x][i+2],mem[x][i+3],mem[x][i+4],mem[x][i+5],mem[x][i+6],mem[x][i+7]); 
		}
	}
}

