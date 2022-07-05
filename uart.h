/*
 * uart.h
 *
 *  Created on: 22 abr. 2022
 *      Author: Sergio
 */

#ifndef UART_H_
#define UART_H_

typedef uint8_t byte;
void init_interrupciones();
void init_uart(void);
void Sentit_Dades_Rx(void);
void Sentit_Dades_Tx(void);
void TxUAC0(uint8_t bTxdData);
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction,
              byte Parametros[16]);
struct RxReturn RxPacket(void);

typedef struct RxReturn
{
    byte bCount, bLenght, bChecksum, received_data;
    byte Rx_time_out;
    byte StatusPacket[32];
    bool error;

} RxReturn;



#endif /* UART_H_ */
