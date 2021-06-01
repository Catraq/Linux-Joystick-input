#include <assert.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "joystick_map.h"


void joystick_map_create(struct joystick_map * const map, const uint32_t input_count, const uint32_t output_count)
{
	assert(map != NULL);
	assert(input_count <= JOYSTICK_MAP_INPUT_MAX);
	assert(output_count <= JOYSTICK_MAP_OUTPUT_MAX);

	memset(map, 0, sizeof(struct joystick_map));	

	map->map_output_count = output_count;
	map->map_input_count = input_count;
	
}

void joystick_map_destroy(struct joystick_map * const map)
{

}


void joystick_map_transform(struct joystick_map * const map, const uint32_t input_index, const float *output_scale, const uint32_t output_scale_count)
{
	assert(map != NULL);

	assert(output_scale != NULL);
	assert(output_scale_count == map->map_output_count);
	assert(input_index < map->map_input_count);	

	const uint32_t col_i = input_index;

	for(uint32_t i = 0; i < map->map_output_count; i++)
	{
		map->map_matrix[col_i][i] = output_scale[i];
	}

}

void joystick_map_translate(struct joystick_map * const map, struct joystick_device *device,  float * const output, const uint32_t output_length)
{
	assert(map != NULL);
	assert(output != NULL);
		
	assert(output_length == map->map_output_count);

	float *input = device->input_value.joystick_axis_value;	

	/*
	 * Matrix vector mul. Ax = b
	 */

	for(uint32_t i = 0; i < map->map_output_count; i++)
	{
		float o_i = map->map_offset[i];

		for(uint32_t j = 0; j < map->map_input_count; j++)	
		{
			float v = input[j];
			float e = map->map_matrix[j][i];
			o_i = o_i + v*e;
		}

		output[i] = o_i;
	}
}

void joystick_map_print(struct joystick_map * const map, FILE * const output)
{
	/* 
	 * Inner is column and outer is row.
	 */
#if 0
	for(uint32_t j = 0; j < map->map_output_count; j++){
		for(uint32_t i = 0; i < map->map_input_count; i++){
			float e = map->map_matrix[i][j];
			fprintf(output, "i=%i, %f,", index, e);
		}	
		fprintf(output, "\n");
	}	
#endif
}


