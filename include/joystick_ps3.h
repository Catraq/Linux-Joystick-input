#ifndef JOYSTICK_PS3_H_
#define JOYSTICK_PS3_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#include <pthread.h>

#include <linux/joystick.h>

#define JOYSTICK_NAME_LENGTH 128

enum joystick_ps3_button 
{
	JOYSTICK_PS3_BUTTON_CROSS = 0,
	JOYSTICK_PS3_BUTTON_CIRCLE = 1,
	JOYSTICK_PS3_BUTTON_TRAINGLE = 2,
	JOYSTICK_PS3_BUTTON_SQUARE = 3,
	JOYSTICK_PS3_BUTTON_LEFT_TOP = 4,
	JOYSTICK_PS3_BUTTON_RIGHT_TOP = 5,
	JOYSTICK_PS3_BUTTON_LEFT_BOTTON = 6,
	JOYSTICK_PS3_BUTTON_RIGHT_BOTTOM = 7,
	JOYSTICK_PS3_BUTTON_SELECT = 8,
	JOYSTICK_PS3_BUTTON_START = 9,
	JOYSTICK_PS3_BUTTON_PSN = 10,
	JOYSTICK_PS3_BUTTON_JOYSTICK_LEFT = 11,
	JOYSTICK_PS3_BUTTON_JOYSTICK_RIGHT = 12,
	JOYSTICK_PS3_BUTTON_UP = 13,
	JOYSTICK_PS3_BUTTON_DOWN = 14,
	JOYSTICK_PS3_BUTTON_LEFT  = 15,
	JOYSTICK_PS3_BUTTON_RIGHT = 16,
	JOYSTICK_PS3_BUTTON_LENGTH = 17,
};

enum joystick_ps3_axis
{
	JOYSTICK_PS3_AXIS_LEFT_X = 0,
	JOYSTICK_PS3_AXIS_LEFT_Y = 1,
	JOYSTICK_PS3_AXIS_LEFT_BOTTON = 2,
	JOYSTICK_PS3_AXIS_RIGHT_X = 4,
	JOYSTICK_PS3_AXIS_RIGHT_Y = 3,
	JOYSTICK_PS3_AXIS_RIGHT_BOTTOM = 5,
	JOYSTICK_PS3_AXIS_LENGTH = 6,
};

struct joystick_ps3
{
	enum joystick_ps3_button button[JOYSTICK_PS3_BUTTON_LENGTH];
	enum joystick_ps3_axis 	axis[JOYSTICK_PS3_AXIS_LENGTH];
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

/** 
 * Initialize ps3 joystick context.  
 * 
 * @param context joystick ps3 context that should be initialized. 
 *
 * @param device_path should be system path to ps3 controller. Usually /dev/input/js0
 * 
 * @param device_must_exist 1 if device have to be present when initializing. Will fail 
 * if device is not present and device_must_exist is 1. 
 *
 * @return Returns 0 on success, -1 on failure.  
 */

int joystick_ps3_initialize(struct joystick_ps3_context *context, const char *device_path, int device_must_exist);


/*
 * Destroy ps3 joystick context. 
 *
 * @param joystick ps3 context to destroy. 
 *
 * @return 0 on success. -1 on failure. 
 */
int joystick_ps3_destroy(struct joystick_ps3_context *context);

/* 
 * Read PS3 joystick values. 
 *
 * @param context Initialized context. 
 *
 * @param input Where read data will be stored.  
 *
 * @param timeout_usec Timeout in microseconds the calling thread should wait for most recent data. If timeout 
 * is 0 then it will take the data without waiting. Could be old.  
 *
 * @return Returns 1 on success. 0 on timeout. -1 on failure. 
 */

int joystick_ps3_input(struct joystick_ps3_context *context, struct joystick_ps3 *input, uint32_t timeout_usec);

#ifdef __cplusplus
}
#endif

#endif
