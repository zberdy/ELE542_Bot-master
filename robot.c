/*
 * robot.c
 *
 * Created: 2014-01-23 14:03:21
 *  Author: KOLY-453
 */ 

#include "USART.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>
//#include <util/delay.h>
#include "pwm.h"
#include "ADC.h"
#include "moteur.h"

//Variable global pour USART
volatile uint8_t debug=0;
char debugChaine[TAILLE_CHAINE];
volatile uint8_t etat = 1;


//Variable de Temps
volatile uint8_t flag5ms=0;

//Variable de calcul de la vitesse des moteurs
volatile int16_t somme_vitesse[2] = {0, 0};
volatile uint8_t nombre_echantillon[2] = {0, 0};
float vitesseMoteurCorr[2]={0.0,0.0};
float vitesseMoteur[2]={0.0,0.0};
float dutyCycleG=0.0, dutyCycleD=0.0;


//Variable de calibration des moteurs
uint16_t vMaxPlus[2]={0,0}, vZeroPlus[2]={6,6}, vMaxMoins[2]={0,0}, vZeroMoins[2]={5,5};
float aPlus[]={1.0,1.0}, aMoins[]={1.0,1.0};

//Variable de commande du Robot
volatile float vitesseCommande=0, angleCommande=0;

//Variable de Test à suprimmer
uint8_t test1=0;
volatile uint16_t test;
//
////Variables globales I2C
//volatile uint8_t I2CLireTermine, I2CEcrireTermine;
//uint8_t SonarEtat=0;
//extern uint8_t CircularBufferOutIndex;
//
////Variable test
//volatile uint8_t testI2C;
//uint8_t MSB, LSB;
//uint16_t distance;
//


int main(void)
{
	sei();   
	DDRB=0xFF;
	PORTB=0x00;
	USART_Init();
	
	ADC_Init();
	PWM_Init();
	PWM_calibrer();
	flag5ms=0;


	PORTD=0b01100100;
	//PORTD=0b10101000;
	//PORTA|=0b00001000;
	//PIND=0b01100100;
	//OCR1A=2000;
	//OCR1B=0;
	//PORTD=0b10000100;

	while(1)
    {
	
		//Reception de la commande
		
		if (etat == 0)
		{

			etat = 1;
			vitesseCommande = (vitesseCommande - (float)100)*0.01;
			angleCommande *= CST_ROT;
		}

	
		if (flag5ms) //Toutes les 5ms on déclanche le calcul du PWM
		{

			flag5ms = 0;
			
			// On calcul la moyenne de la vitesse non corrigé des 5ms prec
			moyenne_Moteur(vitesseMoteur); //Attention vitesse moteur est encore une valeure entre 0 et 1023					

				//Moteur Droit
			if (vitesseMoteur[DROIT] > 0.0)//Si le moteur tourne en marche avant
			{
				vitesseMoteurCorr[DROIT]=vitesseMoteur[DROIT]*aPlus[DROIT];
			}
			else
			{
				vitesseMoteurCorr[DROIT]=vitesseMoteur[DROIT]*aMoins[DROIT];
			}
			
			
			//Moteur gauche
			if (vitesseMoteur[GAUCHE] > 0.0)//Si le moteur tourne en marche avant
			{
				vitesseMoteurCorr[GAUCHE]=vitesseMoteur[GAUCHE]*aPlus[GAUCHE];		
			}
			else
			{
				vitesseMoteurCorr[GAUCHE]=vitesseMoteur[GAUCHE]*aMoins[GAUCHE];
			}			
			

			//Calcul du duty cycle 
			/*
			sprintf(debugChaine, "%d  %d", (int)(vitesseCommande*1000),(int)(angleCommande*1000));
			debug=1;
			UCSRB|=(1<<UDRIE);
			*/

			CalculPWM(vitesseCommande, angleCommande, vitesseMoteurCorr[GAUCHE], vitesseMoteurCorr[DROIT], &dutyCycleG, &dutyCycleD);
			
			ordre_Moteur2(dutyCycleG, dutyCycleD);

			/*sprintf(debugChaine, "%d", OCR1B);
			debug=1;
			UCSRB|=(1<<UDRIE);

			*/
		}

		
	}
			
}		
		
		
