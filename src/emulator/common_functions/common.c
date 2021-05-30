#include <stdio.h>
#include "../../defns.h"
#include "binaryString.h"


byte_t get_First_Nibble (byte_t byte) {
	return byte >> 4; 
}

byte_t get_Second_Nibble (byte_t byte) {
	return byte & readBinary("1111");
}


word_t shifter (byte_t shiftType, byte_t shiftAmount, word_t word, bit_t *carry) {
	switch (shiftType){
		case 0: // logical left
			*carry = ((word << (shiftAmount - 1)) >> 31) & 1; 
			return word << shiftAmount;
		case 1: // logical right
			*carry = ((word >> (shiftAmount - 1))) & 1; 
			return word >> shiftAmount;
		case 2: // arithmetic right
			*carry = ((word >> (shiftAmount - 1))) & 1; 
			return word / (1 << shiftAmount);
		case 3: // rotate right
			*carry = ((word >> (shiftAmount - 1))) & 1; 
			return  (word >> shiftAmount) | (word << (32 - shiftAmount));
		default: // ERROR: Should never be reached
			return -1;
	}
} 
