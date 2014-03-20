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
#include <util/delay.h>
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
float vitesseMoteur[2]={0,0};
float dutyCycle[2]={0.0,0.0};


//Variable de calibration des moteurs
uint16_t vMaxPlus[2]={0,0}, vZeroPlus[2]={6,6}, vMaxMoins[2]={0,0}, vZeroMoins[2]={5,5};
float aPlus[]={1,1}, bPlus[]={0,0}, aMoins[]={1,1}, bMoins[]={0,0};
//Variable de commande du Robot
volatile float vitesseCommande=0, angleCommande=0;

//Variable de Test à suprimmer
uint8_t test1=0;
volatile uint16_t test;

//static volatile int8_t compteur1;
//volatile int8_t calibration_rdy=0;
//volatile uint8_t calibration_req=1;
Ordre ordreG;
Ordre ordreD;
int main(void)
{
	sei();   
	DDRB=0xFF;
	PORTB=0x00;
	USART_Init();
	
	ADC_Init();
	PWM_Init();
	PWM_calibrer();
	PORTB^=0b00010000;
	flag5ms=0;
	PORTD=0b01100100;
	//PORTD=0b10101000;
	//PORTA|=0b00001000;
	//PIND=0b01100100;
	OCR1A=2000;
	OCR1B=2000;

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
			//Test de diode clignotant toute les 500ms (à supprimer)
			test1++;
			if (test1>100)
			{
				test1=0;
				PORTB^=0b10000000;
			}
			flag5ms=0;
			
			// On calcul la moyenne de la vitesse non corrigé des 5ms prec
			moyenne_Moteur(vitesseMoteur); //Attention vitesse moteur est encore une valeure entre 0 et 1023		
			
			//Moteur gauche
			if (vitesseMoteur[GAUCHE] > 0.0)//Si le moteur tourne en marche avant
			{
				vitesseMoteurCorr[GAUCHE]=(vitesseMoteur[GAUCHE]*aPlus[GAUCHE])/1024.0;		
			}
			else
			{
				vitesseMoteurCorr[GAUCHE]=(vitesseMoteur[GAUCHE]*aMoins[GAUCHE])/1024.0;
			}			
			
			//Moteur Droit
			if (vitesseMoteur[DROIT] > 0.0)//Si le moteur tourne en marche avant
			{
				vitesseMoteurCorr[DROIT]=(vitesseMoteur[DROIT]*aPlus[DROIT])/1024.0;
			}
			else
			{
				vitesseMoteurCorr[DROIT]=(vitesseMoteur[DROIT]*aMoins[DROIT])/1024.0;
			}
			

			//Calcul du duty cycle 

			CalculPWM(vitesseCommande, angleCommande, vitesseMoteurCorr[GAUCHE], vitesseMoteurCorr[DROIT], &dutyCycle[GAUCHE], &dutyCycle[DROIT]);

			if (dutyCycle[GAUCHE] == 0.0)
			{
				OCR1A = 0;
				ordreG = frein;
			}
			else if (dutyCycle[GAUCHE] < 0.0)
			{
				ordreG = arriere;
				OCR1A = (uint16_t)((float)ICR1 * -dutyCycle[GAUCHE]);
			}
			else if (dutyCycle[GAUCHE] > 0.0)
			{
				ordreG = avant;
				OCR1A = (uint16_t)((float)ICR1 * dutyCycle[GAUCHE]);
			}
			else
			{
				ordreG = neutre;

			}
		
			if (dutyCycle[DROIT] == 0.0)
			{
				OCR1A = 0;
				ordreD = frein;
			}
			else if (dutyCycle[DROIT] < 0.0)
			{
				ordreD = arriere;
				OCR1A = (uint16_t)((float)ICR1 * -dutyCycle[GAUCHE]);
			}
			else if (dutyCycle[DROIT] > 0.0)
			{
				ordreD = avant;
				OCR1A = (uint16_t)((float)ICR1 * dutyCycle[GAUCHE]);
			}
			else
			{
				ordreD = neutre;

		    } 
		
			ordre_Moteur(ordreG, ordreD);

		}

		
	}
			
}		
		
		
