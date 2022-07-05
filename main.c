#include "msp.h"

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "uart.h"
#include "motor.h"
#include "sensor.h"
#include "lib_PAE.h"

#define TOO_CLOSE 80
#define NEAR 50
#define FAR 40
#define LEFT_WALL 1
#define RIGHT_WALL 2
#define VELOCITY 600

uint8_t followingWall = 0;
bool hasFoundFrontWall = false;


void init_timers(void){


    // Timer a 1ms para el timeout
    TA1CTL |= TASSEL_1 + MC_1 + TACLR;
    TA1CCTL0 &= ~(CAP+CCIFG);
    TA1CCTL0 |= CCIE;
    TA1CCR0 = 33;

}

uint8_t nextStateFromFree(uint8_t left, uint8_t front, uint8_t right)
{
    if(front > NEAR) {
        // Hi ha una paret davant del robot
        if(right > NEAR) {
            // També hi ha una paret a la dreta del robot
            return 4; // Girar a l'esquerra
        } else {
            return 3; // Girar a la dreta
        }
    }

    if(right > NEAR) {
        // Hi ha una paret a prop a la dreta
        followingWall = RIGHT_WALL;
        return 1; // Seguir paret dreta
    } else if (left > NEAR) {
        followingWall = LEFT_WALL;
        // Hi ha una paret a prop a l'esquerra
        return 2; // Seguir paret esquerra
    }

    return 0;
}

uint8_t nextFromRightWall(uint8_t left, uint8_t front, uint8_t right)
{
    if(front > NEAR) {
        // Detecta una paret davant
        return 4; // Girar a l'esquerra
    }
    if(right < FAR) {
        // No detecta paret a la dreta
        return 3; // Girar a la dreta
    }
    if(right > NEAR && left > NEAR) {
        // Detecta parets a la dreta i a l'esquerra
        return 5; // El robot quedaria atrapat
    }
    return 1; // Continuar seguint paret
}

uint8_t nextFromLeftWall(uint8_t left, uint8_t front, uint8_t right)
{
    if(front > NEAR) {
        // Detecta una paret davant
        return 3; // Girar a la dreta
    }
    if(left < FAR) {
        // No detecta paret a l'esquerra
        return 4; // Girar a l'esquerra
    }
    if(right > NEAR && left > NEAR) {
        // Detecta parets a la dreta i a l'esquerra
        return 5; // El robot quedaria atrapat
    }
    return 2; // Continuar seguint paret
}

uint8_t nextFromTurningRight(uint8_t left, uint8_t front, uint8_t right)
{
    if(left > NEAR && front < FAR) {
        // Detecta una paret a l'esquerra
        followingWall = LEFT_WALL;
        return 2; // Seguir paret esquerra
    }
    if(right > NEAR && front < FAR) {
        // Detecta una paret a la dreta
        followingWall = RIGHT_WALL;
        return 1; // Seguir paret dreta
    }
    return 3; // Continuar girant
}

uint8_t nextFromTurningLeft(uint8_t left, uint8_t front, uint8_t right)
{
    if(right > NEAR && front < FAR) {
        // Detecta una paret a la dreta
        followingWall = RIGHT_WALL;
        return 1; // Seguir paret dreta
    }
    if(left > NEAR && front < FAR) {
        // Detecta una paret a l'esquerra
        followingWall = LEFT_WALL;
        return 2; // Seguir paret esquerra
    }
    return 4; // Continuar girant
}

uint8_t nextFromTurningBack(uint8_t left, uint8_t front, uint8_t right)
{
    hasFoundFrontWall |= front > NEAR;

    if(hasFoundFrontWall) {
        if(front < FAR) {
            hasFoundFrontWall = false;
            if(followingWall == RIGHT_WALL) {
                // follow right wall
                return 1;
            } else {
                // follow left wall
                return 2;
            }
        }
    }
    // keep turning back
    return 5;
}

void robotFree()
{
    forward(VELOCITY);
}

