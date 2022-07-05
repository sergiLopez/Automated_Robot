/*
 * sensor.c
 *
 *  Created on: 5 may. 2022
 *      Author: Sergio
 */


/*
 * sensor.c
 *
 *  Created on: 5 may. 2022
 *      Author: Sergio
 */



#include "msp.h"
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include"uart.h"
#include "sensor.h"
#include "lib_PAE.h"


struct sensors readIRSensors() {
    byte bID = 100;
    byte bInstruction = 0x02;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = 0x1A;
    bParameters[1] = 3;

    TxPacket(bID, bParameterLength, bInstruction, bParameters);

    struct RxReturn returnPacket = RxPacket();

    struct sensors readSensors;
    readSensors.left = returnPacket.StatusPacket[5];
    readSensors.front = returnPacket.StatusPacket[6];
    readSensors.right = returnPacket.StatusPacket[7];

    return readSensors;
}

// Escollim quin dels tres sensors volem llegir
int readSensor(byte ID, byte sensor)
{

    struct RxReturn returnPacket;

    byte bInstruction = 0x02;
    byte bParameterLength = 2;
    byte bParameters[16];
    bParameters[0] = sensor;
    bParameters[1] = 1;

    TxPacket(ID, bParameterLength, bInstruction, bParameters);
    returnPacket = RxPacket();

    return returnPacket.StatusPacket[5];
}


uint8_t front_s() {
    return readSensor(100, 0X1B);
}

uint8_t left_s() {
    return readSensor(100, 0x1A);
}

uint8_t right_s() {
    return readSensor(100, 0x1C);
}
