//code specific for driving the Winter Camp mini Rover hardware
#ifndef ROVER_H
#define ROVER_H

#include <Arduino.h>
#include <Servo.h>

#define motorAFwd 2
#define motorARev 14
#define motorBFwd 12
#define motorBRev 15
#define servoPin 13
#define whiteLedPin 4

void roverInit(); //sets up the hardware
void roverDrive(uint8_t); //drives the drive motors - odd bits are motor power, even bits are direction
void roverClaw(uint8_t); //drives servo attached to claw
void flashlight(bool); 

#endif