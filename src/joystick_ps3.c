#include "joystick_ps3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>



static int joystick_ps3_open(const char *device_path, int verbose)
{
	uint8_t axis_count, button_count;
	int8_t name[JOYSTICK_NAME_LENGTH];
	int device_fd;


	device_fd = open(device_path, O_RDONLY);
	if(device_fd < 0){
		
		if(verbose)
			fprintf(stdout, "PS3 Joystick: Could not open device %s. \n", device_path);
		return -1;
	}
	
	
	if(ioctl(device_fd, JSIOCGAXES, &axis_count) < 0){
		
		if(verbose)
			perror("PS3 Joystick:ioctl(JSIOCGAXES)");
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGBUTTONS, &button_count) < 0){
		
		if(verbose)
			perror("PS3 Joystick:ioctl(JSIOCGBUTTONS)");
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name) < 0)
	{
		if(verbose)
			perror("PS3 Joystick:ioctl(JOYSTICK_NAME_LENGTH)");
		goto exit;
	}
	
#if 0	
	if(axis_count != JOYSTICK_PS3_AXIS_LENGTH){
		
		if(verbose)
			fprintf(stderr, "Error: device have %u axises but a PS3 controller should have %u. \n", (uint32_t)axis_count, (uint32_t)JOYSTICK_PS3_AXIS_LENGTH);
		goto exit;	
	}	

	
	if(button_count != JOYSTICK_PS3_BUTTON_LENGTH){
		
		if(verbose)
			fprintf(stderr, "Error: device have %u buttons but a PS3 controller should have %u. \n", (uint32_t)button_count, (uint32_t)JOYSTICK_PS3_BUTTON_LENGTH);
		goto exit;	
	}
#endif 

	return device_fd;
	
exit:
	close(device_fd);
	return -1;
}

static void *joystick_input_thread(void *args)
{	
	assert(args);
	
	int result = 1;
	struct joystick_ps3_context *context = (struct joystick_ps3_context *)args;

	while(context->thread_state)
	{
		
		/* Error in last iteration, device is closed */
		if(result < 0){
			context->device_fd = joystick_ps3_open(context->device_path, 0);
			if(!(context->device_fd < 0))
			{
				result = 1;
			}
		}
		
		else
		{
			struct joystick_ps3 input;
			struct js_event event;
			
			memset(&input, 0, sizeof input);
			
			while((result = read(context->device_fd, &event, sizeof(event))) > 0 && context->thread_state)
			{
			
				if(result == sizeof(event))
				{
					
					/* Read event */
					switch(event.type & ~JS_EVENT_INIT)
					{
						case JS_EVENT_AXIS:
							input.axis[event.number] = event.value;
						break;	

						case JS_EVENT_BUTTON:
							input.button[event.number] = event.value;

						break;	
					}

					/* Write to shared memory */
					pthread_mutex_lock(&context->mutex);
					memcpy((uint8_t *)&context->input, (uint8_t *)&input, sizeof(input));
					pthread_cond_signal(&context->condition);
					pthread_mutex_unlock(&context->mutex);

				}
			}

			/* Error, close fd */
			if(result < 0)
			{
				fprintf(stderr, "input thread error: could not read control device.\n");
				close(context->device_fd);	
			}
		}
	}

	pthread_exit(NULL);
}

int joystick_ps3_input(struct joystick_ps3_context *context, struct joystick_ps3 *input, uint32_t timeout_usec)
{
	int result = 1;

	pthread_mutex_lock(&context->mutex);

	/* Grab latest data */
	memcpy(input, &context->input, sizeof (struct joystick_ps3));

	/* Timeout for most recent data */
	if(timeout_usec != 0)
	{

		/* Using absolue time */
		struct timeval time;
		gettimeofday(&time, NULL);

		struct timespec out = {
			.tv_sec = timeout_usec/(1000 * 1000),
			.tv_nsec = (long)timeout_usec * 1000
		};

		/* Wait for read thread */
		int result = pthread_cond_timedwait(&context->condition, &context->mutex, &out);
		if(errno == ETIMEDOUT){
			/* Timeout, have copied the data earlier in code */
			result = 0;	
		}else if(result < 0){
			perror("pthread_cond_timewait()");
			result = -1;
		}else{
			/* Copy most recent */
			memcpy(input, &context->input, sizeof (struct joystick_ps3));
			result = 1;
		}
	}

	pthread_mutex_unlock(&context->mutex);
	
	return result;
}



int joystick_ps3_initialize(struct joystick_ps3_context *context, const char *device_path, int device_must_exist)
{
	assert(context);
	assert(device_path);
	
	int result = 0;
	memset(context, 0, sizeof(struct joystick_ps3_context));

	
	const int verbose = 1;
	context->device_path = device_path;
	context->device_fd = joystick_ps3_open(context->device_path, verbose);
	
	if((context->device_fd < 0) && (device_must_exist == 1)){
		return -1;	
	}


	context->thread_state = 1;
	result = pthread_cond_init(&context->condition, NULL);
	assert(!(result < 0));
	
	result = pthread_mutex_init(&context->mutex, NULL);
	assert(!(result < 0));
	
	result = pthread_create(&context->thread, NULL, joystick_input_thread, (void*)context);
	assert(!(result < 0));
	

	return 0;

}


int joystick_ps3_destroy(struct joystick_ps3_context *context)
{
	context->thread_state = 0;

	int result = pthread_join(context->thread, NULL);
	assert(!(result < 0));

	result = pthread_cond_destroy(&context->condition);
	assert(!(result < 0));
	
	result = pthread_mutex_destroy(&context->mutex);
	assert(!(result < 0));

	close(context->device_fd);


	return 0;
}


