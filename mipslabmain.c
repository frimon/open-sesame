/* mipslabmain.c

   This file written 2015 by Axel Isaksson,
   modified 2015 by F Lundevall

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include "rfid.h"
#include "servo.h"

uint8_t cards[2][5] = {
  { 0x06, 0xf3, 0x25, 0xa0, 0x70 }, // Vitt RFID-kort
  { 0x83, 0x9e, 0x18, 0x32, 0x37 }  // Simons SL-kort
};

void init(void) {

  rfid_init();
  PORTFSET = servo_init();
  return;
}

void loop(void) {

  int no_of_cards = sizeof(cards) / sizeof(cards[0]);
  int status = rfid_validate_card(cards, no_of_cards);
  if (status) {
    PORTF = PORTF & ~1 | servo_toggle();
  }
}

int main(void) {

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
	SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;

	init(); /* Do any lab-specific initialization */

	while(1) {
	  loop(); /* Do lab-specific things again and again */
	}

	return 0;
}
