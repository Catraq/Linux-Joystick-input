/*TODO:
 *	- Different errors depending on error. Etc disconnected or out of memory. 
 */


/*
 * Decription:
 * 	Used for finding joystick devices satisfying certain requirements.
 * 	Open and read the joystick device. 
 * Notes
 *	- Uninitialized means that the structure/data is zeroed out using etc. memset. 
 * Error 
 * 	If malloc fails. 
 * 	Assert on logical error. 
 */

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


void joystick_input_attrib_print(struct joystick_input_attrib *input_attrib, FILE *out);

uint32_t joystick_axis_count(struct joystick_context *context);

/* 
 * Read joystick values. 
 *
 * @param context Initialized context. 
 *
 * @param value Where read data will be stored.  
 *
 * @return Returns 0 on success. -1 on failure. 
 */

int joystick_input(struct joystick_context *context, struct joystick_input_value *value);


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

size_t joystick_identify_by_requirement(struct joystick_input_requirement *input_requirement, struct joystick_input_attrib *input_attrib, size_t input_attrib_max);

/* 
 * Initialize joystick context.  
 * 
 * @param context joystick context that should be initialized. 
 *
 * @param device_path should be system path to ps3 controller. Usually /dev/input/*
 * 
 * @return Returns 0 on success. -1 on failure.
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



/*  
 *  The structure joystick_input_value is used for reading the data 
 *  from the joystick and then mapping it to the application input 
 *  using the functions in joystick_map.h.
 */


/*
 * Create joystick input buffer. 
 * Allocates memory for buttons and axises. 
 *
 * @param context Joystick context that is initialized. 
 *
 * @param value Pointer to uninitialized structure. The structure data have to be all 0. 
 *
 * @return 0 on success. -1 on failure. 
 */

int joystick_input_create(struct joystick_context *context, struct joystick_input_value *value);

/*
 * Create joystick input buffer. 
 * Deallocates memory for buttons and axises. 
 *
 * @param value Pointer to structure that have been initialized. 
 *
 * @return 0 on success. -1 on failure. 
 */
int joystick_input_destroy(struct joystick_input_value *value);

/*
 * Clears the memory if the input buffer. 
 *
 * @param context Joystick context that is initialized. 
 *
 * @param value Pointer to a initialized structure.
 *
 * @return 0 on success. -1 on failure. 
 */

void joystick_input_clear(struct joystick_context *context, struct joystick_input_value *value);

#ifdef __cplusplus
}
#endif

#endif
