# binary-converter
This device converts an 8-bit binary number to decimal and hexadecimal. 

Each bit of the binary number is represented by one of the 
momentary switches. Activating the switches toggles the corresponding bit, 
and the binary number and its hexadecimal and decimal representation are 
displayed on the LCD. 

This project was written in C, without using any arduino libraries, and compiled with 
avr-gcc.

The wiring for the switch is pretty straightforward, simply connect each switch to a pin on the arduino. 
To make the source code compatible with your wiring setup, change the function 
`querySwitches()` so that the input from the switches is read correctly. 

The wiring for the LCD is exactly like in [this tutorial](https://www.arduino.cc/en/Tutorial/HelloWorld)

To compile the code and upload it to your arduino: 
```
mkdir build 
cd build 
cmake .. 
make upload
```

You might have to change the `PORT` variable in your CMakeLists.txt so that avrdude uses the correct port. 