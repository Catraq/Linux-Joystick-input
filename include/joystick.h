/*
 * Decription:
 * 	Used for finding joystick devices satisfying certain requirements.
 * 	Open and read the joystick device. 
 * Notes:
 *	- Uninitialized means that the structure/data is zeroed out using etc. memset. 
 * 	- Any return value less than 0 indicates that there have been a error and the 
 *	  file descriptor have been closed. Use reopen for same device or open for any
 *	  new device. 
 * Error:
 * 	Assert on logical error. 
 */

#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>

#include <pthread.h>

#include <linux/joystick.h>
#include <linux/limits.h>

#define JOYSTICK_NAME_LENGTH 128

#define JOYSTICK_AXIS_MAX 	32
#define JOYSTICK_BUTTON_MAX 	32

#ifdef JOYSTICK_LOG_FD
extern FILE *joystick_log_fd;
#endif 


struct joystick_input_attrib
{
	uint8_t joystick_device_path[PATH_MAX];
	uint8_t joystick_name[JOYSTICK_NAME_LENGTH];
	uint8_t joystick_axis_count;
	uint8_t joystick_button_count;
};

struct joystick_input_requirement 
{
	uint8_t requirement_axis_count_min;
	uint8_t requirement_button_count_min;

	uint8_t requirement_axis_count_max;
	uint8_t requirement_button_count_max;
};

struct joystick_input_value
{
	float 	joystick_axis_value[JOYSTICK_AXIS_MAX];
	int16_t joystick_button_value[JOYSTICK_BUTTON_MAX];
}; 


struct joystick_device
{
	int device_fd;

	struct joystick_input_attrib input_attrib;
	struct joystick_input_value input_value;
};


void joystick_input_attrib_print(struct joystick_input_attrib *input_attrib, FILE *out);


/*
 * Get number of axises on a controller 
 * 
 * @param joystick_device that is initialized. 
 *
 * @return number of axises. 
 */
uint32_t joystick_device_axis_count(struct joystick_device *device);

/* 
 * Poll new values into value. 
 *
 * @param device Initialized device. 
 *
 * @param value Where read data will be stored.  
 *
 * @return Returns 1 if there are new values, 0 on nothing, but success. -1 on failure. 
 */

int joystick_device_poll(struct joystick_device *device);




/* 
 * Used for identifying joystick devices that satisfys some requirements. 
 *
 * @param input_requirement have to be filled with minium requirements. 
 *
 * @param input_attrib will be filled with the results. 
 *
 * @param input_attrib_max maxium elements input_attrib fits. 
 *
 * @return returns 0 if no devices are found. Otherwise number of devices found. 
 *
 */

size_t joystick_device_identify_by_requirement(struct joystick_input_requirement *input_requirement, struct joystick_input_attrib *input_attrib, size_t input_attrib_max);

/* 
 * Initialize joystick device.  
 * 
 * @param device joystick device that should be initialized. 
 *
 * @param device_path should be system path to ps3 controller. Usually /dev/input
 * 
 * @param input_requirement should be initialized with the applciation requirements. The min attributes can be 0 and max should reflect the limitations in inputs buffers. 
 * 
 * @return Returns 0 on success. -1 on failure.
 */

int joystick_device_open(struct joystick_device *device, struct joystick_input_requirement *input_requirement, const char *device_path);

/* 
 * Reopen joystick device. 
 * 
 * @param device joystick device that should be initialized. 
 *
 * @return Returns 0 on success. -1 on failure.
 */


int joystick_device_reopen(struct joystick_device *device);

/*
 * Destroy ps3 joystick device. 
 *
 * @param joystick ps3 device to destroy. 
 *
 * @return 0 on success. -1 on failure. 
 */

int joystick_device_close(struct joystick_device *device);

#ifdef __cplusplus
}
#endif

#endif
