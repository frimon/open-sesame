#include "rfid.h" /* Declarations for RFID */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdint.h>   /* Declarations of uint_32 and the like */

void rfid_init() {
  TRISGCLR = 5 << 6; // Sätter SCK1 och MOSI som outputs
  TRISGSET = 1 << 7; // Sätter MISO som input
  TRISECLR = 1 << 7; // Sätter Slaveselect-porten som output
  PORTESET = 1 << 7; // Slaveselect (inte används)

  /* TIMEOUT */
  rfid_write_register(0x2A,0x80); // TMODEREG
  rfid_write_register(0x2B,0xA9); // TPRESCALERREG
  rfid_write_register(0x2C,0x03); // TRELOADREGH
  rfid_write_register(0x2D,0xE8); // TRELOADREGL
  rfid_write_register(0x15,0x40); // TXASKREG
  rfid_write_register(0x11,0x3D); // MODEREG

  /* ANTENNA */
  uint8_t ant_value = rfid_read_register(0x14); // TXCONTROLREG
  if ((ant_value & 0x03) != 0x03) {
    rfid_write_register(0x14, ant_value | 0x03);
  }

  /* CARD PRESENT CHECK */

}
uint8_t rfid_send_data(uint8_t reg, uint8_t value){
  PORTFSET = 0x10; // DISPLAY_CHANGE_TO_DATA_MODE
  PORTECLR = 1 << 7; // Sätter till att RFID-kortet ska agera som slave
  spi_send_recv(reg);
  uint8_t received = spi_send_recv(value);
  PORTESET = 1 << 7; // Sätter att RFID-kortet inte längre är slave
  PORTFCLR = 0x10; // DISPLAY_CHANGE_TO_COMMAND_MODE
  return received;
}
void rfid_write_register(uint8_t reg, uint8_t value) {
  rfid_send_data((reg << 1) & 0x7E, value);
}
uint8_t rfid_read_register(uint8_t reg) {
  return rfid_send_data(0x80 | ((reg << 1) & 0x7E), 0);
}
