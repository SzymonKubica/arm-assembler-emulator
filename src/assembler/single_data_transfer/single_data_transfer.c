#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "../../defns.h"
#include "../../assembler_defs.h"
#include "single_data_transfer.h"

static word_t get_sdt_instruction_template(void) {
	return 1 << 26; // Bits 27-26 in sdt instruction are always: 01.
}

static bool is_ldr_instruction(char *mnemonic) {
	return strcmp(mnemonic, "ldr") == 0;
}

static void set_Nth_bit(word_t *word, int n) {
	*word |= (1 << n);
}

static void set_cond(word_t *binary_instruction) {
	// ldr/str instructions don't have a cond in the syntax, cond is always al.
	// al = 1110 (14 in decimal).
	*binary_instruction |= (14 << 28); 
}

static void set_Rd(word_t *binary_instruction, int Rd) {
	*binary_instruction |= (Rd << 12);
}

static void set_Rn(word_t *binary_instruction, int Rn) {
	*binary_instruction |= (Rn << 16);
}

static void set_offset(word_t *binary_instruction, int offset) {
	*binary_instruction |= abs(offset);
}

//static void set_I_bit(word_t *binary_instruction) {
	//set_Nth_bit(binary_instruction, 25);
//}

static void set_P_bit(word_t *binary_instruction) {
	set_Nth_bit(binary_instruction, 24);
}

static void set_U_bit(word_t *binary_instruction) {
	set_Nth_bit(binary_instruction, 23);
}

static void set_L_bit(word_t *binary_instruction) {
	set_Nth_bit(binary_instruction, 20);
}

static bool is_pre_indexing_address(char *address) {
	return address[0] == '[' && address[strlen(address) - 1] == ']';
}

static bool is_zero_offset(char *address) {
	// If the offset is zero, the address has a form [Rn] which has length 4.
	// Or in the case when the register if higher than 9 we have e.g. [r11]
	// which has length 5.
	int length = strlen(address);
	return length == 4 || length == 5;
}

static bool is_digit(char ch) {
	return '0' <= ch && ch <= '9';
}

static int parse_base_register_number(char *address) {
	char *ptr = address;
	// Parses throught the address string until finds the first 'r'
	// That 'r' indicates the reference to the base register.
	while (*ptr != 'r') {
		ptr++;
	}
	assert(*ptr == 'r');
	// ptr is adjusted to point to the first digit.
	ptr++;
	if (!is_digit(*(ptr + 1))) {
		char register_digit[1];
		register_digit[0] = *ptr;
		return atoi(register_digit);
	} else {
		char register_digit[2];
		register_digit[0] = *ptr;
		register_digit[1] = *(ptr + 1);
		return atoi(register_digit);
	}
}

static bool is_offset_by_expression(char *address) {
	// If offset is specified by <#expression> char '#' is present in the string.
	char *ptr = address;
	for (; *ptr != '\0'; ptr++) {
		if (*ptr == '#') {
			return true;
		}
	}
	return false;
}

static int parse_offset_by_expression(char *address) {
	assert(is_offset_by_expression(address));
	char *ptr = address;
	while (*ptr != '#') {
		ptr++;
	}
	assert(*ptr == '#');
	// Adjusting ptr so that it points to the first digit or '-' character.
	ptr++;
	char *first_digit = ptr;
	int length_of_offset_decimal_representation = 0;
	while (is_digit(*ptr) || *ptr == '-') {
		length_of_offset_decimal_representation++;
		ptr++;
	}
	char offset_representation[length_of_offset_decimal_representation + 1]; 
	for (int i = 0; i < length_of_offset_decimal_representation; i++) {
		offset_representation[i] = *first_digit;
		first_digit++;
	} 
	offset_representation[length_of_offset_decimal_representation] = '\0';
	return atoi(offset_representation);
}

word_t assemble_single_data_transfer_instruction(
	char *mnemonic, 
	int Rd,
	char *address) 
{

	word_t result = get_sdt_instruction_template();
	set_cond(&result);
	set_Rd(&result, Rd);

	int Rn = parse_base_register_number(address);
	set_Rn(&result, Rn);

	if (is_ldr_instruction(mnemonic)) {
		set_L_bit(&result);
		if (address[0] == '=') {
			// A numeric constant of the form: <=expression> (ldr only).
			// TODO: implement this case. (See page 15, bulletpoint 1)
		}
	}

	if (is_pre_indexing_address(address)) {
		set_P_bit(&result);
	}

	if (is_zero_offset(address)) {
		// In the case of zero offset, the Up bit is set as we 'add' zero.
		set_U_bit(&result);
	} else if (is_offset_by_expression(address)) {

		// Parsing the offset by expression starting with '#'.
		int offset = parse_offset_by_expression(address);	
		if (offset > 0) {
			set_U_bit(&result);
		}
		// Offset is an immediate value, I bit not set.
		set_offset(&result, offset);
	} else {
		// TODO (optional): pre-indexing case: [Rn, {+/-} Rm{, <shift>}].
		// TODO (optional): post-indexint case: [Rn],{+/-} Rm{, <shift>}.
	}
	return result;
}
	
