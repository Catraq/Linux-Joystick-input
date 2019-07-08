# PS3 Controller library

Library for reading most recent PS3 controller data from the driver, keeps an buffer updated with the most recent values.

Requires pthread and sixaxis.

Quick start.

    #include "joystick_ps3.h"

    ....
    struct joystick_ps3_context ps3;
    result = joystick_ps3_intialize(&ps3, device_path);
    if(result < 0) /* Error */
    ....

    ....
    struct timeval time;
    gettimeofday(&time, NULL);

    /* 1 ms timeout */
    struct timespec timeout =
    {
    	.tv_sec = time.tv_sec,
    	.tv_nsec = (long)time.tv_usec * 1000 + (long)1000*1000
    };

    struct joystick_ps3 input;
    result = joystick_ps3_input(&ps3, &input, &timeout);
    if(result < 0) /* Error */
    ....

    /* Something something.. */
    uint8_t ... = input.button[JOYSTICK_PS3_BUTTON_CIRCLE]
    int32_t ... = input.axis[JOYSTICK_PS3_AXIS_LEFT_X]


See test/test.c for example.  

	mkdir build
	cd build
	cmake ..
	make 
	./test /dev/input/js0 
