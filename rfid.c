#include "rfid.h" /* Declarations for RFID */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdint.h>   /* Declarations of uint_32 and the like */

void rfid_init() {

  TRISGCLR = 5 << 6; // Sets SCK1 and MOSI as outputs
  TRISGSET = 1 << 7; // Sets MISO as input
  TRISECLR = 1 << 7; // Sets Slaveselect port as output
  PORTESET = 1 << 7; // Slaveselect (not used)

  rfid_write_register(0x01, 0x0f); // Reset

  rfid_write_register(0x2A,0x8D);  // TModeReg
  rfid_write_register(0x2B,0x3E);  // TPrescalerReg
  rfid_write_register(0x2D, 30);   // TReloadRegL
  rfid_write_register(0x2C, 0);    // TReloadRegH
  rfid_write_register(0x15, 0x40); // TxAutoReg
  rfid_write_register(0x11, 0x3D); // ModeReg

  /* ANTENNA ON */
  uint8_t ant_value = rfid_read_register(0x14); // TxControlReg
  if (~(ant_value & 0x03)) {
    rfid_set_register_bitmask(0x14, 0x03);
  }
}

uint8_t rfid_send_data(uint8_t reg, uint8_t value) {

  PORTFSET = 0x10; // DISPLAY_CHANGE_TO_DATA_MODE
  PORTECLR = 1 << 7; // Sets so that RFID-reader should act as slave.

  spi_send_recv(reg);
  uint8_t received = spi_send_recv(value);

  PORTESET = 1 << 7; // Sets so that RFID-reader is no longer slave.
  PORTFCLR = 0x10; // DISPLAY_CHANGE_TO_COMMAND_MODE

  return received;
}

void rfid_write_register(uint8_t reg, uint8_t value) {
  rfid_send_data((reg << 1) & 0x7E, value); // First bit set to 0 for write
}

uint8_t rfid_read_register(uint8_t reg) {
  return rfid_send_data(0x80 | ((reg << 1) & 0x7E), 0); // First bit set to 1 for write
}

uint8_t* rfid_read_fifo(char fifo_buffer[]) {

  PORTFSET = 0x10; // DISPLAY_CHANGE_TO_DATA_MODE
  PORTECLR = 1 << 7; // // Sets so that RFID-reader should act as slave.

  uint8_t count = 64;

  int i;
  spi_send_recv(0x80 | ((0x09 << 1) & 0x7E));

  for (i = 0; i < count; i++) {

    quicksleep(1000);

    if (i == count - 1) {
      fifo_buffer[i] = spi_send_recv(0);
    } else {
      fifo_buffer[i] = spi_send_recv(0x80 | ((0x09 << 1) & 0x7E));
    }
  }

  PORTESET = 1 << 7; // Sets so that RFID-reader is no longer slave.
  PORTFCLR = 0x10; // DISPLAY_CHANGE_TO_COMMAND_MODE
}

void rfid_set_register_bitmask(uint8_t reg, uint8_t mask) {

  uint8_t tmp = rfid_read_register(reg);
  rfid_write_register(reg, tmp | mask);
}

void rfid_clear_register_bitmask(uint8_t reg, uint8_t mask) {

  uint8_t tmp = rfid_read_register(reg);
  rfid_write_register(reg, tmp & (~mask));
}

