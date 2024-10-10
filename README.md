# FYP E13 Connetics Protection Relay Test Fixture

## Project status
prototype2.ino:
Code has largely been tested to work on a veroboard prototype. Pinmapping has since been changed for the PCB layout which is untested. UART comms is untested. 

## Description of each file
prototype2.ino:
This is the main file for the PCB which controls the user interface including the status buttons, LEDs, AC current source potentiomenter and 7 segment display. 
May be edited for reasons such as:
    - changing the spring charging delay
    - changing the frequency of communication to the AC current source MCU
    - if pushing a button changes the wrong LED
    - adding automatic behaviour to respond to button presses

