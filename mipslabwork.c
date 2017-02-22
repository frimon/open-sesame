/*
   This file written 2017 by Open Sesame INC
*/

#include "servo.h"

/* Interrupt Service Routine */
void user_isr(void) {
  servo_interrupt();
}