#include "servo.h" /* Declarations for servo motor */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include <stdint.h>   /* Declarations of uint_32 and the like */

/*
INPUT VOLTAGE = 5V
OC1           = PIN 3  PORT D0
*/
int servo_time_counter;
uint8_t servo_open;

int servo_init() {

  OC1CON = 0x00000000; // Nollställ oc1
  OC1CON = 0x00000006; // Sätt PWM mode on (bit 0-2), 32-bit Compare Mode bit till 0 (16 bits klocka)

  servo_open = 1;
  servo_rotate_time(SERVO_COUNTER_CLOCKWISE, 120);

  return servo_open;
}

void servo_rotate_time(int direction, int time) {

  if (!servo_timer_active()) {

    servo_set_direction(direction);
    servo_start_rotation();
    servo_start_timer(time);
  }
}

void servo_start_timer(int time) {

  servo_time_counter = time; // Set timer length

  T2CON = 5 << 4;     // 1:64 scaling
  TMR2 = 0;           // Nollställ klockan
  PR2 = 25000;        // Räkna upp till 31 250

  enable_interrupt();
  IECSET(0) = 0x00000100; // Sätt index 8 till en etta (motsvarar timer 2)
  IPCSET(2) = 0x00000004; // Sätt prio till 4

  T2CONSET = 1 << 15; // Starta klockan
}

void servo_stop_timer(){
  T2CONCLR = 1 << 15; // Stoppa klockan
}

void servo_interrupt(){

  servo_time_counter--;
  IFSCLR(0) = 1 << 8;

  if (servo_time_counter == 0) {

    servo_stop_timer();
    servo_stop_rotation();
  }
}

int servo_toggle() {

  if (!servo_timer_active()) {
    if (servo_open) {

      servo_rotate_time(SERVO_CLOCKWISE, 120);
      servo_open = 0;
    } else {

      servo_rotate_time(SERVO_COUNTER_CLOCKWISE, 120);
      servo_open = 1;
    }
  }

  return servo_open;
}

void servo_set_direction(int direction){
  OC1R = direction;
  OC1RS = direction + 3;
}

void servo_start_rotation() {
  OC1CON = (OC1CON & 0xffff7fff) | 1 << 15;
}

void servo_stop_rotation() {
  OC1CON &= 0xffff7fff;
}

int servo_timer_active(){
  return (T2CON >> 15) & 1;
}
