# PULSE @ Electromagnetci Field 2016

Pulse is an interactive light and sound installation at Electromagnetic Field 2016. In a Guitar Hero style, look stupid as you jump around hitting buttons to music as pretty lights stream down the 8-metre poles in front of you.

The system runs on a series of microcontrollers; STM32L0s controlling the LED poles, and ATMEGAs controlling the button podiums and scoreboard. These are all linked to a controlling computer via RS422 differential signals, and the controlling program is running in python.