void find_wall()
{
    struct sensors sensorsIR;
    uint8_t right, left, front;
    uint8_t dist;
    sensorsIR = readIRSensors();
    right = sensorsIR.right;
    left = sensorsIR.left;
    front = sensorsIR.front;

    if (front < left) {
        // Llegeix sensor esquerra amb prioritat
        if (right < left) {
            // rota fins tenir una paret al davant
            pivotLeft(VELOCITY);
            dist = 0;
            while(left > dist) {
                sensorsIR = readIRSensors();
                dist = sensorsIR.front;
                left = sensorsIR.left;
            }

        } else { // cas paret a la dreta
            pivotRight(VELOCITY);
            dist = 0;
            while(right > dist) {
                sensorsIR = readIRSensors();
                dist = sensorsIR.front;
                right = sensorsIR.right;
            }
        }
    }
    // Avanca cap a la paret
    forward(VELOCITY);
    dist = 0;
    while (dist < NEAR){
        dist = front_s();
    }
    sensorsIR = readIRSensors();
    right = sensorsIR.right;
    left = sensorsIR.left;
    if(right < left){
        // Girar a la dreta
        turnRight(VELOCITY);
        dist = 0;
        while (dist < NEAR ) { // Situa paret a lesquerra i avanca
            dist = left_s();
        }
    } else {
        // Girar a l'esquerra
        turnLeft(VELOCITY);
        dist = 0;
        while(dist < NEAR) {
            dist = right_s();
        }
    }
    forward(VELOCITY);
}


void followRightWall(uint8_t right)
{
    followingWall = 1;
    // min dist = 120, max dist = 100;
    if(right < NEAR) {
        // el robot és massa lluny de la paret. S'ha d'apropar
        //turnSlightlyRight(VELOCITY);
        turnRight(VELOCITY);
    } else if(right > TOO_CLOSE) {
        // el robot és massa aprop de la paret. S'ha d'allunyar
        //turnSlightlyLeft(VELOCITY);
        turnLeft(VELOCITY);
    } else {
        // el robot està a una distància adequada de la paret
        forward(VELOCITY);
    }
}

void followLeftWall(uint8_t left)
{
    followingWall = 2;
    if(left < NEAR) {
        // el robot és massa lluny de la paret. S'ha d'apropar
        //turnSlightlyLeft(VELOCITY);
        turnLeft(VELOCITY);
    } else if(left > TOO_CLOSE) {
        // el robot és massa aprop de la paret. S'ha d'allunyar
        //turnSlightlyRight(VELOCITY);
        turnRight(VELOCITY);
    } else {
        // el robot està a una distància adequada de la paret
        forward(VELOCITY);
    }
}

void robotTurnRight(uint8_t wall)
{
    if(wall < FAR) {
        // No hi ha cap paret davant del robot
        turnRight(VELOCITY);
    } else {
        // Hi ha una paret davant del robot
        pivotRight(VELOCITY);
    }
}

void robotTurnLeft(uint8_t wall)
{
    if(wall < FAR) {
        // No hi ha cap paret davant del robot
        turnLeft(VELOCITY);
    } else {
        // Hi ha una paret davant del robot
        pivotLeft(VELOCITY);
    }
}

void robotTurnBack(void)
{
    if(followingWall == 1) { // seguint la paret dreta
        pivotLeft(VELOCITY);
    } else {                // seguint la paret esquerra
        pivotRight(VELOCITY);
    }
}


void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	init_ucs_24MHz();
	init_uart();
	init_interrupciones();
	init_timers();
	halLcdInit();
	halLcdClearScreen(0);
	wheelMode();


	uint8_t front, left, right;
	uint8_t state = 0;

	readIRSensors();


	while (true){

        // Moviment segons estat
        switch(state) {
            case 0:
                // No detecta cap obstacle
                find_wall();
                break;
            case 1:
                // seguir paret dreta
                followRightWall(right);
                break;
            case 2:
                // seguir paret esquerra
                followLeftWall(left);
                break;
            case 3:
                // girant a la dreta
                robotTurnRight(front);
                break;
            case 4:
                // girant a l'esquerra
                robotTurnLeft(front);
                break;
            case 5:
                // robot atrapat
                robotTurnBack();
            default:
                stop();
                break;
        }

        struct sensors IRSensors = readIRSensors();
        right = IRSensors.right;
        front = IRSensors.front;
        left = IRSensors.left;

        // Escollir el proper estat
        switch(state) {
            case 0:
                // No detecta cap obstacle
                state = nextStateFromFree(left, front, right);
                break;
            case 1:
                // seguir paret dreta
                state = nextFromRightWall(left, front, right);
                break;
            case 2:
                // seguir paret esquerra
                state = nextFromLeftWall(left, front, right);
                break;
            case 3:
                // girant a la dreta
                state = nextFromTurningRight(left, front, right);
                break;
            case 4:
                // girant a l'esquerra
                state = nextFromTurningLeft(left, front, right);
                break;
            case 5:
                state = nextFromTurningBack(left, front, right);
            default:
                stop();
                break;
        }

    }




}
