/*
 * uart.c
 *
 *  Created on: 22 abr. 2022
 *      Author: Sergio
 */


#include "msp.h"
#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include"uart.h"
#include "lib_PAE.h"

uint32_t current_time = 0; // Variable que se usa en el Timer0


// Función time_out es True si el tiempo del contador (current_time) supera el tiempo que hemos pasado como parámetro (time)
bool time_out(int time){
    if(current_time>=time){
        return 1;
    }else{return 0;}
}


void init_interrupciones()
{


    //Int. port 4
    NVIC->ICPR[1] |= 1 << (PORT4_IRQn & 31); //M'asseguro que no queda cap interrupcio residual pendent per aquest port,
    NVIC->ISER[1] |= 1 << (PORT4_IRQn & 31); //i habilito les interrupcions del port

    //Int. port5
    NVIC->ICPR[1] |= 1 << (PORT5_IRQn & 31); //M'asseguro que no queda cap interrupcio residual pendent per aquest port,
    NVIC->ISER[1] |= 1 << (PORT5_IRQn & 31); //i habilito les interrupcions del port

    // Timer A1
    NVIC->ICPR[0] |= BITA;  //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[0] |= BITA;  //y habilito las interrupciones del puerto

    //Int. port 1 = 35 corresponde al bit 3 del segundo registro ISER1:
    NVIC->ICPR[1] |= 1 << (PORT1_IRQn & 31); //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= 1 << (PORT1_IRQn & 31); //y habilito las interrupciones del puerto

    //Int. EUSCIA2
    NVIC->ICPR[0] |= 1 << (EUSCIA2_IRQn);
    NVIC->ISER[0] |= 1 << (EUSCIA2_IRQn);

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.

}


void init_uart(void)
{
    UCA2CTLW0 |= UCSWRST;         //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK;   //UCSYNC=0 mode asíncron
                                  //UCMODEx=0 seleccionem mode UART
                                  //UCSPB=0 nomes 1 stop bit
                                  //UC7BIT=0 8 bits de dades
                                  //UCMSB=0 bit de menys pes primer
                                  //UCPAR=x ja que no es fa servir bit de paritat
                                  //UCPEN=0 sense bit de paritat
                                  //Triem SMCLK (24MHz) com a font del clock BRCLK
    UCA2MCTLW = UCOS16;       // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3;          //Prescaler de BRCLK fixat a 13. Com SMCLK va a24MHz,
                           //volem un baud rate de 115200kb/s i fem sobre-mostreig de 16
                           //el rellotge de la UART ha de ser de ~1.85MHz (24MHz/13).
     //UCBRSx, part fractional del baud rate
    //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3;          //I/O funció: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~(BIT2 | BIT3 );

    P3SEL0 &= ~BIT0;
    P3SEL1 &= ~BIT0;

    P3DIR |= BIT0;
    P3OUT &= ~BIT0;

    UCA2CTLW0 &= ~UCSWRST;          //Reactivem la línia de comunicacions sèrie
    //EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG; // Clear eUSCI RX interrupt flag
    EUSCI_A2->IE |= EUSCI_A_IE_RXIE; // Enable USCI_A0 RX interrupt, nomes quan tinguem la recepcio

    UCA2IE |= UCRXIE;

}


#define TXD2_READY (UCA2IFG & UCTXIFG)

/* funcions per canviar el sentit de les comunicacions */



/* funció TxUACx(byte): envia un byte de dades per la UART 0 */
void TxUAC2(uint8_t bTxdData)
{
    while (!TXD2_READY)
        ; // Espera a que estigui preparat el buffer de transmissió
    UCA2TXBUF = bTxdData;
}
void Sentit_Dades_Rx(void)
{ //Configuració del Half Duplex dels motors: Recepció
    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 (Rx)
}

void Sentit_Dades_Tx(void) //enviar
{ //Configuració del Half Duplex dels motors: Transmissió
    P3OUT |= BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)
}


