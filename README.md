# PS3 Controller library

Library for reading most recent PS3 controller data from the driver, keeps an buffer updated with the most recent values.

Requires pthread and sixaxis.

Quick start.

    #include "joystick_ps3.h"

    ....
    int device_have_to_exist = 1;
    struct joystick_ps3_context ps3;
    result = joystick_ps3_intialize(&ps3, device_path, device_have_to_exist);
    if(result < 0) /* Error */
    ....

    ....
    const uint32_t usec_timeout = 10;
    struct joystick_ps3 input;
    result = joystick_ps3_input(&ps3, &input, usec_timeout);
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
