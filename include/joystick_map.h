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
 * 	if malloc fails. 
 * 	Assert on logical. 
 */


#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <joystick.h>


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
	uint32_t inputs;

	/* Rows in A */
	uint32_t outputs;
	
	/* A - Matrix used for mixing */
	float *matrix;	
	
	/* b - Offset */
	float *offset;
};



int joystick_map_create(struct joystick_map * const map, const uint32_t input, const uint32_t output);
int joystick_map_destroy(struct joystick_map * const map);


int joystick_map_mix(struct joystick_map * const map, const uint32_t input_index, const float *output_scale, const uint32_t output_scale_count);

void joystick_map_print(struct joystick_map * const map, FILE * const output);



void joystick_map_translate(struct joystick_map * const map, struct joystick_device *device,  float * const output, const uint32_t output_length);


#ifdef __cplusplus
}
#endif


#endif 
