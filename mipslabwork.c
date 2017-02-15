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

/*
SDI1/MISO = PIN 12  PORT G7
SDO1/MOSI = PIN 11  PORT G8
SCK1      = PIN 13  PORT G6
SS1       = PIN 33  PORT E7
*/

char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{
  return;
}

/*
uint8_t spi_send_receive(uint8_t send) {

  PORTESET = 1;
  SPI1BUF = send;
  while ((SPI1STAT >> 11) & 1);

  PORTECLR = 1;

  return SPI1BUF;
}
*/

/*
uint8_t read_register(uint8_t reg) {

  // select slave
  spi_send_receive(0x80 | (reg & 0x7e));
  uint8_t response = spi_send_receive(0);

  // unselect slave
  return response;
}
*/

/*
uint8_t spi_send_receive(uint8_t data) {

  while(!(SPI1STAT & 0x08));
  SPI1BUF = data;
  while(!(SPI1STAT & 1));
  return SPI1BUF;
}
*/

/* Lab-specific initialization goes here */
void labinit( void )
{

  // 80000000/256/31250

  //TRISD |= (0x7f << 5); // 5 through 11 to ones

  TRISDCLR = 1 << 4; // Sätter SS1 som output
  TRISGCLR = 5 << 6; // Sätter SCK1 och MOSI som outputs
  TRISGSET = 1 << 7; // Sätter MISO som input
  TRISECLR = 0xff;

  //PORTDSET = 1 << 4; // Sätter slave select till 1
  PORTESET = 1 << 7;

  T2CON = 5 << 4;     // 1:32 scaling
  TMR2 = 0;           // Nollställ klockan
  PR2 = 25000;        // Räkna upp till X

  OC1CON = 0x00000000; // Nollställ oc1
  OC1CON = 0x00000006; // Sätt PWM mode on (bit 0-2), 32-bit Compare Mode bit till 0 (16 bits klocka)

  OC1R = CLOCKWISE;
  OC1RS = CLOCKWISE + 3;

  //char junk;
  //SPI2CON = 0;
  //SPI1CONCLR = 1 << 15; // SPI Peripheral On bit
  //junk = SPI2BUF;
  //SPI1BRG = 7;

  // Test fredriks kod
  /*
  SPI2BRG = 4;
  SPI2STATCLR = 0x40;
  */
  /*
  SPI1CONSET = 0x40;
  SPI1CONSET = 0x20;
  SPI1CONSET = 0x8000;
  */
  /*
  SPI2CONSET = 1 << 5; // Master Mode Slave Select Enable bit
  SPI2CONSET = 1 << 8;  // SPI Clock Edge Select bit
  SPI2CONSET = 1 << 15; // SPI Peripheral On bit
  */

  T2CONSET = 1 << 15; // Starta klockan

  //TRISE = (TRISE & 0xffffff0f) | (1 << 6);
  //TRISECLR = 1 << 7;
  //PORTESET = 1 << 7;

  /*
  for (int i = 0; i < 25; i++) {
    spi_send_receive(0x00);
  }
  */

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

  // Om timeout flaggan är 1, räkna upp timerCounter och nollställ timeoutflaggan.
  if (IFS(0) & 0x100) {

    timerCounter++;
    IFS(0) = 0;
  }

  // Om vår egna räknare är mindre än 10, ignorera koden nedan.
  if (timerCounter < 100) {
    return;
  }

  display_string(0, itoaconv(counter));
  counter++;

  unsigned char received;
  /*

  spi_send_receive(0xEE);
  received = spi_send_receive(0x00);
  */

  PORTFCLR = 0x10; // DISPLAY_CHANGE_TO_COMMAND_MODE

  //PORTDCLR = 1 << 4; // Sätter slave select till 0
  PORTECLR = 1 << 7;
  spi_send_recv(0xEE);
  received = spi_send_recv(0);
  PORTESET = 1 << 7;
  //PORTDSET = 1 << 4; // Sätter slave select till 1

  PORTFSET = 0x10; // DISPLAY_CHANGE_TO_DATA_MODE

  //PORTESET = received;

  unsigned char digit1 = received >> 4;
  unsigned char digit2 = received & 0x0f;

  digit1 += digit1 <= 9 ? 0x30 : 0x37;
  digit2 += digit2 <= 9 ? 0x30 : 0x37;

  char out[] = {
    '0',
    'x',
    digit1,
    digit2,
    0x00
  };

  /*
  if ((PORTF >> 2) & 1) {
    display_string(3, "1");
  } else {
    display_string(3, "0");
  }
  */

  display_string(3, out);

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