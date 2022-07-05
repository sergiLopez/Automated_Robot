/*
 * motor.h
 *
 *  Created on: 22 abr. 2022
 *      Author: Sergio
 */

#ifndef MOTOR_H_
#define MOTOR_H_
#include "uart.h"

void forward(unsigned int speed);
void move(byte ID, bool rotation, unsigned int speed);
void ledOn();
void wheelMode();
void turnLeft(unsigned int speed);
void turnRight(unsigned int speed);
void pivotRight(unsigned int speed);
void pivotLeft(unsigned int speed);
void turnSlightlyLeft(unsigned int speed);
void turnSlightlyRight(unsigned int speed);


#endif /* MOTOR_H_ */
