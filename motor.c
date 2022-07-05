/*
 * motor.c
 *
 *  Created on: 22 abr. 2022
 *      Author: Sergio
 */


#include "msp.h"
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include"motor.h"
#include"uart.h"
#include "lib_PAE.h"



#define RIGHT_WHEEL 2
#define LEFT_WHEEL 3
#define MAX_SPEED 1023//0x3FF

#define LEFT 0
#define RIGHT 1
#define CW_ADRESS 0x06
#define CCW_ADRESS 0x08
#define DIRECTION_ADRESS 0x21

#define WRITE_DATA 0x03;
#define REG_WRITE 0x04;
#define ACTION 0x05;
#define BROADCASTING 0xFE;

void wheelMode(){

    byte bID = BROADCASTING;
    byte bInstruction = WRITE_DATA;
    byte bparameterLength = 5;
    byte bParameters[16];

    bParameters[0] = CW_ADRESS;

    bParameters[1] = 0;
    bParameters[2] = 0;
    bParameters[3] = 0;
    bParameters[4] = 0;

    TxPacket(bID, bparameterLength, bInstruction, bParameters);


}

void ledOn(){

    byte bParameterLength = 3;
    byte bInstruction = 0x05;
    byte bParameters[16];

    bParameters[0] = 0x18;
    bParameters[1] = 0x00;
    bParameters[2] = 0x00;

    TxPacket(1, bParameterLength, bInstruction, bParameters);
    RxPacket();

}


void move(byte ID, bool rotation, unsigned int speed)
{
    byte speed_H,speed_L;
    speed_L = speed;

    if(speed<1024){ // Velocidad maxima 1023

        if(rotation){ // Rotation == 1
            speed_H = (speed >> 8)+4;   // Mover a la derecha
        }else{
            speed_H = speed >> 8;       // Mover a la izquierda
        }

        byte bInstruction = REG_WRITE;
        byte bParameterLength = 3;
        byte bParameters[16];

        // Empezamos por la dirección 0x20
        bParameters[0] = 0x20;

        // Escribimos la velocidad y la dirección
        bParameters[1] = speed_L;
        bParameters[2] = speed_H;

        TxPacket(ID, bParameterLength, bInstruction, bParameters);
        RxPacket();

    }
}

void action() {
    byte ID = BROADCASTING;
    byte bInstruction = ACTION;
    byte bParameterLength = 0;
    byte bParameters[16];


    TxPacket(ID, bParameterLength, bInstruction, bParameters);
}


void forward(unsigned int speed)
{
    //Move forward mateixa velovitat en les dues rodes
    if (speed < MAX_SPEED)
    {
        move(LEFT_WHEEL, LEFT, speed);
        move(RIGHT_WHEEL, RIGHT, speed);
        action();

    }
}


void turnLeft(unsigned int speed)
{
    // Pivot left
    if (speed < MAX_SPEED)
    {
        // Left wheel velocitat a 0
        move(RIGHT_WHEEL, RIGHT, speed);
        move(LEFT_WHEEL, LEFT, 0);
        action();

    }
}
void turnRight(unsigned int speed)
{
   // Pivot Right

    if(speed < MAX_SPEED){
        // Right wheel velocitat a 0
        move(RIGHT_WHEEL, RIGHT, 0);
        move(LEFT_WHEEL, LEFT, speed);
        action();


    }
}

void stop(){
    move(RIGHT_WHEEL, RIGHT, 0);
    move(LEFT_WHEEL, LEFT, 0);
    action();

}


void pivotRight(unsigned int speed)
{
   // Girar a la derecha
    if(speed < 1024){
        move(RIGHT_WHEEL, LEFT, speed);
        move(LEFT_WHEEL, LEFT, speed);
        action();

    }

}

void pivotLeft(unsigned int speed)
{
    // Girar a la izquierda
    if(speed < 1024){
        move(RIGHT_WHEEL, RIGHT, speed);
        move(LEFT_WHEEL, RIGHT, speed);
        action();

    }
}

void turnSlightlyRight(unsigned int speed)
{
    int speed2 = speed * 0.95;
    if(speed < MAX_SPEED){
        move(RIGHT_WHEEL, RIGHT, speed2);
        move(LEFT_WHEEL, LEFT, 800);
        action();

    }
}

void turnSlightlyLeft(unsigned int speed)
{
    int speed2 = speed * 0.95;
   // Pivot Right
    if(speed < MAX_SPEED){
        // Right wheel velocitat a 0.9 times speed
        move(RIGHT_WHEEL, RIGHT, speed);
        move(LEFT_WHEEL, LEFT, speed2);
        action();

    }
}






