#include <stdint.h>   /* Declarations of uint_32 and the like */

/*
SDI1/MISO = PIN 12  PORT G7
SDO1/MOSI = PIN 11  PORT G8
SCK1      = PIN 13  PORT G6
SS1       = PIN 33  PORT E7
*/

void rfid_init();
uint8_t rfid_send_data(uint8_t reg, uint8_t value);
void rfid_write_register(uint8_t reg, uint8_t value);
uint8_t rfid_read_register(uint8_t reg);
