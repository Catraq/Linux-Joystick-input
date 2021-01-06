#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include "joystick.h"
#include "joystick_map.h"


/* 
 * Joystick requirement 
 */
const uint32_t app_input_axis_req = 2;
const uint32_t app_input_button_req = 1;


/*
 * The input is translated into this many application inputs.  
 */
const uint32_t app_inputs = 6;


/* 
 * Joystick context. 
 */

static struct joystick_context joystick_controller;

/* 
 * This mapps the input from the joystick to the inputs in the appplciation.
 */
static struct joystick_map joystick_controller_map;

/* 
 * Minium rquirement of input device. 
 */

static struct joystick_input_requirement joystick_input_req = {
	.joystick_axis_count = app_input_axis_req,
	.joystick_button_count = app_input_button_req,
};



int main(int args, char *argv[])
{
	int result = 0;
	const char *joystick_device_path = NULL;

	/* 
	 * If no joystick path is passed from command line, then 
	 * try to find joystick that satisfy the requirements.
	 */

	const size_t input_attrib_list_count = 8; // This number is arbitary 
	struct joystick_input_attrib input_attrib_list[input_attrib_list_count];
	memset(&input_attrib_list, 0, sizeof input_attrib_list);


	if(joystick_device_path == NULL)
	{
		
		size_t number_of_joysticks = joystick_identify_by_requirement(&joystick_input_req, input_attrib_list, input_attrib_list_count);
		if(number_of_joysticks == 0){
			fprintf(stderr, "Could not find any joystick device \n");		
		}else
		{
			fprintf(stderr, "Found device");
			/* Pick first found */
			joystick_device_path = input_attrib_list[0].joystick_device_path;
		}

	}

	
	if((joystick_device_path == NULL) && (args < 2)){
		fprintf(stdout, "Usage: %s ps3-dev-path \n", argv[0]);
		exit(EXIT_FAILURE);
	}else if(args == 2)
	{
		joystick_device_path = argv[2];
	}


	result = joystick_init(&joystick_controller, joystick_device_path);
	if(result < 0){
		fprintf(stderr, "joystick_ps3_intialize(): error \n");
		exit(EXIT_FAILURE);
	}
	else{

		/* Determined by the input joystick HW. */	
		const uint32_t linear_inputs = joystick_axis_count(&joystick_controller);

		/* Determined by the application requirement. */
		const uint32_t outputs = app_inputs;

		
		result = joystick_map_create(&joystick_controller_map, linear_inputs, outputs);
		if(result < 0){
			fprintf(stderr, "joystick_map_init(): error \n");
			exit(EXIT_FAILURE);
		}

		/*
		 * Assing input index 0 to output index 1,4,5,6
		 */

		{	
			uint32_t input_index = 0;
			float output_channels[6] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
			uint32_t output_channel_count = outputs;

			result = joystick_map_mix(&joystick_controller_map, input_index, output_channels, output_channel_count);
			if(result < 0)
			{
				fprintf(stderr, "joystick_map_mix(): error \n");
				exit(EXIT_FAILURE);
			}
		}
	}

	struct joystick_input_value input_values;
       	result = joystick_input_create(&joystick_controller, &input_values);
	if(result < 0)
	{
		fprintf(stderr, "joystick_input_create(): error \n");
		exit(EXIT_FAILURE);
	}


       	joystick_input_clear(&joystick_controller, &input_values);


	while(1)
	{
		
		clock_t time = clock();
		
		result = joystick_input(&joystick_controller, &input_values);
		if(result < 0){
			//Invalid state. What todo? Reopen device! 
			printf("joystick_input(): error \n");
		}

		
			
		float norm_output[6];
		float norm_input[2];
		for(uint32_t i = 0; i < 2; i++)
		{
			/* 
			 * Input is in range int16_t. Convert to uint16_t range.
			 */
			norm_input[i] = (float)(input_values.joystick_axis_value[i])/INT16_MAX;
		}

		
		result = joystick_map_translate(&joystick_controller_map, norm_input, 2, norm_output, 6);
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

		for(int i = 0; i < 6; i++){
			float value = norm_output[i];
			printf("%f,", value); 
		}
		printf("\n");

#if 0
		if(input.button[JOYSTICK_PS3_BUTTON_CIRCLE] == 1){
			printf("JOYSTICK_PS3_BUTTON_CIRCLE pressed. Exiting. \n");	
			joystick_destroy(&joystick_controller);
			break;
		}
#endif 


	}


	
	return 0;
}