int rfid_command_tag(uint8_t cmd, uint8_t data[], int dlen, uint8_t *result, int *rlen) {

  int status = MI_ERR;
  uint8_t irqEn = 0x00;
  uint8_t waitIRq = 0x00;
  uint8_t lastBits, n;
  int i;

  switch (cmd) {
    case MFRC522_AUTHENT:
      irqEn = 0x12; //IdleIEn, ErrIEn
      waitIRq = 0x10;
      break;
    case MFRC522_TRANSCEIVE:
      irqEn = 0x77; // TxIEn, RxIEn, IdleIEn, LoAlertIEn, ErrIEn, TimerIEn
      waitIRq = 0x30;
      break;
    default:
      break;
  }

  rfid_write_register(CommIEnReg, irqEn|0x80);
  rfid_clear_register_bitmask(CommIrqReg, 0x80);
  rfid_set_register_bitmask(FIFOLevelReg, 0x80);

  rfid_write_register(CommandReg, MFRC522_IDLE);  // No action, cancel the current command.

  // Write to FIFO
  for (i=0; i < dlen; i++) {
    rfid_write_register(FIFODataReg, data[i]);
  }

  // Execute the command.
  rfid_write_register(CommandReg, cmd);
  if (cmd == MFRC522_TRANSCEIVE) {
    rfid_set_register_bitmask(BitFramingReg, 0x80);  // StartSend=1, transmission of data starts
  }

  // Waiting for the command to complete so we can receive data.
  i = 100; // Max wait time is 25ms.
  do {
    quicksleep(100);
    // CommIRqReg[7..0]
    // Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
    n = rfid_read_register(CommIrqReg);
    i--;
  } while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  rfid_clear_register_bitmask(BitFramingReg, 0x80);  // StartSend=0

  if (i != 0) { // Request did not time out.
    if(!(rfid_read_register(ErrorReg) & 0x1D)) {  // BufferOvfl Collerr CRCErr ProtocolErr
      status = MI_OK;
      if (n & irqEn & 0x01) {
        status = MI_NOTAGERR;
      }

      if (cmd == MFRC522_TRANSCEIVE) {
        n = rfid_read_register(FIFOLevelReg);
        lastBits = rfid_read_register(ControlReg) & 0x07;
        if (lastBits) {
          *rlen = (n-1)*8 + lastBits;
        } else {
          *rlen = n*8;
        }

        if (n == 0) {
          n = 1;
        }

        if (n > MAX_LEN) {
          n = MAX_LEN;
        }

        // Reading the recieved data from FIFO.
        for (i=0; i<n; i++) {
          result[i] = rfid_read_register(FIFODataReg);
        }
      }
    } else {
      status = MI_ERR;
    }
  }

  return status;
}

int rfid_request_tag(uint8_t mode, uint8_t* data) {

  int status, len;
  rfid_write_register(BitFramingReg, 0x07);  // TxLastBists = BitFramingReg[2..0]

  data[0] = mode;
  status = rfid_command_tag(MFRC522_TRANSCEIVE, data, 1, data, &len);

  if ((status != MI_OK) || (len != 0x10)) {
    status = MI_ERR;
  }

  return status;
}

int rfid_anti_collision(uint8_t* serial) {

  int status, i, len;
  uint8_t check = 0x00;

  rfid_write_register(BitFramingReg, 0x00);  // TxLastBits = BitFramingReg[2..0]

  serial[0] = MF1_ANTICOLL;
  serial[1] = 0x20;
  status = rfid_command_tag(MFRC522_TRANSCEIVE, serial, 2, serial, &len);
  len = len / 8; // len is in bits, and we want each byte.
  if (status == MI_OK) {
    // The checksum of the tag is the ^ of all the values.
    for (i = 0; i < len-1; i++) {
      check ^= serial[i];
    }
    // The checksum should be the same as the one provided from the
    // tag (serial[4]).
    if (check != serial[i]) {
      status = MI_ERR;
    }
  }

  return status;
}

int rfid_validate_card(uint8_t cards[][5], int no_of_cards) {

  uint8_t data[MAX_LEN];
  uint8_t serial[5];

  int status = rfid_request_tag(MF1_REQIDL, data);
  if (status != MI_OK) {
    return 0;
  }

  status = rfid_anti_collision(serial);

  int i;
  for (i = 0; i < no_of_cards; i++) { // Match serial against cards that are set as keys

    if (
    serial[0] == cards[i][0] &&
    serial[1] == cards[i][1] &&
    serial[2] == cards[i][2] &&
    serial[3] == cards[i][3] &&
    serial[4] == cards[i][4]) {

      return 1;
    }
  }

  return 0;
}
