// decoder.c

#include "decoder.h"

/**
 * Converts a number into its bit position value
 * Examples:
 *  2 is converted to 0b00000000000000000000000000000100
 *  7 is converted to 0b00000000000000000000000010000000
 * 31 is converted to 0b10000000000000000000000000000000
 * Since it returns an int, this function works with 5 bits numbers
 */ 

unsigned int decoder(unsigned char b) {
    
    return 1 << (b & DECODER_BITMASK);

}