byte TxPacket(byte bID, byte bParameterLength, byte bInstruction,
              byte Parametros[16])
{


    char error[] = "adr. no permitida";
    if ((Parametros[0] < 6) && (bInstruction == 3)){//si se intenta escribir en una direccion <= 0x05,
            //emitir mensaje de error de direccion prohibida:
            halLcdPrintLine(error, 8, INVERT_TEXT);
           //y salir de la funcion sin mas:
              return 0;
    }

    byte bCount, bCheckSum, bPacketLength;
    byte TxBuffer[32];
    Sentit_Dades_Tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)
    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID; //ID del mòdul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength + 2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction; //Instrucció que enviem al Mòdul

    for (bCount = 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d’enviar
    {
        TxBuffer[bCount + 5] = Parametros[bCount];
    }
    bCheckSum = 0;
    bPacketLength = bParameterLength + 4 + 2; //longitud del paquet total
    for (bCount = 2; bCount < bPacketLength - 1; bCount++) //Càlcul del checksum
    {
        bCheckSum += TxBuffer[bCount];
    }

    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)
    for (bCount = 0; bCount < bPacketLength; bCount++) //Aquest bucle és el que envia la trama al Mòdul Robot
    {
        TxUAC2(TxBuffer[bCount]);
    }
    while ((UCA2STATW & UCBUSY)); //Espera fins que s’ha transmès el últim byte
    Sentit_Dades_Rx(); //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta
    return (bPacketLength);
}

byte received_data;
byte read_data_UART;
struct RxReturn RxPacket(void)
{
    struct RxReturn respuesta;
    byte bCount, bLenght, bChecksum;
    int Rx_time_out = 0;

    //Ponemos la linea half duplex en Rx
    Sentit_Dades_Rx();
    //Activa_TimerA1_TimeOut();

    //es llegeixen els 4 primers parametres
    for (bCount = 0; bCount < 4; bCount++) //bRxPacketLength; bCount++)
    {
        //Reset_Timeout();
        current_time = 0;
        received_data = 0; //No_se_ha_recibido_Byte();
        while (!received_data) //Se_ha_recibido_Byte())
        {
            Rx_time_out = time_out(10); // tiempo en decenas de microsegundos (ara 10ms)
            if (Rx_time_out)
                break; //sale del while
        }
        if (Rx_time_out){
            break; //sale del for si ha habido Timeout
        }

//Si no, es que todo ha ido bien, y leemos un dato:
        respuesta.StatusPacket[bCount] = read_data_UART; //Get_Byte_Leido_UART();
    } //fin del for
    if (!Rx_time_out){
        bLenght = respuesta.StatusPacket[3] + 4;
    // Continua llegint la resta de bytes del Status Packet
        bChecksum = 0;
        for(bCount = 4; bCount < bLenght; bCount++){

            current_time = 0;
            received_data = 0; //No_se_ha_recibido_Byte();
            while (!received_data) //Se_ha_recibido_Byte())
            {
                Rx_time_out = time_out(10); // tiempo en decenas de microsegundos (ara 10ms)
                if (Rx_time_out)
                    break; //sale del while
            }
            if (Rx_time_out)
            {
                break; //sale del for si ha habido Timeout
            }
            respuesta.StatusPacket[bCount] = read_data_UART;
        }
        for(bCount = 0 ; bCount < bLenght-1 ; bCount++){
            bChecksum += respuesta.StatusPacket[bCount];
        }

        bChecksum = ~bChecksum; // Checksum a complement a 1

        // Comparem que el checkSum rebut amb el càlcul
        if(respuesta.StatusPacket[bLenght-1] != bChecksum){
            respuesta.error = true;
        }

    }
    return respuesta;

}
void EUSCIA2_IRQHandler(void)
{ //interrupcion de recepcion en la UART A0
    UCA2IE &= ~UCRXIE;//Interrupciones desactivadas en RX
    read_data_UART = UCA2RXBUF;
    received_data = 1;
    UCA2IE |= UCRXIE;//Interrupciones reactivadas en RX
}


void TA1_0_IRQHandler(void) {

    TA1CCTL0 &= ~CCIE;      // Deshabilitamos interrupciones mientras tratamos esta

    current_time++;         // Contador timeout

    TA1CCTL0 &= ~CCIFG;     // Quitamos flag de interrupción
    TA1CCTL0 |= CCIE;       // Habilitamos interrupciones

}












