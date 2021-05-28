#include"../../defns.h"

/*
 * Single data transfer module: 
 * Decodes and executes a 32 bit single data transfer instruction.
 */

extern void execute_single_data_transfer(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t* memory);
