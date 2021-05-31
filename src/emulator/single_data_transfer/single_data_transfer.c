#include <stdlib.h>
#include <stdio.h>
#include "../../defns.h"
#include "binaryString.h"
#include "common.h"

static byte_t get_Rn (byte_t secondByte) {
	return secondByte & 0xf;
}

static byte_t get_Rd (byte_t thirdByte) {
	return thirdByte >> 4;
}

/*
 * Single data transfer module: implementation.
 */ 
// Execute function:
void execute_single_data_transfer (
		byte_t *firstByte, 
		word_t *registers, 
		byte_t *memory);

// Helper functions for execute_single_data_transfer.
// Declared in descending level of abstraction.
static void execute_pre_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t *memory);

static void execute_post_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t *memory);

static void load(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory);
static void store(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory);

// Functions applying an offset to the base register.
static byte_t apply_offset(byte_t Rn, byte_t *firstByte, word_t *registers);
static void change_base_register_by_offset(
		byte_t Rn, 
		byte_t *firstByte, 
		word_t *registers);

// Decoding functions for reading 32b instruction.
static byte_t immediate_operand (byte_t firstByte);
static byte_t get_pre_post_indexing_bit(byte_t firstByte);
static byte_t get_up_bit(byte_t secondByte);
static byte_t get_load_store_bit(byte_t secondByte);
static byte_t get_shifted_register(byte_t thirdByte);
static word_t get_offset(
		byte_t thirdByte, 
		byte_t fourthByte, 
		byte_t immediate_offset, 
		word_t * registers);

// Performs single data transfer instruction execution. 
void execute_single_data_transfer (
		byte_t *firstByte, 
		word_t *registers, 
		byte_t *memory) 
{
	if (get_pre_post_indexing_bit(firstByte[0])) {

		execute_pre_indexing(firstByte, registers, memory);

	} else {

		execute_post_indexing(firstByte, registers, memory);

	}
}

// Offset added/subtracted to the base register before transferring the data.
static void execute_pre_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t *memory) 
{
	byte_t Rn = get_Rn(firstByte[1]);
	Rn = apply_offset(Rn, firstByte, registers);

	if (get_load_store_bit(firstByte[1])) {
		load(Rn, get_Rd(firstByte[2]), registers, memory);
	} else {
		store(Rn, get_Rd(firstByte[2]), registers, memory);
	}
}

// Offset added/subtracted to the register after transferring the data.	
static void execute_post_indexing(
		byte_t *firstByte, 
		word_t *registers,
		byte_t *memory) 
{
	byte_t Rn = get_Rn(firstByte[1]);
	Rn = registers[Rn];

	if (get_load_store_bit(firstByte[1])) {
		load(Rn, get_Rd(firstByte[2]), registers, memory);
	} else {
		store(Rn, get_Rd(firstByte[2]), registers, memory);
	}

	Rn = apply_offset(Rn, firstByte, registers);

	change_base_register_by_offset(Rn, firstByte, registers);
}

static void load(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory) {
	word_t loadWord = 0;
	// load entire word at memory[Rn] in Big Endian
	for (int i = 3; i >= 0; i--) {
		loadWord = loadWord << 8;
		loadWord |= memory[Rn + i];
	}
	registers[Rd] = loadWord;
}

static void store(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory) {
	word_t storeWord = registers[Rd];
	// store word to little Endian
	for (int i = 0; i < 4; i++) {
		memory[Rn + i] = storeWord;
		storeWord >>= 8;
	}
	// registers[Rd] = loadWord;

	// memory[registers[Rd]] = registers[Rn];
}

// Used in case of pre-indexing. Offset is applied and new register address is returned.
// The original register remains unchanged.
static byte_t apply_offset(byte_t Rn, byte_t *firstByte, word_t *registers) {
	// Offset is added when Up bit is set.
	if (get_up_bit(firstByte[1])) {

		Rn = registers[Rn] + get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers);	

	} else {

		Rn = registers[Rn] - get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers);	

	}
	return Rn;
}

static void change_base_register_by_offset(
		byte_t Rn, 
		byte_t *firstByte, 
		word_t *registers) 
{
	// Offset is added when Up bit is set.
	if (get_up_bit(firstByte[1])) {

		registers[Rn] = registers[Rn] + get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers);

	} else {

		registers[Rn] = registers[Rn] -   
				get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers);

	}
}



static byte_t immediate_operand (byte_t firstByte) {
	return (firstByte & 2) >> 1;
}

static byte_t get_pre_post_indexing_bit(byte_t firstByte) {
	return (firstByte & 1);	
}

static byte_t get_up_bit(byte_t secondByte) {
	return secondByte & 0x80;
}

static byte_t get_load_store_bit(byte_t secondByte) {
	return secondByte & readBinary("00010000");
}

static byte_t get_shifted_register(byte_t thirdByte) {
	return get_Second_Nibble(thirdByte);
}

static word_t get_offset (byte_t thirdByte, byte_t fourthByte, 
		byte_t immediate_offset, word_t *registers) {
	
	bit_t carry = 0;
	// Operand2 immediate value
	if (!(immediate_offset)) {
		byte_t rotation = (thirdByte & readBinary("1111")) << 1; 
		return shifter (3, rotation, fourthByte, &carry);
	}

	// Operand2 register
	else {		
		byte_t Rm = fourthByte & readBinary("1111");

		// The shift is specified by the second half of the thirdByte and the 
		// first half of the fourthByte.

		byte_t shift = ((thirdByte & readBinary("1111")) << 4) | (fourthByte >> 4);
		byte_t shiftType = (shift & readBinary("110")) >> 1;

		word_t wordToShift = registers[Rm];

		if (!(shift & 1)) { // Bit 4 is 0: shift by a constant.
			byte_t integer = shift >> 3;
			return shifter (shiftType, integer, wordToShift, &carry); 
		} else {
			// Bit 4 is 1: shift by a specified register.
			//(optional)
			byte_t Rs = get_shifted_register (thirdByte);
			return shifter (shiftType, registers[Rs], wordToShift, &carry);
		}
	}
}