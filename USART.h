#ifndef USART_H_
#define USART_H_


#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>


#define F_CPU ((uint32_t)16000000)
#define BAUD_RATE ((uint32_t)9600)
#define UART_BAUD_SELECT (uint8_t)(F_CPU/(BAUD_RATE << 4) - 1)
#define TAILLE_CHAINE 20
#define DEBUT_DEBUG 0xFE
#define FIN_DEBUG	0xFF
#define CST_ROT     0.03490658504 //=2.pi/180



void USART_Init(void);


#endif 
