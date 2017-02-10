/* mipslabwork.c

   This file written 2015 by F Lundevall

   This file should be changed by YOU! So add something here:

   This file modified 2015-12-24 by Ture Teknolog

   Latest update 2015-08-28 by F Lundevall

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

const CLOCKWISE = 0x3000;
const COUNTER_CLOCKWISE = 0x500;

int servo_clockwise = 1;
int mytime = 0x5957;
int counter = 0;
int timerCounter = 0;
int rfid_clock = 1;

// 30, 31, 32, 33
// 30 = PORTE[4] = SCLK (klocka - utgång)
// 31 = PORTE[5] = MOSI (master out - utgång)
// 32 = PORTE[6] = MISO (master in - ingång)
// 33 = PORTE[7] = SS (aktiv låg - utgång)

//volatile int *rfid_address = (volatile int*) 0x28;

char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}

/* Lab-specific initialization goes here */
void labinit( void )
{

  // 80000000/256/31250

  TRISD |= (0x7f << 5); // 5 through 11 to ones

  T2CON = 5 << 4;     // 1:32 scaling
  TMR2 = 0;           // Nollställ klockan
  PR2 = 25000;        // Räkna upp till X

  OC1CON = 0x00000000; // Nollställ oc1
  OC1CON = 0x00000006; // Sätt PWM mode on (bit 0-2), 32-bit Compare Mode bit till 0 (16 bits klocka)

  OC1R = CLOCKWISE;
  OC1RS = CLOCKWISE + 3;

  T2CONSET = 1 << 15; // Starta klockan

  TRISE = (TRISE & 0xffffff0f) | (1 << 6);
  PORTECLR = 1 << 7;

  return;
}

int buttons_pushed = 0;
int button2_pushed = 0;
int button4_pushed = 0;


/* This function is called repetitively from the main program */
void labwork( void )
{

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

/*
  if (*rfid_address == 0) {
    display_string(3, "equals zero");
  } else {
    display_string(3, "not zero");
  }
  */


  /*
  if (button4 == 1) {
    OC1CON = (OC1CON & 0xffff7fff) | 1 << 15;
    display_string( 3, "on");
  } else {
    OC1CON &= 0xffff7fff;
    display_string( 3, "off");
  }
  */

/*
    int button4 = buttons & (1 << 2);

    // Är knapp 4 intryckt?
    if (button4) {
      OC1CON = (OC1CON & 0xffff7fff) | 1 << 15;

      display_string( 3, "on");
    } else {
      OC1CON &= 0xffff7fff;
      display_string( 3, "off");
    }
  }
  */

  // Om timeout flaggan är 1, räkna upp timerCounter och nollställ timeoutflaggan.
  if (IFS(0) & 0x100) {

    timerCounter++;
    IFS(0) = 0;
  }

  int rfid_in = (PORTE >> 6) & 1;
  if (rfid_in) {
    display_string( 3, "1");
  } else {
    display_string( 3, "0");
  }

  // Om vår egna räknare är mindre än 10, ignorera koden nedan.
  if (timerCounter < 200) {
    return;
  }

  rfid_clock = !rfid_clock;

  if (rfid_clock) {

    // if raising edge
    PORTESET = 1 << 4;

  } else {

    // if falling edge
    PORTECLR = 1 << 4;

    switch (counter) {

      // 3
      case 0: display_string( 2, "counter 0"); PORTECLR = 1 << 5; break;
      case 1: display_string( 2, "counter 1"); PORTECLR = 1 << 5; break;
      case 2: display_string( 2, "counter 2"); PORTESET = 1 << 5; break;
      case 3: display_string( 2, "counter 3"); PORTESET = 1 << 5; break;

      // 7
      case 4: display_string( 2, "counter 4"); PORTECLR = 1 << 5; break;
      case 5: display_string( 2, "counter 5"); PORTESET = 1 << 5; break;
      case 6: display_string( 2, "counter 6"); PORTESET = 1 << 5; break;
      case 7: display_string( 2, "counter 7"); PORTESET = 1 << 5; counter = -1; break;
    }

    counter++;

    //display_string( 3, "falling edge");
  }

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

  //PORTD = 0xffffffff;

  //delay( 1000 );
  //time2string( textstring, mytime );
  //display_string( 3, textstring );

  display_update();
  tick( &mytime );
  display_image(96, icon);
}