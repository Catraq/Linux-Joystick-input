/*
 * Decription:
 * 	Used for mapping input to output. Indended to be used as a 
 * 	configurable layer between the input device and the application. 
 * Notes
 *
 * Error 
 * 	if malloc fails. 
 * 	Assert on logical. 
 */


#ifndef JOYSTICK_MAP_H
#define JOYSTICK_MAP_H

#include <assert.h>

#include <stddef.h>
#include <stdint.h>


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



int joystick_map_create(struct joystick_map * const map, const uint32_t input, const uint32_t output)
{
	assert(map != NULL);

	assert(input != 0);
	assert(output != 0);

	memset(map, 0, sizeof(struct joystick_map));	

	
	map->matrix = (float *)malloc(sizeof(float)*input*output);
	map->offset = (float *)malloc(sizeof(float)*output);

	if(map->matrix == NULL){
		goto cleanup_failure;
	}

	if(map->offset == NULL){
		goto cleanup_failure;
	}

	map->outputs 	= output;
	map->inputs 	= input;

	for(uint32_t i = 0; i < input*output; i++){
		map->matrix[i] = 0.0f;
	}
	
	for(uint32_t i = 0; i < output; i++){
		map->offset[i] = 0.0f;
	}

	return 0;

cleanup_failure:
	free(map->matrix);
	free(map->offset);

	return -1;

}

int joystick_map_destroy(struct joystick_map * const map)
{
	assert(map != NULL);
	assert(map->matrix != NULL);
	assert(map->offset != NULL);
	
	free(map->matrix);
	free(map->offset);

	return 0;
}


int joystick_map_mix(struct joystick_map * const map, const uint32_t input_index, const float *output_scale, const uint32_t output_scale_count)
{
	assert(map != NULL);
	assert(map->matrix != NULL);
	assert(map->offset != NULL);

	assert(output_scale != NULL);
	assert(output_scale_count > 0);
	
	
	/*		
	*	cols*i + j < input * output 
	*
	*	So max(i) have to be number of rows - 1 
	*	and the max(j) should be max columns
	*/

	const uint32_t col_i = input_index;

	const uint32_t cols = map->inputs;
	const uint32_t rows = map->outputs;

	assert(!(col_i >= map->inputs));
	assert(!(output_scale_count > rows));

	for(uint32_t i = 0; i < output_scale_count; i++)
	{
		/* Update value */
		uint32_t index 	= cols*i + col_i;
		map->matrix[index] = output_scale[i];
	}

		
	return 0;
}

void joystick_map_print(struct joystick_map * const map, FILE * const output)
{
	const uint32_t cols = map->inputs;
	/* 
	 * Inner is column and outer is row
	 */

	for(uint32_t j = 0; j < map->outputs; j++){

		for(uint32_t i = 0; i < map->inputs; i++){

			uint32_t index = cols*j + i;
			float e = map->matrix[index];

			fprintf(output, "i=%i, %f,", index, e);
		}	

		fprintf(output, "\n");
	}	

}


int joystick_map_translate(struct joystick_map * const map, struct joystick_device *device,  float * const output, const uint32_t output_length)
{
	assert(map != NULL);
	assert(map->matrix != NULL);
	assert(map->offset != NULL);


	float *input = device->input_value.joystick_axis_value;	
	uint32_t input_length = device->input_attrib.joystick_axis_count;

	assert(input != NULL);
	assert(output != NULL);
		
	/* Boundary check */
	assert(input_length == map->inputs);
	assert(output_length == map->outputs);
	
	/* Avoid confusing ourself */
	const uint32_t cols = map->inputs;


	/*
	 * Matrix vector mul. Ax = b
	 * Matrix is in row order. (1, 1) => [0], ... (M, N) => [(M-1)*N + N-1], 
	 */

	for(uint32_t i = 0; i < output_length; i++)
	{
		float o_i = map->offset[i];

		for(uint32_t j = 0; j < input_length; j++)	
		{
			float v = input[j];
			float e = map->matrix[i*cols + j];
			o_i = o_i + v*e;
		}

		output[i] = o_i;
	}
	
	return 0;
}


#endif 
