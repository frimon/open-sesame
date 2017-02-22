#define SERVO_CLOCKWISE 0x3000
#define SERVO_COUNTER_CLOCKWISE 0x500

int servo_init();
void servo_start_rotation();
void servo_stop_rotation();
void servo_rotate_time(int direction, int time);
void servo_start_timer(int time);
void servo_set_direction(int direction);
void servo_stop_timer();
void servo_interrupt();
int servo_toggle();
void servo_interrupt();
int servo_timer_active();
