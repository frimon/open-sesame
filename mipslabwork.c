/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include "rfid.h" /* Declarations for RFID */

const CLOCKWISE = 0x3000;
const COUNTER_CLOCKWISE = 0x500;

int servo_clockwise = 1;
int mytime = 0x5957;
int counter = 0;
int timerCounter = 0;
int rfid_clock = 1;
char fifo_buffer[64];

char textstring[] = "text, more text, and even more text!";

/* Char to hexstring */
char* char_to_hexstring (uint8_t data) {

  unsigned char digit1 = data >> 4;
  unsigned char digit2 = data & 0x0f;

  digit1 += digit1 <= 9 ? 0x30 : 0x37; // Convert to Ascii
  digit2 += digit2 <= 9 ? 0x30 : 0x37;

  char out[] = {
    '0', 'x', digit1, digit2, 0
  };

  return out;
}

/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}



/* Lab-specific initialization goes here */
void labinit( void )
{

  // 80000000/256/31250 = 10

  rfid_init();

  T2CON = 5 << 4;     // 1:32 scaling
  TMR2 = 0;           // Nollställ klockan
  PR2 = 25000;        // Räkna upp till X

  OC1CON = 0x00000000; // Nollställ oc1
  OC1CON = 0x00000006; // Sätt PWM mode on (bit 0-2), 32-bit Compare Mode bit till 0 (16 bits klocka)

  OC1R = CLOCKWISE;
  OC1RS = CLOCKWISE + 3;

  T2CONSET = 1 << 15; // Starta klockan

  /*
  AUTO TEST
  rfid_write_register(0x01, 0x0f); // Soft reset
  int i;
  for (i = 0; i < 25; i++) {
    rfid_write_register(0x00, 0x00); // Sending 0x00 25 times to clear buffer
  }

  rfid_write_register(0x36, 0x09); // Enable autotestreg
  rfid_write_register(0x09, 0x00); // Write 0x00 to fifo buffer
  rfid_write_register(0x01, 0x03); // Start self test with CalcCRC command

  rfid_read_fifo(&fifo_buffer);
  */

  return;
}

/* Motor */

int buttons_pushed = 0;
int button2_pushed = 0;
int button4_pushed = 0;


