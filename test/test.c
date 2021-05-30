#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>


#define JOYSTICK_LOG_TAG 
#include "joystick.h"
#include "joystick_map.h"



static FILE *joystick_log_tag = NULL;

/* 
 * Joystick requirement 
 */

#define APP_INPUT_AXIS_REQ 	2
#define APP_INPUT_BUTTON_REQ 	1


/*
 * The input is translated into this many application inputs.  
 */

#define APP_INPUTS 6


/* 
 * Joystick context. 
 */

static struct joystick_device joystick_controller;

/* 
 * This mapps the input from the joystick to the inputs in the appplciation.
 */
static struct joystick_map joystick_controller_map;

/* 
 * Minium rquirement of input device. 
 */

static struct joystick_input_requirement joystick_input_req = {
	.joystick_axis_count = APP_INPUT_AXIS_REQ,
	.joystick_button_count = APP_INPUT_BUTTON_REQ,
};

int main(int args, char *argv[])
{
	joystick_log_tag = stdout;

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
		
		size_t number_of_joysticks = joystick_device_identify_by_requirement(&joystick_input_req, input_attrib_list, input_attrib_list_count);
		if(number_of_joysticks == 0){
			fprintf(stderr, "Could not find any joystick device \n");		
		}else
		{
			joystick_device_path = input_attrib_list[0].joystick_device_path;
			joystick_input_attrib_print(&input_attrib_list[0],stdout);
		}

	}

	
	if((joystick_device_path == NULL) && (args < 2)){
		fprintf(stdout, "Usage: %s ps3-dev-path \n", argv[0]);
		exit(EXIT_FAILURE);
	}else if(args == 2)
	{
		joystick_device_path = argv[2];
	}


	result = joystick_device_open(&joystick_controller, joystick_device_path);
	if(result < 0){
		fprintf(stderr, "joystick_ps3_intialize(): error \n");
		exit(EXIT_FAILURE);
	}
	else{

		/* Determined by the input joystick HW. */	
		const uint32_t linear_inputs = joystick_device_axis_count(&joystick_controller);

		/* Determined by the application requirement. */
		const uint32_t outputs = APP_INPUTS;

		
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
			float output_channels[APP_INPUTS] = {1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
			uint32_t output_channel_count = outputs;

			result = joystick_map_mix(&joystick_controller_map, input_index, output_channels, output_channel_count);
			if(result < 0)
			{
				fprintf(stderr, "joystick_map_mix(): error \n");
				exit(EXIT_FAILURE);
			}
		}
	}

	while(1)
	{
		
		clock_t time = clock();
		float output[APP_INPUTS];
		const uint32_t output_count = APP_INPUTS;
		

		result = joystick_device_poll(&joystick_controller);
		if(result < 0){
			result = joystick_device_reopen(&joystick_controller, joystick_device_path);
			(void)result;
		}
		else if(result > 0)
		{
			joystick_map_translate(&joystick_controller_map, &joystick_controller, output, output_count);
						
#if 1
			float dt = (float)(clock() - time)/(float)CLOCKS_PER_SEC;
			printf("DT: %f :", dt);

			for(int i = 0; i < APP_INPUTS; i++){
				printf("%f,", output[i]); 
			}
			printf("\n");

#endif 
		}
			

	}

	joystick_map_destroy(&joystick_controller_map);
	joystick_device_close(&joystick_controller);

	
	return 0;
}
