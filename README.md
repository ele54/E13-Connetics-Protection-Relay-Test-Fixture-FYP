# FYP E13 Connetics Protection Relay Test Fixture

## Project status
**prototype2.ino:**

Code has largely been tested to work on a veroboard prototype. Pinmapping has since been changed for the PCB layout which is untested. UART comms section is untested. 

## Description of each file
**prototype2.ino:**

This is the main file for the PCB which controls the user interface including the status buttons, LEDs, AC current source potentiometer and 7 segment display. 
May be edited for reasons such as:
- changing the spring charging delay
- changing the frequency of communication to the AC current source MCU
- if pushing a button changes the wrong LED
- adding automatic behaviour to respond to button presses

**AnalogButtonArray.ino:**
Should be unneeded. Was used in earlier prototyping stages but may be modified to debug buttons if needed.

**Calibration.ino:**
Should be unneeded. Has been updated to read pins connected to buttons on PCB but has been unused/tested. May be used to recalibrate or debug buttons.

**displayAndDial.ino:**
Should be unneeded. Has been updated to read pins connected to the AC current source user interface on PCB but has been unused/untested. May be used for debugging. 

**ac_current_source_spwm**
The is the code used on the AC current souce in the present implementation (22/10/24). It uses SPWM and a low-pass filter to generate a sinusoidal voltage instead of a DAC.
It is presently written with testing in mind. This code is suppose to be ran with another Arduino runing sender_test, which this code sends the 10-bit DAC code using UART.

More details about its functionality and the connections required can be found in the OneDrive handover document.

**ac_current_source_dac**
An alternative version that uses the DAC instead of SPWM. Currently untested.

**sender_test**
This file is contained in the ac_current_source_spwm folder. Another Arduino can run this software to send ADC codes to the current source board to change its output.