/* This function is called repetitively from the main program */
void labwork( void )
{
  /* Motor */
  int buttons = getbtns();
  int button4 = (buttons >> 2) & 1;

  if (button4 && !button4_pushed) {

    OC1R = CLOCKWISE;
    OC1RS = CLOCKWISE + 3;
    button4_pushed = 1;
  } else if (!button4 && button4_pushed) {
    button4_pushed = 0;
  }

  int button2 = buttons & 1;
  if (button2 && !button2_pushed) {

    OC1R = COUNTER_CLOCKWISE;
    OC1RS = COUNTER_CLOCKWISE + 3;
    button2_pushed = 1;
  } else if (!button2 && button2_pushed) {
    button2_pushed = 0;
  }

  if (buttons && !buttons_pushed) {

    OC1CON = (OC1CON & 0xffff7fff) | 1 << 15;
    //display_string( 3, "on");
    buttons_pushed = 1;
  } else if (!buttons && buttons_pushed) {

    OC1CON &= 0xffff7fff;
    //display_string( 3, "off");
    buttons_pushed = 0;
  }

  /* Timer */

  // Om timeout flaggan är 1, räkna upp timerCounter och nollställ timeoutflaggan.
  if (IFS(0) & 0x100) {

    timerCounter++;
    IFS(0) = 0;
  }

  // Om vår egna räknare är mindre än 10, ignorera koden nedan.
  if (timerCounter < 100) {
    return;
  }

  //rfid_write_register(0x01, 0x00); // CommandReg, PCD_Idle
  //rfid_write_register(0x04, 0x7F); // ComIrqReg, ???
  // PCD_SetRegisterBitMask(FIFOLevelReg, 0x80);
  //rfid_write_register(0x09, 0x26); // FIFODataReg, SendData (PICC_CMD_REQA)
  // PCD_WriteRegister(BitFramingReg, bitFraming);
  //rfid_write_register(0x01, 0x0C); // CommandReg, PCD_Idle

  /*
  uint8_t received = rfid_read_register(0x09);
  display_string(3, char_to_hexstring(received));
  */

  //display_string(0, itoaconv(counter));

  uint8_t waitIRq = 0x77;
  uint8_t irqEn = 0x77;

  rfid_write_register(0x0D, 0x07); // self.Write_MFRC522(self.BitFramingReg, 0x07)

  rfid_write_register(0x02, irqEn | 0x80);   // self.Write_MFRC522(self.CommIEnReg, irqEn|0x80)
  rfid_clear_register_bitmask(0x04, 0x80);  // self.ClearBitMask(self.CommIrqReg, 0x80)
  rfid_set_register_bitmask(0x0A, 0x80);    // self.SetBitMask(self.FIFOLevelReg, 0x80)

  rfid_write_register(0x01, 0x00); // self.Write_MFRC522(self.CommandReg, self.PCD_IDLE);
  rfid_write_register(0x09, 0x26); // self.Write_MFRC522(self.FIFODataReg, sendData[i]) --> PICC_REQIDL
  rfid_write_register(0x01, 0x0C); // self.Write_MFRC522(self.CommandReg, command) --> PCD_TRANSCEIVE
  rfid_set_register_bitmask(0x0D, 0x80); // self.SetBitMask(self.BitFramingReg, 0x80)

  int i = 25;
  uint8_t n;

  do {
    quicksleep(100);
    n = rfid_read_register(0x04);
    i--;
  } while ((i!=0) && !(n&0x01) && !(n&waitIRq));

  display_string(1, itoaconv(i));
  display_string(2, char_to_hexstring(n));

  rfid_clear_register_bitmask(0x0D, 0x80);

  // Starting anticoll
  rfid_write_register(0x0D, 0x00); // self.Write_MFRC522(self.BitFramingReg, 0x00)

  rfid_write_register(0x02, irqEn | 0x80);  // self.Write_MFRC522(self.CommIEnReg, irqEn|0x80)
  rfid_clear_register_bitmask(0x04, 0x80);  // self.ClearBitMask(self.CommIrqReg, 0x80)
  rfid_set_register_bitmask(0x0A, 0x80);    // self.SetBitMask(self.FIFOLevelReg, 0x80)
  rfid_write_register(0x01, 0x00); // self.Write_MFRC522(self.CommandReg, self.PCD_IDLE);

  // self.Write_MFRC522(self.FIFODataReg, sendData[i])
  rfid_write_register(0x09, 0x93); // serNum.append(self.PICC_ANTICOLL)
  rfid_write_register(0x09, 0x20); // serNum.append(0x20)

  rfid_write_register(0x01, 0x0C); // self.Write_MFRC522(self.CommandReg, command) --> PCD_TRANSCEIVE
  rfid_set_register_bitmask(0x0D, 0x80); // self.SetBitMask(self.BitFramingReg, 0x80)

  uint8_t error = rfid_read_register(0x06); // if (self.Read_MFRC522(self.ErrorReg) & 0x1B)==0x00:
  if (!(error & 0x1B)) {
    display_string(3, "MI_OK");

    if (n & irqEn & 0x01) {
      display_string(3, "MI_NOTAGERR");
    } else {
      n = rfid_read_register(0x0A);
      uint8_t lastBits = rfid_read_register(0x0C) & 0x07;

      uint8_t* buffer = rfid_read_fifo();
      display_string(0, char_to_hexstring(buffer[1]));

      //display_string(0, char_to_hexstring(lastBits));
    }
  } else {
    display_string(2, "MI_ERR");
  }




  //rfid_read_fifo(fifo_buffer);

  //display_string(1, char_to_hexstring(counter));
  //display_string(3, char_to_hexstring(fifo_buffer[counter]));
  counter++;

  /* VERSION */
  /*
  uint8_t received = rfid_read_register(0x37);
  display_string(3, char_to_hexstring(received));
  */

  /* Motor (Kanske behövs) */

  /*
  if (counter < 20) {
    PORTESET = 1 << 6;
  } else {
    PORTECLR = 1 << 6;

    if (counter > 24) {
      counter = 0;
    }
  }
  */

  /*
  if (servo_clockwise) {
    OC1R = COUNTER_CLOCKWISE;
    OC1RS = COUNTER_CLOCKWISE + 3;
  } else {
    OC1R = CLOCKWISE;
    OC1RS = CLOCKWISE + 3;
  }

  servo_clockwise = !servo_clockwise;
  */

  //OC1CON &= 0xfff7fff;

  // Om räknaren har räknat upp till 10, nollställ och räkna sen upp klocka etc.
  timerCounter = 0;

  display_update();
  //tick( &mytime );
  //display_image(96, icon);
}
