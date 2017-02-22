#define SERVO_CLOCKWISE 0x3000
#define SERVO_COUNTER_CLOCKWISE 0x500

void servo_init();
void servo_start_rotation();
void servo_stop_rotation();
void servo_rotate_time(int direction, int time_in_ms);
void servo_start_timer(int time_in_ms);
void servo_set_direction(int direction);
void servo_stop_timer();
void servo_interrupt();
void servo_toggle();
void servo_interrupt();
