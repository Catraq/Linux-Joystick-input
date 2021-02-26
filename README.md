# PS3 Controller library

Library for reading joystick input and mapping the input to output suitable for the application. 

The input is fetched from the kernel joystick driver. The axis values are then mapped to outputs using a linear matrix, making mixing of the inputs straight forward.  
Can be used without linear map. 

See test/joystick.c for example.  

mkdir build
cd build
cmake ..
make 
./test /dev/input/js0 
