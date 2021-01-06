#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>


#include <pthread.h>

#include "joystick.h"

/* Write log to file */
#define LOG_TAG "JOYSTICK"


#ifdef JOYSTICK_LOG_TAG
#define LOG_FD JOYSTICK_LOG_FD
#else
#define LOG_FD stderr
#endif 

#include "li_log.h"

#define JOYSTICK_EVENT_BUFFER_SIZE 128

int joystick_input(struct joystick_context *context, struct joystick_input_value *input)
{
	int result = 0;
		
	struct js_event js_event_buffer[JOYSTICK_EVENT_BUFFER_SIZE];
	result = read(context->device_fd, js_event_buffer, sizeof js_event_buffer);
	if((result == -1)  && (errno != EAGAIN)){
		return -1;	
	}
	
	const uint32_t buffer_size_verify = result%sizeof(struct js_event); 
	const uint32_t buffer_size = result/sizeof(struct js_event); 
	if(buffer_size_verify == 0)
	{

		const int16_t joystick_axis_count = context->input_attrib.joystick_axis_count;
		const int16_t joystick_button_count = context->input_attrib.joystick_button_count;

		for(int i = 0; i < buffer_size; i++)
		{
			/*
			 * Read event and write to input buffer
			 */

			__u8 number = js_event_buffer[i].number;
			__s16 value = js_event_buffer[i].value;
			switch(js_event_buffer[i].type & ~JS_EVENT_INIT)
			{


				case JS_EVENT_AXIS:
					if(number < joystick_axis_count){
						input->joystick_axis_value[number] = value;
					}
				break;	

				case JS_EVENT_BUTTON:
					if(number < joystick_button_count){
						input->joystick_button_value[number] = value;
					}
				break;	
			}

		}
	}

	return 0;
}


size_t joystick_identify_by_requirement(struct joystick_input_requirement *input_requirement,
	       	struct joystick_input_attrib *input_attrib,
	       	size_t input_attrib_max)
{

	assert(input_requirement != NULL);
	assert(input_attrib != NULL);
	assert(input_attrib_max > 0);

	/* 
	 * Increase for every joystick that fits requirements.
	 * Is less than input_attrib_max
	 */	

	size_t joystick_device_found_index = 0;

	
	/* 
	 * Open directroy with joystick devices. 
	 */

	struct dirent *dirent = NULL;
	const char *dev_path = "/dev/input/";

	LOGI("Finding joysticks in %s with requirements(", dev_path);
	LOG_MSG("axis count=%lu, ", (long unsigned int)input_requirement->joystick_axis_count);
	LOG_MSG("button count=%lu) \n", (long unsigned int)input_requirement->joystick_button_count);



	DIR *dir = opendir(dev_path);

	if(dir == NULL){
		LOGE("Could not open directory %s. Error: %s ", dev_path, strerror(errno));
		return joystick_device_found_index;
	}


	
	/*
	 * 	Write dev path to buffer without terminating zero. 
	 * 	PATH_MAX is max path length by definition in linux. 
	 */
	char full_file_path[PATH_MAX];
	size_t dev_path_length = strlen(dev_path);
	memcpy(full_file_path, dev_path, dev_path_length);
		


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
			
			/* Note that full_file_path is pointer, */
			char *full_file_path_filename_offset = (char *)full_file_path + dev_path_length;
			memcpy(full_file_path_filename_offset, dirent->d_name, filename_length);
			
			/* 
			 * Null terminate the string.
			 */

			full_file_path[filename_last_index] = 0;
			
			struct joystick_input_attrib curr_input_attrib;
			int joystick_device_fd = joystick_open(full_file_path, &curr_input_attrib);
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
				
				LOGI("Name=%s, ", curr_input_attrib.joystick_name);
				LOG_MSG("axis count=%lu, ", (long unsigned int)curr_input_attrib.joystick_axis_count);
				LOG_MSG("button count=%lu \n", (long unsigned int)curr_input_attrib.joystick_button_count);


				if((input_requirement->joystick_axis_count <= curr_input_attrib.joystick_axis_count) &&
					       (input_requirement->joystick_button_count <= curr_input_attrib.joystick_button_count))
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
		}
	}

	closedir(dir);
	LOGI("Found %li devices satsifying the requirements. \n", joystick_device_found_index);

	return joystick_device_found_index;
}



int joystick_open(const char *device_path, struct joystick_input_attrib *input_attrib)
{
	assert(device_path != NULL);
	assert(input_attrib != NULL);

	uint8_t axis_count, button_count;
	int8_t name[JOYSTICK_NAME_LENGTH];
	int device_fd;


	device_fd = open(device_path, O_RDONLY | O_NONBLOCK);
	if(device_fd < 0){
		return -1;
	}
	
	
	if(ioctl(device_fd, JSIOCGAXES, &axis_count) < 0){
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGBUTTONS, &button_count) < 0){
		
		goto exit;
	}
	
	if(ioctl(device_fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name) < 0)
	{
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

uint32_t joystick_axis_count(struct joystick_context *context)
{
	return context->input_attrib.joystick_axis_count;
}


int joystick_input_create(struct joystick_context *context, struct joystick_input_value *value)
{
	assert(context != NULL);
	assert(value != NULL);


	/* 
	 * Allocate memory for all the inputs 
	 * According to the linux doc are all joystick input in the signed 
	 * int16 range. 
	 */	

	value->joystick_axis_value = (int16_t *)malloc(sizeof(int16_t) * context->input_attrib.joystick_axis_count);
	value->joystick_button_value = (int16_t *)malloc(sizeof(int16_t) * context->input_attrib.joystick_button_count);

	if(value->joystick_axis_value == NULL){
		goto error;					
	}

	if(value->joystick_button_value == NULL){
		goto error;	
	}

	return 0;
error:

	/*
	 * Free NULL is fine
	 */

	free(value->joystick_axis_value);
	free(value->joystick_button_value);

	return -1;
}

void joystick_input_clear(struct joystick_context *context, struct joystick_input_value *value)
{
	assert(context != NULL);
	assert(value != NULL);


	memset(value->joystick_axis_value, 0, sizeof(int16_t) * context->input_attrib.joystick_axis_count);
	memset(value->joystick_button_value, 0, sizeof(int16_t) * context->input_attrib.joystick_button_count);

}

int joystick_input_destroy(struct joystick_input_value *value)
{
	assert(value != NULL);

	free(value->joystick_axis_value);
	free(value->joystick_button_value);

	return 0;
}

int joystick_init(struct joystick_context *context, const char *device_path)
{
	assert(context != NULL);
	assert(device_path != NULL);
	
		
	int result = 0;
	memset(context, 0, sizeof(struct joystick_context));
	
	/* 
	 * Open device and get attributes.
	 */

	context->device_fd = joystick_open(device_path, &context->input_attrib);
	
	if(context->device_fd < 0){
		return -1;	
	}


	return 0;

error:
	close(context->device_fd);

	return -1;
}


int joystick_destroy(struct joystick_context *context)
{
	assert(context != NULL);

	close(context->device_fd);

	return 0;
}

