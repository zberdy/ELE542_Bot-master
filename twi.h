/*
 * twi.h
 *
 * Created: 20/03/2014 22:38:47
 *  Author: Hadrien
 */ 


#ifndef TWI_H_
#define TWI_H_

#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//Etat de registre I2C
#define I2C_START      0x08
#define I2C_RESTART	   0x10
#define I2C_ADRW_ACK   0x18
#define I2C_DATA_ACK   0x28
#define I2C_ADRR_ACK   0x40

//Fonction
void TWI_Init(void);
void I2C_Commande (uint8_t ADRI2C, uint8_t registreI2C, uint8_t dataI2C);
void I2C_Mesure (uint8_t ADRI2C);

#endif /* TWI_H_ */
