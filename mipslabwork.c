/*
   This file written 2017 by Open Sesame INC
*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include "rfid.h" /* Declarations for RFID */
#include "servo.h" /* Declarations for Servo */

uint8_t cards[2][5] = {
  { 0x06, 0xf3, 0x25, 0xa0, 0x70 }, // Vitt RFID-kort
  { 0x83, 0x9e, 0x18, 0x32, 0x37 }  // Simons SL-kort
};

const CLOCKWISE = 0x3000;
const COUNTER_CLOCKWISE = 0x500;

int counter = 0;
int timerCounter = 0;

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
  /* Check if flag is set (every ms)*/
  servo_interrupt();
}

/* Lab-specific initialization goes here */
void labinit( void ) {

  rfid_init();
  servo_init();

  //TRISECLR = 1; // LED på pin 26 till output
  //PORTECLR = 1;

  return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{

  int no_of_cards = sizeof(cards) / sizeof(cards[0]);
  int status = rfid_validate_card(cards, no_of_cards);
  if (status) {
    servo_toggle();
  }

  display_update();
}
