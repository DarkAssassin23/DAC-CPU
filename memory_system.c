#include <stdio.h>
#include <string.h>
#include "memory_system.h"
#include "memory.h"
#include "decoder.h"

void split_address(int address, unsigned char *lower, unsigned char *upper)
{
	*lower = address&0x1f;
	*upper = (address>>5)&0x1f;
}
void memory_store(int address, unsigned char value)
{
	unsigned char lower, upper;
	split_address(address, &lower, &upper);
	unsigned int lower_decoded = decoder(lower);
	unsigned int upper_decoded = decoder(upper);
	mem_put(upper_decoded, lower_decoded, value);
}
unsigned char memory_fetch(int address)
{
	unsigned char upper, lower;
	split_address(address, &lower, &upper);
	return mem_get(decoder(upper), decoder(lower));
}
unsigned int memory_fetch_word(int address)
{
	//unsigned char lower, upper;
	unsigned int value = 0;
	
	for(int i=address;i<address+4;i++)
	{
		//split_address(i,&lower,&upper);
		//unsigned char val = mem_get(decoder(upper), decoder(lower));
		unsigned char val = memory_fetch(i);
		value = (value << 8) | val;
		//printf("address: 0x%04x val: 0x%02x\n", i, val);
	}
	return value;
}
void memory_dump(int start_address, int num_bytes)
{
	int start = start_address - (start_address%8);
	int bound = num_bytes + (start_address - start);
	printf("start:  %d 0x%04x\n",start, start);
	for(int x=start;x<start+bound;x+=8)
	{
		printf("0x%04x (0d%04d) 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
			x,x, memory_fetch(x),memory_fetch(x+1),memory_fetch(x+2),memory_fetch(x+3),memory_fetch(x+4),memory_fetch(x+5),memory_fetch(x+6),memory_fetch(x+7));
	}
	
}
void memory_store_word(int address, unsigned int value)
{
	unsigned char lower, upper;
	int byte_pos = 24;
	for(int i=address;i<address+4;i++)
	{
		split_address(i,&lower,&upper);
		unsigned char val = (value >> byte_pos) & 0xff;
		mem_put(decoder(upper), decoder(lower), val);
		byte_pos -= 8;
	}
}
void load_memory(char *filename)
{
	/*
	 * first is the address next are values
	 */
	FILE *f = fopen(filename,"r");
	unsigned int addr;
	//unsigned char val;
	unsigned int val;
	int count = 0;
	while(1)
	{
		if(count==0)
			//fscanf(f,"%u",&addr);
			fscanf(f,"%x",&addr);
		//if(fscanf(f,"%c",&val)==EOF)
		if(fscanf(f,"%x",&val)==EOF)
			break;
		else
		{
			memory_store_word(addr,val);
			addr+=4;
		}
		count++;
	}
}
void memory_fill(int start_address, unsigned char value, int num_bytes)
{
	for(int x=start_address;x<start_address+num_bytes;x++)
		memory_store(x,value);
}
