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

uint8_t cards[][4] = {
  { 0x06, 0xf3, 0x25, 0xa0 }, // Vitt RFID-kort
  { 0x83, 0x9e, 0x18, 0x32 } // Simons SL-kort
};

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

  TRISECLR = 1; // LED på pin 26 till output
  PORTECLR = 1;

  T2CONSET = 1 << 15; // Starta klockan
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

  int status = rfid_validate_card(cards);
  if (status) {
    PORTESET = 1;
    //display_string(0, "OK");
  } else {
    //display_string(0, "NOT OK");
  }

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
