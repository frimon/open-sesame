#include "servo.h" /* Declarations for servo motor */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdint.h>   /* Declarations of uint_32 and the like */

/*
INPUT VOLTAGE = 3V
OC1           = PIN 3  PORT D0
*/

const CLOCKWISE = 0x3000;
const COUNTER_CLOCKWISE = 0x500;

void servo_init() {
  OC1CON = 0x00000000; // Nollställ oc1
  OC1CON = 0x00000006; // Sätt PWM mode on (bit 0-2), 32-bit Compare Mode bit till 0 (16 bits klocka)
  OC1R = CLOCKWISE; // Sätt pulsstart
  OC1RS = CLOCKWISE + 3; // Sätt pulsslut
}

void servo_set_clockwise(){
  OC1R = CLOCKWISE;
  OC1RS = CLOCKWISE + 3;
}

void servo_set_counter_clockwise(){
  OC1R = COUNTER_CLOCKWISE;
  OC1RS = COUNTER_CLOCKWISE + 3;
}

void servo_start_rotation() {
  OC1CON = (OC1CON & 0xffff7fff) | 1 << 15;
}

void servo_stop_rotation() {
  OC1CON &= 0xffff7fff;
}
