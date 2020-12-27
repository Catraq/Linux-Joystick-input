#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include "joystick_ps3.h"
#include "joystick_map.h"



/* 
 * Joystick context. 
 */

static struct joystick_context joystick_controller;

/* 
 * This mapps the input from the joystick to the inputs in the appplciation.
 */
static struct joystick_map_ joystick_controller_map;

/* 
 * Minium rquirement of input device. 
 */

static struct joystick_input_requirement joystick_input_req = {
	.joystick_axis_count = 4,
	.joystick_button_count = 4,
};



int main(int args, char *argv[])
{
	int result = 0;
	const char *joystick_device_path = NULL;

	

	if(args < 2){
		fprintf(stdout, "Usage: %s ps3-dev-path \n", argv[0]);
		return -1;
	}
	

	/* 
	 * No joystick path passed from command line
	 */

	const size_t input_attrib_list_count = 4;
	struct joystick_input_attrib input_attrib_list[input_attrib_list_count];
	memset(&input_attrib_list, 0, sizeof input_attrib_list);
	if(joystick_device_path == NULL)
	{
		
		size_t number_of_joysticks = joystick_identify_by_requirement(&joystick_input_req, input_attrib_list, input_attrib_list_count);
		if(number_of_joysticks == 0){
			fprintf(stderr, "Could not find any joystick with \n");		
			exit(EXIT_FAILURE);
		}

		joystick_device_path = input_attrib_list[0].joystick_device_path;

	}


	result = joystick_init(&joystick_controller, joystick_device_path,  0);
	if(result < 0){
		fprintf(stderr, "joystick_ps3_intialize(): error \n");
		exit(EXIT_FAILURE);
	}
	else{

		/* Determined by the input joystick. */	
		const uint32_t linear_inputs = joystick_axis_count(&joystick_controller);

		/* Determined by the application requirement. */
		const uint32_t outputs = 4;

		
		result = joystick_map_create(&joystick_controller_map, linear_inputs, outputs);
		if(result < 0){
			fprintf(stderr, "joystick_map_init(): error \n");
			exit(EXIT_FAILURE);
		}

		/*
		 * Assing input index 0 to output index 1,4
		 */

		{	
			uint32_t input_index = 0;
			float output_channels[4] = {1.0f, 0.0f, 0.0f, 1.0f};
			uint32_t output_channel_count = outputs;

			result = joystick_map_mix(&joystick_controller_map, input_index, output_channels, output_channel_count);
			if(result < 0)
			{
				fprintf(stderr, "joystick_map_mix(): error \n");
				exit(EXIT_FAILURE);
			}
		}
	}

	

	/* Something....*/
	while(1)
	{
		struct joystick_ps3 input;
		
		clock_t time = clock();
		
		/* Read input vector */
		const uint32_t usec = 0;
		result = joystick_input(&joystick_controller, &input, usec);
		if(result < 0){
			printf("joystick_ps3_input(): error \n");
		}

		for(uint32_t i = 0; i < JOYSTICK_PS3_AXIS_LENGTH; i++){
			input.axis[i] = INT16_MAX;
		}
		
			
		float norm_output[4];
		float norm_input[JOYSTICK_PS3_AXIS_LENGTH];
		for(uint32_t i = 0; i < JOYSTICK_PS3_AXIS_LENGTH; i++)
		{
			/* 
			 * Input is in range int16_t. Convert to uint16_t range.
			 */
			norm_input[i] = (float)input.axis[i]/INT16_MAX;
		}

		
		result = joystick_map_translate(&joystick_controller_map, norm_input, JOYSTICK_PS3_AXIS_LENGTH, norm_output, 4);
		if(result < 0)
		{
			//TODO: Error, but what and how? 	
		}
	


		float dt = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
		printf("DT: %f :", dt);

		struct timespec loop_sleep = {
			.tv_sec = 0,
			.tv_nsec = (long)(100 * 1000 * 1000)

		};

		result = nanosleep(&loop_sleep, NULL);
		if(result < 0){
			printf("nanosleep(): error \n");
		}

		for(int i = 0; i < 4; i++){
			float value = norm_output[i];
			printf("%f,", value); 
		}
		printf("\n");


		if(input.button[JOYSTICK_PS3_BUTTON_CIRCLE] == 1){
			printf("JOYSTICK_PS3_BUTTON_CIRCLE pressed. Exiting. \n");	
			joystick_destroy(&joystick_controller);
			break;
		}


	}


	
	return 0;
}
