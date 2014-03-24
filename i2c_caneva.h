/******************************************************************************
 *
 *
 *****************************************************************************/


#ifndef I2C_H
#define I2C_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
#define F_CPU ((uint32_t)16000000)
#include <util/delay.h>

#define CIRCULAR_BUFFER_SIZE 20
/** Macro*/


/** Prototype */

void I2C_Init(void);

void putDataOutBuf(uint8_t data);
uint8_t getDataOutBuf(void);
void putDataInBuf(uint8_t *ptr);
uint8_t *getDataInBuf(void);

void twiWrite(uint8_t address, uint8_t registre, uint8_t data);
void twiRead(uint8_t address, uint8_t registre, uint8_t *ptr);

/** Les variables */
uint8_t CircularBufferOut[CIRCULAR_BUFFER_SIZE];
uint8_t *CircularBufferIn[CIRCULAR_BUFFER_SIZE];

uint8_t CircularBufferOutEnd;
uint8_t CircularBufferOutIndex;
uint8_t CircularBufferInEnd;
uint8_t CircularBufferInIndex;

volatile uint8_t I2CReStart;

extern volatile uint8_t I2CLireTermine;
extern volatile uint8_t I2CEcrireTermine;
extern volatile uint8_t debug;
extern char debugChaine[];


extern volatile uint8_t testI2C;

#endif
