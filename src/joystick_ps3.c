#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <linux/limits.h>
#include <sys/types.h>


#include <pthread.h>

#include "joystick_ps3.h"

size_t joystick_identify_by_requirement(struct joystick_input_requirement *input_requirement,
	       	struct joystick_input_attrib *input_attrib,
	       	size_t input_attrib_max)
{

	assert(input_requirement != NULL);
	assert(input_attrib != NULL);
	assert(input_attrib_max > 0);

	
	/* 
	 * Open directroy with joystick devices. 
	 */

	struct dirent *dirent 	= NULL;
	const char *dev_path = "/dev/input/";
	DIR *dir = opendir(dev_path);

	if(dir == NULL){
		fprintf(stderr, "Joystick: Could not open directory %s. Error: %s ", dev_path, strerror(errno));
		return 0;
	}


	
	/*
	 * 	Write dev path to buffer without terminating zero. 
	 * 	PATH_MAX is max path length by definition in linux. 
	 */
	char full_file_path[PATH_MAX];
	size_t dev_path_length = strlen(dev_path);
	memcpy(full_file_path, dev_path, dev_path_length);
		

	/* 
	 * Increase for every joystick that fits requirements.
	 * Is less than input_attrib_max
	 */	

	size_t joystick_device_found_index = 0;


	/* 
	 *	Iterate over all filenames in the folder
	 *	and try to open them as joystick devices. 	
	 */
	while((dirent = readdir(dir)) != NULL)
	{
		/* 
		 * Fill buffer with a complete path 
		 * Last +1 is for the '0' terminator
		 */

		char *filename = dirent->d_name;	
		size_t filename_length = strlen(dev_path);
		size_t filename_last_index = filename_length + dev_path_length + 1; 
		if(filename_last_index < PATH_MAX)
		{

			char *full_file_path_filename_offset = full_file_path + dev_path_length;
			memcpy(full_file_path_filename_offset, dirent->d_name, filename_length);
			
			/* 
			 * Null terminate the string.
			 */

			full_file_path[filename_last_index] = 0;
			
			/* Try to open as joystick and Retrive attributes */
			fprintf(stderr, "Trying to open: %s \n", full_file_path);

			int verbose = 1;
			struct joystick_input_attrib curr_input_attrib;
			int joystick_device_fd = joystick_open(full_file_path, &curr_input_attrib, verbose);
			if(joystick_device_fd < 0)
			{
				/* Not a joystick device */	
			}
			else
			{
				/* 
				 * Close after opening.
				 */
				joystick_close(joystick_device_fd);

				/* 
				 * If it matches the requirements 
				 */

				if((input_requirement->joystick_axis_count <= curr_input_attrib.joystick_axis_count) &&
					       (input_requirement->joystick_button_count <= curr_input_attrib.joystick_axis_count))
				{

					struct joystick_input_attrib *attrib = &input_attrib[joystick_device_found_index];

					/* 
					 * Copy found attributes. 
					 */		
					memcpy(attrib, &curr_input_attrib, sizeof(curr_input_attrib));

					memcpy(attrib->joystick_device_path, full_file_path, PATH_MAX);
					
					/* 
					 * Quit if the array is full. 
					 */

					joystick_device_found_index = joystick_device_found_index + 1;
					if(joystick_device_found_index == input_attrib_max){
						break;	
					}
				
				}

			}
		
		}else
		{
			/* CANT HAPPEN! Write some log error.  */	
		}

	}

	closedir(dir);

	return joystick_device_found_index;
}



int joystick_open(const char *device_path, struct joystick_input_attrib *input_attrib, int verbose)
{
	assert(device_path != NULL);
	assert(input_attrib != NULL);

	uint8_t axis_count, button_count;
	int8_t name[JOYSTICK_NAME_LENGTH];
	int device_fd;


	device_fd = open(device_path, O_RDONLY);
	if(device_fd < 0){
		
		if(verbose)
			fprintf(stdout, "Joystick: Could not open device %s. \n", device_path);
		return -1;
	}
	
	
	if(ioctl(device_fd, JSIOCGAXES, &axis_count) < 0){
		
		if(verbose)
			perror("Joystick:ioctl(JSIOCGAXES)");
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGBUTTONS, &button_count) < 0){
		
		if(verbose)
			perror("Joystick:ioctl(JSIOCGBUTTONS)");
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name) < 0)
	{
		if(verbose)
			perror("Joystick:ioctl(JOYSTICK_NAME_LENGTH)");
		goto exit;
	}

	input_attrib->joystick_axis_count = axis_count;
	input_attrib->joystick_button_count = button_count;
	memcpy(input_attrib->joystick_name, name, sizeof(input_attrib->joystick_name));


	return device_fd;
	
exit:
	close(device_fd);
	return -1;
}

int joystick_close(int joystick_fd)
{
	close(joystick_fd);

	return 0;
}


static void *joystick_input_thread(void *args)
{	
	assert(args);
	
	int result = 1;
	struct joystick_context *context = (struct joystick_context *)args;

	while(context->thread_state)
	{
		
		/* Error in last iteration, device is closed */
		if(result < 0){

			/*TODO: This have to match previous */
			struct joystick_input_attrib input_attrib;
			context->device_fd = joystick_open(context->device_path, &input_attrib, 0);
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
			
			ssize_t bytes_read = 0;			
			while((bytes_read= read(context->device_fd, &event, sizeof(event))) > 0 && context->thread_state)
			{
			
				if(bytes_read == (ssize_t)sizeof(event))
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

uint32_t joystick_axis_count(struct joystick_context *context)
{
	return context->input_attrib.joystick_axis_count;
}

int joystick_input(struct joystick_context *context, struct joystick_ps3 *input, uint32_t timeout_usec)
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



int joystick_init(struct joystick_context *context, const char *device_path, int device_must_exist)
{
	assert(context != NULL);
	
		
	int result = 0;
	memset(context, 0, sizeof(struct joystick_context));
	
	/* 
	 * Open device. Get attributes 
	 */

	int verbose = 1;	
	context->device_path = device_path;
	context->device_fd = joystick_open(context->device_path, &context->input_attrib, verbose);
	
	if((context->device_fd < 0) && (device_must_exist == 1)){
		return -1;	
	}
	/* 
	 * Allocate memory for all the inputs 
	 * NULL if it fails. 
	 */	

	context->input_value.joystick_axis_value = (int16_t *)malloc(sizeof(int16_t) * context->input_attrib.joystick_axis_count);
	context->input_value.joystick_button_value = (int16_t *)malloc(sizeof(int16_t) * context->input_attrib.joystick_button_count);

	if(context->input_value.joystick_axis_value == NULL){
		goto error;					
	}

	if(context->input_value.joystick_button_value == NULL){
		goto error;	
	}


	context->thread_state = 1;
	result = pthread_cond_init(&context->condition, NULL);
	assert(!(result < 0));
	
	result = pthread_mutex_init(&context->mutex, NULL);
	assert(!(result < 0));
	
	result = pthread_create(&context->thread, NULL, joystick_input_thread, (void*)context);
	assert(!(result < 0));
	

	return 0;

error:
	close(context->device_fd);


	/*
	 * Free NULL is fine
	 */

	free(context->input_value.joystick_axis_value);
	free(context->input_value.joystick_button_value);

	return -1;
}


int joystick_destroy(struct joystick_context *context)
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


