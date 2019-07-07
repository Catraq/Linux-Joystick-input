#ifndef JOYSTICK_PS3_H_
#define JOYSTICK_PS3_H_

#define JOYSTICK_NAME_LENGTH 128

#define JOYSTICK_PS3_BUTTON_CROSS 		0
#define JOYSTICK_PS3_BUTTON_CIRCLE 		1
#define JOYSTICK_PS3_BUTTON_TRAINGLE 		2
#define JOYSTICK_PS3_BUTTON_SQUARE 		3
#define JOYSTICK_PS3_BUTTON_LEFT_TOP 		4
#define JOYSTICK_PS3_BUTTON_RIGHT_TOP 		5
#define JOYSTICK_PS3_BUTTON_LEFT_BOTTON 	6
#define JOYSTICK_PS3_BUTTON_RIGHT_BOTTOM 	7
#define JOYSTICK_PS3_BUTTON_SELECT 		8 
#define JOYSTICK_PS3_BUTTON_START 		9
#define JOYSTICK_PS3_BUTTON_PSN 		10
#define JOYSTICK_PS3_BUTTON_JOYSTICK_LEFT 	11
#define JOYSTICK_PS3_BUTTON_JOYSTICK_RIGHT 	12
#define JOYSTICK_PS3_BUTTON_UP 			13
#define JOYSTICK_PS3_BUTTON_DOWN 		14
#define JOYSTICK_PS3_BUTTON_LEFT 		15
#define JOYSTICK_PS3_BUTTON_RIGHT 		16
#define JOYSTICK_PS3_BUTTON_LENGTH 		17

#define JOYSTICK_PS3_AXIS_LEFT_X 	0
#define JOYSTICK_PS3_AXIS_LEFT_Y 	1
#define JOYSTICK_PS3_AXIS_LEFT_BOTTON 	2
#define JOYSTICK_PS3_AXIS_RIGHT_X 	3 
#define JOYSTICK_PS3_AXIS_RIGHT_Y 	4 
#define JOYSTICK_PS3_AXIS_RIGHT_BOTTOM 	5
#define JOYSTICK_PS3_AXIS_LENGTH	6

#include <stdint.h>

#include <pthread.h>

#include <linux/joystick.h>


struct joystick_ps3
{
	uint8_t button[JOYSTICK_PS3_BUTTON_LENGTH];
	int32_t axis[JOYSTICK_PS3_AXIS_LENGTH];
};


struct joystick_ps3_context
{
	const char *device_path;

	int device_fd;


	volatile int thread_state;

	pthread_cond_t 	condition;
	pthread_mutex_t mutex;
	pthread_t 	thread;


	struct joystick_ps3 input;
};

int joystick_ps3_intialize(struct joystick_ps3_context *context, const char *device_path);
int joystick_ps3_destroy(struct joystick_ps3_context *context);
int joystick_ps3_input(struct joystick_ps3_context *context, struct joystick_ps3 *input, struct timespec *timeout);

#endif 
