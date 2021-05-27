#include "../../defns.h"
#include "binaryString.h"
#include "common.h"

byte_t get_Rn (byte_t secondByte) {
	return get_Second_Nibble (secondByte);
}

byte_t get_Rd (byte_t thirdByte) {
	return thirdByte >> 4;
}

byte_t immediate_operand (byte_t firstByte) {
	return (firstByte & readBinary("00000010")) >> 1;
}

byte_t get_pre_post_indexing_bit(byte_t firstByte) {
	return firstByte & readBinary("00000001");	
}

byte_t get_up_bit(byte_t secondByte) {
	return secondByte & readBinary("10000000");
}

byte_t get_load_store_bit(byte_t secondByte) {
	return secondByte & readBinary("00010000");
}

byte_t get_shifted_register(byte_t thirdByte) {
	return get_Second_Nibble(thirdByte);
}

// short: 2 bytes (16b), we need 12 bits for the offset or register.
unsigned short get_offset(byte_t thirdByte, byte_t fourthByte, 
		byte_t immediate_offset, word_t * registers) {

	if (immediate_offset) {
		// Offset is a shifted register
		// Shift value is the last byte of the registers content
		return registers[get_shifted_register(thirdByte)] & readBinary("11111111");
	} else {
		// Offset is an immediate offset.
		return ((thirdByte & readBinary("1111")) << 8) | fourthByte;
	}
}

// Helper functions for execute_single_data_transfer.
static void execute_pre_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t*memory);

static void execute_post_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t*memory);

static byte_t apply_offset(byte_t Rn, byte_t *firstByte, word_t *registers);

static void change_base_register_by_offset(
		byte_t Rn, 
		byte_t *firstByte, 
		word_t *registers);

static void load(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory);
static void store(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory);
static byte_t add_offset(byte_t Rn, byte_t offset);
static byte_t subtract_offset(byte_t Rn, byte_t offset);

void execute_single_data_transfer (byte_t *firstByte, word_t *registers, byte_t *memory) {
	if (get_pre_post_indexing_bit(firstByte[1])) {

		execute_pre_indexing(firstByte, registers, memory);

	} else {

		execute_post_indexing(firstByte, registers, memory);

	}
}

// Offset added/subtracted to the base register before transferring the data.
static void execute_pre_indexing(
		byte_t *firstByte, 
		word_t *registers, 
		byte_t*memory) 
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
static void execute_post_indexing(byte_t *firstByte, word_t *registers,
 byte_t *memory
) {
	byte_t Rn = get_Rn(firstByte[1]);

	if (get_load_store_bit(firstByte[1])) {
		load(Rn, get_Rd(firstByte[2]), registers, memory);
	} else {
		store(Rn, get_Rd(firstByte[2]), registers, memory);
	}

	change_base_register_by_offset(Rn, firstByte, registers);
}

// Used in case of pre-indexing. Offset is applied and new register address is returned.
// The original register remains unchanged.
static byte_t apply_offset(byte_t Rn, byte_t *firstByte, word_t *registers) {
	// Offset is added when Up bit is set.
	if (get_up_bit(firstByte[1])) {

		Rn = add_offset(
				Rn, 
				get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers) 
		);	

	} else {

		Rn = subtract_offset(
				Rn, 
				get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers)
		);	

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

		registers[Rn] = add_offset(
				Rn, 
				get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers)
		);	

	} else {

		registers[Rn] = subtract_offset(
				Rn, 
				get_offset(
						firstByte[2], 
						firstByte[3], 
						immediate_operand(firstByte[0]), 
						registers)
		);	

	}
}

static void load(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory) {
	registers[Rn] = memory[registers[Rd]];
}

static void store(byte_t Rn, byte_t Rd, word_t *registers, byte_t *memory) {
	memory[registers[Rd]] = registers[Rn];
}

static byte_t add_offset(byte_t Rn, byte_t offset) {
	return Rn + offset;	
}

static byte_t subtract_offset(byte_t Rn, byte_t offset) {
	return Rn - offset;
}

int main(void) {

}


