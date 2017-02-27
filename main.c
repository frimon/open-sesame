/*
 * Created by Fredrik Eklööf & Simon Östergren
 * Project Open Sesame - Bye bye keys
 */

#include <stdint.h>
#include <pic32mx.h>
#include "main.h"
#include "rfid.h"
#include "servo.h"

uint8_t cards[2][5] = {
  { 0x06, 0xf3, 0x25, 0xa0, 0x70 }, // White RFID card
  { 0x83, 0x9e, 0x18, 0x32, 0x37 }  // Simons SL card
};

int main(void) {

  // Set up SPI as master
  SPI2CON = 0;
  SPI2BRG = 4;
  // SPI2STAT bit SPIROV = 0;
  SPI2STATCLR = 0x40;
  // SPI2CON bit CKP = 1;
  SPI2CONSET = 0x40;
  // SPI2CON bit MSTEN = 1;
  SPI2CONSET = 0x20;
  // SPI2CON bit ON = 1;
  SPI2CONSET = 0x8000;

	init();

	while(1) {
	  loop();
	}

	return 0;
}

void init() {

  rfid_init();
  PORTFSET = servo_init();
}

void loop() {

  int no_of_cards = sizeof(cards) / sizeof(cards[0]);
  int status = rfid_validate_card(cards, no_of_cards);
  if (status) {
    PORTF = PORTF & ~1 | servo_toggle();
  }
}

void user_isr(void) {
  servo_interrupt();
}

void quicksleep(int cyc) {

  int i;
  for(i = cyc; i > 0; i--);
}

uint8_t spi_send_recv(uint8_t data) {

  while(!(SPI2STAT & 0x08));
  SPI2BUF = data;
  while(!(SPI2STAT & 1));
  return SPI2BUF;
}