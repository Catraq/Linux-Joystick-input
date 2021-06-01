#ifndef JOYSTICK_MAP_H
#define JOYSTICK_MAP_H

#ifdef __cplusplus
extern "C"{
#endif


/*
 * Decription:
 * 	Used for mapping input to output. Indended to be used as a 
 * 	configurable layer between the input device and the application. 
 *
 * Notes:
 *
 * Error: 
 * 	Assert on logical. 
 */


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <joystick.h>

#define JOYSTICK_MAP_INPUT_MAX  JOYSTICK_AXIS_MAX
#define JOYSTICK_MAP_OUTPUT_MAX  JOYSTICK_AXIS_MAX


/* 
 * Joystick input map.
 *
 * X - joystick input vector. 
 * O - output values 
 * b - constant value that is added to each output.
 * A - matrix that maps input to output.  
 *
 * O = Ax + b
 */


/* 
 * Maps linear input to linear output.
 */

struct joystick_map
{
	/* Input is the same as columns in A */
	uint32_t map_input_count;

	/* Rows in A */
	uint32_t map_output_count;
	
	/* A - Matrix used for mixing. Input is N cols, Output is N row. */
	float map_matrix[JOYSTICK_MAP_INPUT_MAX][JOYSTICK_MAP_OUTPUT_MAX];	
	
	/* b - Offset */
	float map_offset[JOYSTICK_MAP_OUTPUT_MAX];
};

/* 
*  If the map is created with INPUT_COUNT And OUTPUT_COUNT must the OUTPUT_LENGTH, OUTPUT_SCALE_LENGTH be equal OUTPUT_COUNT. 
*  Will assert if not.
*/

/* 
 * Create a linear map. 
 * 
 * @param map empty joystick_map struct. 
 * 
 * @param input_count Number of inputs to map. Etc, the controller axis count. 
 * 
 * @param output_count Number of outputs to map. Etc, the required inputs to the application. 
 * 
 * @return Return -1 if the input_count or output_count is not between (inclusive) 1 and JOYSTICK_MAP_INPUT_MAX
 * or JOYSTICK_MAP_OUTPUT_MAX. 
 */

void joystick_map_create(struct joystick_map * const map, const uint32_t input_count, const uint32_t output_count);
void joystick_map_destroy(struct joystick_map * const map);


void joystick_map_transform(struct joystick_map * const map, const uint32_t input_index, const float *output_scale, const uint32_t output_scale_count);

void joystick_map_translate(struct joystick_map * const map, struct joystick_device *device,  float * const output, const uint32_t output_length);


void joystick_map_print(struct joystick_map * const map, FILE * const output);

#ifdef __cplusplus
}
#endif


#endif 
