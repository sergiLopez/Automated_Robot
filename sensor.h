/*
 * sensor.h
 *
 *  Created on: 5 may. 2022
 *      Author: Sergio
 */

#ifndef SENSOR_H_
#define SENSOR_H_

struct sensors {
    byte left;
    byte front;
    byte right;
};

struct sensors readIRSensors();

int readSensor(byte ID, byte sensor);

uint8_t front_s();
uint8_t left_s();
uint8_t right_s();
void stop();


#endif /* SENSOR_H_ */
