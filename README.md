# DC Motor Position Control

This project aims to implement Proportional-Integral-Derivative (PID) controller for position control of a DC Motor. A feedback encoder either optical or magnetic is assumed to be attached with the shaft of the Motor. The encoder pulses are fed to the main microcontroller that formulate the error with reference to the current and the desired position (pulses) of the motor and computes the Pulse Width data output using PID controller. This pulse width data is then fed to the slave controller that generates a wave of 2KHz with the pulse width computed and fed from the main controller. This is how the motor speed is controlled until the desired position is smoothly achieved. 

This is in practice a modified PID where Proportional, Integral and Derivative constants are tuned such that there is no over-shoot case in the motor output. Further suggestions and improvements are welcomed.

Link for youtube video is as follows:

https://www.youtube.com/watch?v=0d06akGY4kw