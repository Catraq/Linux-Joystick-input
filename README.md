# PS3 Controller library

Library for reading joystick input and mapping the input to output suitable for the application. 

The input is fetched from the kernel joystick driver. The axis values are then mapped to outputs using a linear matrix, making mixing of the inputs straight forward. There are support for picking a joystick input by requirements on number of axises and buttons.    

See test/test.c for example.  

## Quick start 
mkdir build
cd build
cmake ..
make 
./test
or 
./test /dev/input/js0
