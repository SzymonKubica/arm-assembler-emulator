#ifndef COMMON_H
#define COMMON_H

byte_t get_First_Nibble (byte_t byte);

byte_t get_Second_Nibble (byte_t byte); 

byte_t get_Rn (byte_t secondByte);

byte_t get_Rd (byte_t thirdByte);

byte_t get_Set_Condition_Code (byte_t thirdByte);

#endif