#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdint.h>

#include <pthread.h>

#include <linux/joystick.h>
#include <linux/limits.h>

#define JOYSTICK_NAME_LENGTH 128

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
	uint8_t joystick_axis_count;
	uint8_t joystick_button_count;
};

struct joystick_input_value
{
	int16_t *joystick_axis_value;
	int16_t *joystick_button_value;
}; 


struct joystick_context
{
	int device_fd;

	struct joystick_input_attrib input_attrib;
};

uint32_t joystick_axis_count(struct joystick_context *context);

/* 
 * Used for identifying joystick devices that satisfys some requirements. 
 *
 * @param input_requirement have to be filled with minium requirements. 
 * @param input_attrib will be filled with the results. 
 * @param input_attrib_max maxium elements input_attrib fits. 
 *
 * @return returns 0 if no devices are found. Otherwise number of devices found. 
 *
 */

size_t joystick_identify_by_requirement(struct joystick_input_requirement *input_requirement, struct joystick_input_attrib *input_attrib, size_t input_attrib_max);

/* 
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

int joystick_init(struct joystick_context *context, const char *device_path);

/*
 * Destroy ps3 joystick context. 
 *
 * @param joystick ps3 context to destroy. 
 *
 * @return 0 on success. -1 on failure. 
 */
int joystick_destroy(struct joystick_context *context);

int joystick_input_create(struct joystick_context *context, struct joystick_input_value *value);
void joystick_input_clear(struct joystick_context *context, struct joystick_input_value *value);
int joystick_input_destroy(struct joystick_input_value *value);

/* 
 * Open device 
 */

int joystick_open(const char *joystick_device_path, struct joystick_input_attrib *input_attrib);

/* 
 * Close device 
 */

int joystick_close(int joystick_device_fd);


/* 
 * Read PS3 joystick values. 
 *
 * @param context Initialized context. 
 *
 * @param value Where read data will be stored.  
 *
 * @return Returns 0 on success. -1 on failure. 
 */

int joystick_input(struct joystick_context *context, struct joystick_input_value *value);

#ifdef __cplusplus
}
#endif

#endif
