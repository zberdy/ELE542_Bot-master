/*
 * ADC.c
 *
 * Created: 2014-01-29 10:53:20
 *  Author: KOLY-453
 */ 

#include "ADC.h"
extern volatile int16_t somme_vitesse[];
extern volatile uint8_t nombre_echantillon[];
volatile uint16_t ADMUX_prev=0;

//Variables USART
extern char debugChaine[TAILLE_CHAINE];
extern uint8_t debug;

//Variable de test
extern volatile uint16_t test;
	
void ADC_Init(void) 
{
	//- Fréquence de l’horloge de l’ADC = 125 kHz
	//- Mode d’opération = Freerun
	//- Référence de voltage = AREF
	//- Mode de traitement des acquisitions = Interruption
	ADMUX	= (0 << REFS1) | (0 << REFS0) | (0 << ADLAR) | (0 << MUX4) | (0 << MUX3) | (0 << MUX2)	| (0 << MUX1) | (0 << MUX0);
	ADCSRA	= (1 << ADEN) | (0 << ADSC) | (1 << ADATE) | (0 << ADIF) | (1 << ADIE) |  (1 << ADPS2)	|  (1 << ADPS1)	 |  (1 << ADPS0); 
	SFIOR	= (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0) | (0 << 4)	   |  (0 << ACME) | (0 << PUD)	| (0 << PSR2)	 | (0 << PSR10);
	ADCSRA |= (1 << ADSC); //On lance l'aquisition en mode free running
	ADMUX  = (1 << MUX0); //On change immédiatement de canal pour la prochaine conversion
	DDRA=(1<<CAL);
//	PORTA = (1 << DIR_G) |(1 << DIR_D);
}


ISR(ADC_vect)
{
	int16_t vitesse_echantillon;
	ADMUX_prev=ADMUX;
	ADMUX ^= (1  << MUX0); // le changement de canal débute avant la conversion
	vitesse_echantillon=ADC;
	//vitesse_echantillon = ADCL;
	//PORTB = ADMUX_prev;
	//vitesse_echantillon |= (ADCH << 8);
	//PORTB=ADMUX_prev;
	
	switch((ADMUX & (1 << MUX0))==0) // ADMUX = 0 --> moteur gauche, ADMUX = 1 --> moteur droit 
	{
		case GAUCHE:
		{
			if ((PINA & (1 << DIR_G)) > 0)
			{
				somme_vitesse[GAUCHE] -= vitesse_echantillon;
				nombre_echantillon[GAUCHE]++;
//				PORTB|=0b00000001;
				
			}
			else
			{
				somme_vitesse[GAUCHE] += vitesse_echantillon;
				nombre_echantillon[GAUCHE]++;
//				PORTB|=0b00000010;
			}
			break;
		}
		case DROIT:
		{
			if ((PINA & (1 << DIR_D)) > 0)
			{
				somme_vitesse[DROIT] -= vitesse_echantillon;
				nombre_echantillon[DROIT]++;
//				PORTB|=0b00000100;
			}
			else
			{
				somme_vitesse[DROIT] += vitesse_echantillon;
				nombre_echantillon[DROIT]++;
//				PORTB|=0b00001000;
			}
			break;
				
		}
		//default:
				//PORTB|=0b00000010;
			
		
	}

}

void moyenne_Moteur(float vitesseMoteur[])
{ int16_t tp_somme_vitesse_G, tp_somme_vitesse_D;
  uint8_t tp_nombre_ech_G, tp_nombre_ech_D;

		/*
		sprintf(debugChaine, "%d", nombre_echantillon[GAUCHE]);
		debug=1;
		UCSRB|=(1<<UDRIE);
		*/
	cli();
	tp_somme_vitesse_G = somme_vitesse[GAUCHE];
	tp_somme_vitesse_D = somme_vitesse[DROIT];
    tp_nombre_ech_G = nombre_echantillon[GAUCHE];
	tp_nombre_ech_D = nombre_echantillon[DROIT];
	nombre_echantillon[GAUCHE]=0; //Une fois l'opération de moyenne faite on réinitialise les données à 0
	somme_vitesse[GAUCHE]=0;
	nombre_echantillon[DROIT]=0;
	somme_vitesse[DROIT]=0;
	sei();	

	vitesseMoteur[DROIT]= ((float) tp_somme_vitesse_D)/((float) tp_nombre_ech_D);
		/*
		sprintf(debugChaine, "%d", somme_vitesse[GAUCHE]);
		debug=1;
		UCSRB|=(1<<UDRIE);
		*/
	
	vitesseMoteur[GAUCHE]=((float) tp_somme_vitesse_G)/((float) tp_nombre_ech_G);
}
