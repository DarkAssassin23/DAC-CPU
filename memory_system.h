void memory_store(int address, unsigned char value);
unsigned char memory_fetch(int address);
unsigned int memory_fetch_word(int address);
void memory_dump(int start_address, int num_bytes);
void memory_store_word(int address, unsigned int value);
void load_memory(char *filename);
void memory_fill(int start_address, unsigned char value, int num_bytes);
