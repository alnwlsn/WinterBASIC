#include "rover.h"

Servo servo1;

void roverInit() {
    pinMode(whiteLedPin, OUTPUT);
    pinMode(motorAFwd, OUTPUT);
    digitalWrite(motorAFwd, 0);
    pinMode(motorARev, OUTPUT);
    digitalWrite(motorARev, 0);
    pinMode(motorBFwd, OUTPUT);
    digitalWrite(motorBFwd, 0);
    pinMode(motorBRev, OUTPUT);
    digitalWrite(motorBRev, 0);
    servo1.attach(
        servoPin,
        14,
        0,
        180);
    servo1.write(0);
}
void roverDrive(uint8_t drive) {   //drives the drive motors - odd bits are motor power, even bits are direction
    if (drive & 0b00000001) {      //right motor
        if (drive & 0b00000010) {  //reverse
            digitalWrite(motorBFwd, 0);
            digitalWrite(motorBRev, 1);
        } else {  //forwards
            digitalWrite(motorBFwd, 1);
            digitalWrite(motorBRev, 0);
        }
    } else {  //motor off
        digitalWrite(motorBFwd, 0);
        digitalWrite(motorBRev, 0);
    }
    if (drive & 0b00000100) {      //left motor
        if (drive & 0b00001000) {  //reverse
            digitalWrite(motorAFwd, 0);
            digitalWrite(motorARev, 1);
        } else {  //forwards
            digitalWrite(motorAFwd, 1);
            digitalWrite(motorARev, 0);
        }
    } else {  //motor off
        digitalWrite(motorAFwd, 0);
        digitalWrite(motorARev, 0);
    }
}
void roverClaw(uint8_t v){
    servo1.write(v);
}
void flashlight(bool v){
    digitalWrite(whiteLedPin, v);
}