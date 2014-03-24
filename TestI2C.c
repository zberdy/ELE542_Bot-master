/*
 * TestI2C.c
 *
 * Created: 20/03/2014 21:24:43
 *  Author: Hadrien
 */ 

#include "USART.h"
#include "i2c_caneva.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>

//Variables globales pour USART
volatile uint8_t debug=0;
char debugChaine[TAILLE_CHAINE];
volatile uint8_t etat = 1;
volatile float vitesseCommande;
volatile float angleCommande;

//Variables globales I2C
volatile uint8_t I2CLireTermine, I2CEcrireTermine;
uint8_t SonarEtat=0;
extern uint8_t CircularBufferOutIndex;

//Variable test
volatile uint8_t testI2C;
uint8_t MSB, LSB;
uint16_t distance;


int main(void)
{

USART_Init();
I2C_Init();
DDRB=0xFF;
PORTB=0x00;
sei();

twiWrite(0xE0, 0x02, 150);
twiWrite(0xE0, 0x01, 8);

twiWrite(0xE2, 0x02, 100);
twiWrite(0xE2, 0x01, 7);

    while(1)
    {
		
		_delay_ms(2000);
		twiWrite(0xE2, 0x00, 0x51);
		//PORTB=0xFF;
		_delay_ms(50);
		twiRead(0xE2, 0x02, &MSB);
		twiRead(0xE2, 0x03, &LSB);
		
		_delay_ms(50);
		distance=(MSB << 8)|(LSB);
		sprintf(debugChaine, "%d", distance);
		debug=1;
		UCSRB|=(1<<UDRIE);
		
		/*
		sprintf(debugChaine, "%d", testI2C);
		debug=1;
		UCSRB|=(1<<UDRIE);
		*/
	
		

    }
}
