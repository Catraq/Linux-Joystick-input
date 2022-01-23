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


#define JOYSTICK_EVENT_BUFFER_SIZE 128



int 
joystick_device_close(struct joystick_device *device)
{
	assert(device != NULL);

	close(device->device_fd);
	device->device_fd = -1;

	return 0;
}

void joystick_input_attrib_print(struct joystick_input_attrib *input_attrib, FILE *out)
{
	assert(input_attrib != NULL);
	assert(out != NULL);
	

	fprintf(out,"joystick_device_path={%s} \n", input_attrib->joystick_device_path);
	fprintf(out,"joystick_name={%s} \n", input_attrib->joystick_name);
	fprintf(out,"joystick_axis_count={%i} \n", (int)input_attrib->joystick_axis_count);
	fprintf(out,"joystick_button_count={%i} \n", (int)input_attrib->joystick_button_count);


}

uint32_t joystick_device_axis_count(struct joystick_device *device)
{
	assert(device != NULL);

	return device->input_attrib.joystick_axis_count;
}


int joystick_device_is_open(struct joystick_device *device)
{
	assert(device != NULL);

	return device->device_fd <= 0 ? -1 : 1;
}

int joystick_device_poll(struct joystick_device *device)
{
	assert(device != NULL);

	
	struct js_event js_event_buffer[JOYSTICK_EVENT_BUFFER_SIZE];
	ssize_t bytes_read = read(device->device_fd, js_event_buffer, sizeof js_event_buffer);
	if((bytes_read < 0)  && (errno != EAGAIN)){
		close(device->device_fd);
		device->device_fd = -1;
		return -1;	
	}
	
	const uint32_t buffer_size_verify = ((size_t)bytes_read)%sizeof(struct js_event); 
	if(buffer_size_verify == 0)
	{

		const size_t buffer_size = ((size_t)bytes_read)/sizeof(struct js_event); 

		const int16_t joystick_axis_count = device->input_attrib.joystick_axis_count;
		const int16_t joystick_button_count = device->input_attrib.joystick_button_count;

		for(size_t i = 0; i < buffer_size; i++)
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

						/* Map INT16_T range to float [-1, 1] */
						float mapped = ((float)value)/((float)INT16_MAX);
						device->input_value.joystick_axis_value[number] = mapped;
					}
				break;	

				case JS_EVENT_BUTTON:
					if(number < joystick_button_count){
						device->input_value.joystick_button_value[number] = value;
					}
				break;	
			}

		}

		return 1;
	}

	return 0;
}

static int joystick_open(const char *device_path, struct joystick_input_attrib *input_attrib)
{
	assert(device_path != NULL);
	assert(input_attrib != NULL);

	uint8_t axis_count, button_count;
	int8_t name[JOYSTICK_NAME_LENGTH];
	int device_fd = -1;


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
	
	/* Will be null termianted */	
	if(ioctl(device_fd, JSIOCGNAME(JOYSTICK_NAME_LENGTH), name) < 0)
	{
		goto exit;
	}


	if(axis_count > JOYSTICK_AXIS_MAX)
	{
		goto exit;
	}

	if(button_count > JOYSTICK_BUTTON_MAX)
	{
		goto exit;
	}

	input_attrib->joystick_axis_count = axis_count;
	input_attrib->joystick_button_count = button_count;

	strncpy((char *)input_attrib->joystick_name, (char *)name, sizeof(input_attrib->joystick_name));

	/* +1 for NULL */
	size_t device_path_len = strlen(device_path) + 1;
	memcpy((char *)input_attrib->joystick_device_path, (char *)device_path, device_path_len);

	return device_fd;
	
exit:
	close(device_fd);
	return -1;
}

int 
joystick_device_reopen(struct joystick_device *device)
{
	assert(device != NULL);

	int result = 0;

		
	if(device->device_fd != -1)
	{
		return 0;	
	}
	
	
	/*
	 * Try to open the device and 
	 * make sure that it is the same device. 
	 */
	
	uint8_t *device_path = device->input_attrib.joystick_device_path;

	struct joystick_input_attrib input_attrib;
	memset(&input_attrib, 0, sizeof(input_attrib));

	device->device_fd = joystick_open((const char *)device_path, &input_attrib);
	if(device->device_fd < 0){
		return -1;
	}


	result = strncmp((const char *)input_attrib.joystick_name, (const char *)device->input_attrib.joystick_name, sizeof input_attrib.joystick_name);
	if(result != 0){
		joystick_device_close(device);
		return -1;	
	}

	if(input_attrib.joystick_axis_count != device->input_attrib.joystick_axis_count){
		joystick_device_close(device);
		return -1;	
	}

	if(input_attrib.joystick_button_count != device->input_attrib.joystick_button_count){
		joystick_device_close(device);
		return -1;	
	}


	return 1;
}


size_t joystick_device_identify_by_requirement(struct joystick_input_requirement *input_requirement, struct joystick_input_attrib *input_attrib, size_t input_attrib_max)
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

	
	DIR *dir = opendir(dev_path);

	if(dir == NULL){
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
	 *	Iterate over all filenames in the folder
	 *	and try to open them as joystick devices. 	
	 */
	while((dirent = readdir(dir)) != NULL)
	{

		if(dirent->d_type != DT_CHR){
			continue;
		}
		
		/* jsN for devices with joystick driver */	
		const char *joystick_device_name_ext = "js";
		if(strstr(dirent->d_name, joystick_device_name_ext) == 0)
		{
			continue;	
		}

		/* 
		 * Fill buffer with a complete path 
		 * d_name is null terminated. 
		 */

		char *filename = dirent->d_name;	
		size_t filename_length = strlen(filename);
		size_t filename_last_index = filename_length + dev_path_length;


		if(filename_last_index < PATH_MAX)
		{
			
			/* Note that full_file_path is pointer, */
			char *full_file_path_filename_offset = (char *)full_file_path + dev_path_length;
			memcpy(full_file_path_filename_offset, dirent->d_name, filename_length);
			
					
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
				close(joystick_device_fd);

				/* 
				 * If it matches the requirements 
				 */
				
				if(input_requirement->requirement_axis_count_min > curr_input_attrib.joystick_axis_count)
				{
					continue;	
				}

				if(input_requirement->requirement_axis_count_max < curr_input_attrib.joystick_axis_count)
				{
					continue;	
				}


				if(input_requirement->requirement_button_count_min > curr_input_attrib.joystick_button_count)
				{
					continue;	
				}

				if(input_requirement->requirement_button_count_max < curr_input_attrib.joystick_button_count)
				{
					continue;	
				}



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
	return joystick_device_found_index;
}

size_t joystick_device_identify(struct joystick_input_attrib *input_attrib, size_t input_attrib_max)
{

	struct joystick_input_requirement joystick_input_req = {
		.requirement_axis_count_min = 0,
		.requirement_button_count_min = 0,
		.requirement_axis_count_max = JOYSTICK_AXIS_MAX,
		.requirement_button_count_max = JOYSTICK_BUTTON_MAX,

	};

	return joystick_device_identify_by_requirement(&joystick_input_req, input_attrib, input_attrib_max);
}



int joystick_device_open(struct joystick_device *device, const char *device_path)
{
	assert(device != NULL);
	assert(device_path != NULL);
	
	/* 
	 * Open device and get attributes.
	 */

	device->device_fd = joystick_open(device_path, &device->input_attrib);
	
	if(device->device_fd < 0){
		return -1;	
	}

	memset(&device->input_value, 0, sizeof device->input_value);

	return 0;
}


