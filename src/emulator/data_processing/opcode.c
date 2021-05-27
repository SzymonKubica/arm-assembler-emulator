#include <stdio.h>
#include "../../defns.h"
#include "binaryString.h"

#define and 0 // 0b0000
#define eor 1 // 0b0001
#define sub 2 // 0b0010
#define rsb 3 // 0b0011
#define add 4 // 0b0100
#define tst 8 // 0b1000
#define teq 9 // 0b1001
#define cmp 12// 0b1100
#define mov 13// 0b1101

#define lsl 0 //0b00
#define lsr 1 //0b01
#define asr 2 //0b10
#define ror 3 //0b11

static byte_t get_immediate_operand (byte_t firstByte) {
    return (firstbyte & 2) >> 1;
}

static byte_t get_OpCode (byte_t firstByte, byte_t secondByte) {
    return ((firstByte & 1) + secondByte) >> 4;
}

static byte_t get_Rn (byte_t secondByte) {
    return secondByte & readBinary("1111");
}

static byte_t get_Rd (byte_t thirdByte) {
    return thirdByte >> 4;
}

static byte_t get_S (byte_t secondByte) {
	return get_First_Nibble(secondByte) & 1;
}

static word_t shifter (byte_t shiftType, byte_t shiftAmount, word_t word) {
	switch (shiftType){
		case 0: // logical left
			return word << shiftAmount;
		case 1: // logical right
			return word >> shiftAmount;
		case 2: // arithmetic right
			return word /  (1 << shiftAmount);
		case 3: // rotate right
			return  (word >> shiftAmount) | (word << (32 - shiftAmount));
		default: // ERROR: Should never be reached
			return -1;
	}
} 

static unsigned short get_Operand2 (byte_t thirdByte, byte_t fourthByte, 
byte_t immediate_operand, word_t * registers) {

	// Operand2 immediate value
	if (immediate_operand) {
		byte_t rotation = (thirdByte & readBinary("1111")) << 1; 
		return shifter (3, rotation, fourthByte);
	}

	// Operand2 register
	else {		
		byte_t Rm = fourthByte & readBinary("1111");

		// The shift is specified by the second half of the thirdByte and the 
		// first half of the fourthByte.

		byte_t shift = (thirdByte & readBinary("1111") << 4) | (fourthByte >> 4);
		byte_t shiftType = shift & readBinary("110") >> 1;

		//TODO: Issue: Compilation fails because registers is out of scope in the function.
		word_t wordToShift = registers[Rm];

		if (!(shift & 1)) { // Bit 4 is 0: shift by a constant.
			byte_t integer = shift >> 3;
			return shifter (shiftType, integer, wordToShift); 
		} else {// Bit 4 is 1: shift by a specified register.
			// This part is optional?
			byte_t shiftRegister = shift >> 4;
			return shifter (shiftType, registers[shiftRegister], wordToShift);
		}
	}
}

static byte_t get_Set_Condition_Code (byte_t thirdByte) {
	return (thirdByte >> 4) & 1;
}

static void set_CPSR (word_t result, word_t *cspr) {
	// Set N-bit
	*cpsr &= 0x7fffffff;
	*cpsr |= (result >> 31) << 31;
	// Set Z-bit -- DOUBLE CHECK if-and-only-if on page VI of spec
	if(!result) {
		*cpsr |= 0x40000000;
	} else {
		*cpsr &= 0xbfffffff;
	}
	// TODO: Set C-bit
}


void execute_data_processing (byte_t *firstByte, word_t *registers) {
	word_t operand2 
		= get_Operand2(firstByte[2], firstByte[3], get_immediate_operand(firstByte[0]), registers); 
	byte_t Rn = get_Rn(firstByte[1]);
	byte_t Rd = get_Rd(firstByte[2]);
	byte_t opCode = get_OpCode(firstByte[0], firstByte[1]);
	word_t result = 0; 

	switch (opCode) {
		case and:
			registers[Rd] = registers[Rn] & operand2;
			result = registers[Rd]; 
			break;
		case eor:
			registers[Rd] = registers[Rn] ^ operand2;
			result = registers[Rd];
			break;
		case sub:
			registers[Rd] = registers[Rn] - operand2;
			result = registers[Rd]; 
			break;
		case rsb:
			registers[Rd] = operand2 - registers[Rn];
			result = registers[Rd]; 
			break;
		case add:
			registers[Rd] = registers[Rn] + operand2;
			result = registers[Rd]; 
			break;
		case tst:
			result = registers[Rn] & operand2; 
			break;
		case teq: 
			result = registers[Rn] ^ operand2; 
			break;
		case cmp:
			result = registers[Rn] - operand2; 
			break;
		case orr:
			registers[Rd] = registers[Rn] | operand2; 
			result = registers[Rd];
			break;
		case mov:
			registers[Rd] = operand2; 
			result = registers[Rd]; 
			break; 	
	}

	if (get_S(firstByte[1])) {
		set_CPSR(result, &registers[16]); 
	}
}

int main (void) {

	word_t registers[17];
	memset (registers, 0, 1);

	byte_t tb = readBinary("10001000"), fob = readBinary("10000000");
	printf ("%d \n", get_Rd(tb));
	printf ("%d \n", get_Operand2(tb, fob));

	return 0;
}
