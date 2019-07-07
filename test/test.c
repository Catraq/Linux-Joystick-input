#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>

#include "joystick_ps3.h"

int main(int args, char *argv[])
{
	int result = 0;
	struct joystick_ps3_context ps3;

	if(args < 2){
		fprintf(stdout, "Usage: %s ps3-dev-path \n", argv[0]);
		return -1;
	}	
	
	const char *device_path = argv[1];
	const uint32_t refresh_interval_ms = 10;
	result = joystick_ps3_intialize(&ps3, device_path);
	if(result < 0){
		/* Error */	
	}

	/* Something....*/
	while(1)
	{
		struct joystick_ps3 input;

		struct timeval time;	
		gettimeofday(&time, NULL);
		
		/* 1 ms timeout */
		struct timespec timeout = 
		{
			.tv_sec = time.tv_sec,
			.tv_nsec = (long)time.tv_usec * 1000 + (long)1000*1000
		};

		result = joystick_ps3_input(&ps3, &input, &timeout);
		if(result < 0){
			printf("joystick_ps3_input(): error \n");
		}

		struct timespec loop_sleep = {
			.tv_sec = 0,
			.tv_nsec = (long)(100 * 1000 * 1000)

		};

		result = nanosleep(&loop_sleep, NULL);
		if(result < 0){
			printf("nanosleep(): error \n");
		}

		for(int i = 0; i < JOYSTICK_PS3_BUTTON_LENGTH; i++){
			int value = input.button[i];
			printf("%i,", value); 
		}	

		for(int i = 0; i < JOYSTICK_PS3_AXIS_LENGTH; i++){
			int value = input.axis[i];
			printf("%i,", value); 
		}
		printf("\n");

		if(input.button[JOYSTICK_PS3_BUTTON_CIRCLE] == 1){
			printf("JOYSTICK_PS3_BUTTON_CIRCLE pressed. Exiting. \n");	
			joystick_ps3_destroy(&ps3);
			break;
		}


	}


	
	return 0;
}
