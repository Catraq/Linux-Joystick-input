#include "joystick_ps3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>



void *joystick_input_thread(void *args)
{	
	int result = 1;
	struct joystick_ps3_context *config = (struct joystick_ps3_context *)args;

	while(config->thread_state)
	{
		if(result < 0){

			uint8_t axis_count, button_count;
			uint8_t name[JOYSTICK_NAME_LENGTH];
			int fd = open(config->device_path, O_RDONLY);
			if(!(fd < 0))
			{
				ioctl(fd, JSIOCGAXES, &axis_count);
				ioctl(fd, JSIOCGBUTTONS, &button_count);
				ioctl(fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name);
				config->device_fd = fd;
				result = 0;
			}


		}
		else
		{
			struct joystick_ps3 input;
			struct js_event event;
			while((result = read(config->device_fd, &event, sizeof(event))) > 0 && config->thread_state)
			{
				if(result == sizeof(event))
				{
					switch(event.type & ~JS_EVENT_INIT)
					{
						case JS_EVENT_AXIS:
							input.axis[event.number] = event.value;
						break;	

						case JS_EVENT_BUTTON:
							input.button[event.number] = event.value;

						break;	
					}

					pthread_mutex_lock(&config->mutex);
					memcpy((uint8_t *)&config->input, (uint8_t *)&input, sizeof(input));
					pthread_cond_signal(&config->condition);
					pthread_mutex_unlock(&config->mutex);

				}
			}


			if(result < 0)
			{
				fprintf(stderr, "input thread error: could not read control device.\n");
				close(config->device_fd);	
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
			/* Have copied the data earlier in code */
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
	int result = 0;
		

	memset(context, 0, sizeof(struct joystick_ps3_context));

	uint8_t axis_count, button_count;
	int8_t name[JOYSTICK_NAME_LENGTH];

	const char expected_name[] = "PS3";
	
	context->thread_state = 1;
	context->device_path = device_path;
	context->device_fd = -1;
	
	context->device_fd = open(device_path, O_RDONLY);
	
	int device_found = 0;
	if(context->device_fd < 0){
		fprintf(stdout, "PS3 Joystick: Could not open device %s. \n", device_path);
	}else{

		ioctl(context->device_fd, JSIOCGAXES, &axis_count);
		ioctl(context->device_fd, JSIOCGBUTTONS, &button_count);
		ioctl(context->device_fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name);
		
		if(axis_count != JOYSTICK_PS3_AXIS_LENGTH){
			fprintf(stderr, "Error: device have %u axises but a PS3 controller should have %u. \n", (uint32_t)axis_count, (uint32_t)JOYSTICK_PS3_AXIS_LENGTH);
			goto exit;	
		}	

		
		if(button_count != JOYSTICK_PS3_BUTTON_LENGTH){
			fprintf(stderr, "Error: device have %u buttons but a PS3 controller should have %u. \n", (uint32_t)button_count, (uint32_t)JOYSTICK_PS3_BUTTON_LENGTH);
			goto exit;	
		}

		device_found = 1;
	}	

	if(device_found == 0 && device_must_exist == 1){
		return -1;	
	}



	result = pthread_cond_init(&context->condition, NULL);
	if(result < 0){
		perror("pthread_cond_init()");
		goto exit;
	}
	
	result = pthread_mutex_init(&context->mutex, NULL);
	if(result < 0){
		pthread_cond_destroy(&context->condition);
		perror("pthread_mutex_init()");
		goto exit;	
	}

	result = pthread_create(&context->thread, NULL, joystick_input_thread, (void*)context);
	if(result < 0){
	
		pthread_cond_destroy(&context->condition);
		pthread_mutex_destroy(&context->mutex);

		perror("pthread_create()");
		goto exit;	
	}
	fprintf(stdout, "%s \n", name);	

	return 0;

exit:
	close(context->device_fd);
	return -1;
}


int joystick_ps3_destroy(struct joystick_ps3_context *context)
{
	context->thread_state = 0;

	int result = pthread_join(context->thread, NULL);
	if(result < 0)
	{
		perror("pthread_join()");
		return -1;	
	}

	result = pthread_cond_destroy(&context->condition);
	if(result < 0){
		perror("pthread_cond_destroy()");
		return -1;	
	}
	
	result = pthread_mutex_destroy(&context->mutex);
	if(result < 0){
		perror("pthread_mutex_destroy()");
		return -1;	
	}

	close(context->device_fd);


	return 0;
}


