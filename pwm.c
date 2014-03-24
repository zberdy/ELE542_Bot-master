/**
 * \file PWM.c
 * \brief Modulation des cycles de service du moteur.
 * \author	Hadrien Beck
 * \author Daniel Thiffeault
 * \version 1.0
 * \date 28 janvier  2014
 * \details PWM est une méthode de modulation qui permet d'ajuster le cycle
 *  de service des moteurs. Le cycle sera constamment ajusté à partir des 
 *  valeurs retournées par la fonction \a CalculPWM(). 
 */

/**
 * \fn void PWM_Init(void) 
 * \brief Configure le timer en mode fast PWM
 * \return Aucune.
 */

#include "pwm.h"


//Variable debug USART
extern char debugChaine[TAILLE_CHAINE];
extern uint8_t debug;


//Variables de stockage pour la vitesse des moteurs durant l'intéruption de l'ADC 
extern volatile uint8_t nombre_echantillon[2];
extern volatile uint16_t somme_vitesse[2];
extern volatile uint8_t flag5ms;

static volatile uint16_t compteur1;
static volatile uint8_t calibration_rdy;
static volatile uint8_t calibration_req;

//Variable de calibration des moteurs
extern float vMaxPlus[2], vZeroPlus[2], vMaxMoins[2], vZeroMoins[2];
extern float aPlus[2], aMoins[2];

void PWM_Init(void) {
	
	//- Fast PWM
	//PWMA = moteur droit - Interruption de débordement
	//- Niveau haut au débordement
	//- Niveau bas à la comparaison
	// Configuration mode 14 (Fast PWM avec ICR1)
	DDRD =0b11111100; //(1 << PIND4) | (1 << PIND5);  //Pin 4 et 5 configurées en sortie
	TCCR1A =  (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1)  | (0 << COM1B0)   |  (0 << FOC1A)  |  (0 << FOC1B) |  (1 << WGM11) | (0 << WGM10);
	TCCR1B =  (0 << ICNC1)  | (0 << ICES1)  | (0 << 5)      |  (1 << WGM13)    |  (1 << WGM12)  | ( 0 << CS12)  |  (1 << CS11)  | (0 << CS10);
	OCR1A = 0;                                                                                                                                                                                                                                                                  
	OCR1B = 0;
	ICR1 = PWM_TOP;
	TIMSK = (0 << OCIE2)  | (0 << TOIE2)  | (0 << TICIE1) |   (0 << OCIE1A) | (0 << OCIE1B) |  (1 << TOIE1) | (0 << OCIE0) | (0 << TOIE0);
	
}



ISR(TIMER1_OVF_vect){
	
	// Déclenché à chaque 5 ms en conformité est avec les paramètres d'initialisation 
	// de la fonction PWM_init().
	flag5ms=1;
	//PORTB ^= 1;
	//compteur1=(compteur1+1)%250;
	
	if (calibration_req == 1)
	{
		compteur1++;
		if (compteur1 >= REGIME_TRANSITOIRE)
		{
		    //PORTB ^= 1;
			calibration_req = 0;
			compteur1 = 0;
			calibration_rdy = 1;

		}
		
	}

	
	
}


void PWM_calibrer(void)
{
	//Calibration de Vmax+
	OCR1A=0;
	OCR1B=0;
	PORTD |= (1 << DIR_G1) | (0<<DIR_G2) | (1 << DIR_D1)| (0 << DIR_D2); //conf V+
	PORTA |= (1 << CAL);

	//Lancer aquisition
	compteur1=0;
	calibration_req=1;
	calibration_rdy = 0;
	while (calibration_rdy!=1) // On attend que le moteur soit en régime permanent (2.5s)
	;
	PORTB=0x01;

	calibration_rdy=0;
	nombre_echantillon[GAUCHE]=0;
	nombre_echantillon[DROIT]=0;
	somme_vitesse[GAUCHE]=0;
	somme_vitesse[DROIT]=0;
	while(nombre_echantillon[GAUCHE]<ECHANTILLON_INIT && nombre_echantillon[DROIT]<ECHANTILLON_INIT)
	//	_delay_ms(50);
	;//On attend que l'ADC est recolté assez d'échantillons
	
	moyenne_Moteur(vMaxPlus);

	/*
	sprintf(debugChaine, "%d", vMaxPlus[GAUCHE]);
	debug=1;
	UCSRB|=(1<<UDRIE);
	*/
	
	
	//Calibration de Vzero+
	PORTD |= (1 << DIR_G1)  | (1 << DIR_D1); //conf V+
	PORTD &=   ~(1 << DIR_G2) | ~(1 << PWM_G) | ~(1 << PWM_D) |  ~(1 << DIR_D2); //conf V+
	PORTA &= ~(1 << CAL);
	calibration_req = 1;
	calibration_rdy =0;
	while (calibration_rdy!=1)
	;
	nombre_echantillon[GAUCHE]=0;
	nombre_echantillon[DROIT]=0;
	somme_vitesse[GAUCHE]=0;
	somme_vitesse[DROIT]=0;
	calibration_rdy=0;
	while(nombre_echantillon[GAUCHE]<ECHANTILLON_INIT && nombre_echantillon[DROIT]<ECHANTILLON_INIT)
	;//On attend que l'ADC est recolté assez d'échantillons
	moyenne_Moteur(vZeroPlus);
	

	PORTB^=0b00000010;
	/*
	sprintf(debugChaine, "%d\r\n", (int)(vZeroPlus[GAUCHE]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
	*/
	//Calibration de Vmax-
	PORTD |= (1 << DIR_G2) | (1 << DIR_D2); //conf V-
	PORTD &= ~(1 << DIR_G1) |  ~(1 << PWM_G) | ~(1 << PWM_D) | ~(1 << DIR_D1); //conf V-
	PORTA |= (1 << CAL);
	calibration_req = 1;
	calibration_rdy =0;
	while (calibration_rdy!=1)
	;
	nombre_echantillon[GAUCHE]=0;
	nombre_echantillon[DROIT]=0;
	somme_vitesse[GAUCHE]=0;
	somme_vitesse[DROIT]=0;
	calibration_rdy=0;
	while(nombre_echantillon[GAUCHE]<ECHANTILLON_INIT && nombre_echantillon[DROIT]<ECHANTILLON_INIT)
	;//On attend que l'ADC est recolté assez d'échantillons
	moyenne_Moteur(vMaxMoins);
	PORTB^=0b00000100;
/*	
	sprintf(debugChaine, "%d\r\n", (int)(vMaxMoins[GAUCHE]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
*/	
	//Calibration de Vzero-
	PORTD |= (1 << DIR_G2) |  (1 << DIR_D2); //conf V+
	PORTD &= ~(1 << DIR_G1) |  ~(1 << PWM_G) | ~(1 << PWM_D) | ~(1 << DIR_D1); //conf V+
	PORTA &= ~(1 << CAL);
	calibration_req = 1;
	calibration_rdy =0;
	while (calibration_rdy!=1)
	;
	nombre_echantillon[GAUCHE]=0;
	nombre_echantillon[DROIT]=0;
	somme_vitesse[GAUCHE]=0;
	somme_vitesse[DROIT]=0;
	calibration_rdy=0;
	while(nombre_echantillon[GAUCHE]<ECHANTILLON_INIT && nombre_echantillon[DROIT]<ECHANTILLON_INIT)
	; //On attend que l'ADC est recolté assez d'échantillons
	moyenne_Moteur(vZeroMoins);
	PORTB^=0b00001000;
/*	
	sprintf(debugChaine, "%d\r\n", (int)(vZeroMoins[GAUCHE]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
*/	
	//Une fois tous nos échantillons récupérés, on calcul les termes des fonctions de correction (y=a.x+b)
	aPlus[GAUCHE] = (float)(((float)1023) / ((float)vMaxPlus[GAUCHE] - (float)vZeroPlus[GAUCHE]));
	aPlus[GAUCHE] /= 1024.0;
	aPlus[DROIT]  = (float)(((float)1023) / ((float)vMaxPlus[DROIT] - (float)vZeroPlus[DROIT]));
	aPlus[DROIT] /= 1024.0;
	
	//bPlus[GAUCHE] = (float)(-aPlus[GAUCHE]*vZeroPlus[GAUCHE]/1024); //On divise ici le coef b par 1024 afin d'éviter une division de float
	//bPlus[DROIT]  = (float)(-aPlus[DROIT]*vZeroPlus[DROIT]/1024); //

	aMoins[GAUCHE] = (float)(((float)1023) / ((float)vMaxMoins[GAUCHE] - (float)vZeroMoins[GAUCHE]));
	aMoins[GAUCHE] /= 1024.0;
	aMoins[DROIT] = (float)(((float)1023) / ((float)vMaxMoins[DROIT] - (float)vZeroMoins[DROIT]));
	aMoins[DROIT] /= 1024.0;

	
	//bMoins[GAUCHE]= (float)(-aMoins[GAUCHE]*vZeroMoins[GAUCHE]/1024);
	//bMoins[DROIT] = (float)(-aMoins[DROIT]*vZeroMoins[DROIT]/1024);

	/*
	sprintf(debugChaine, "%d\r\n", (int)(aPlus[GAUCHE]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(20);

	sprintf(debugChaine, "%d\r\n", (int)(aMoins[GAUCHE]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(20);

	sprintf(debugChaine, "%d\r\n", (int)(aPlus[DROIT]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(20);

	sprintf(debugChaine, "%d\r\n", (int)(aMoins[DROIT]*1000));
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(20);
	*/




	

}


void ordre_Moteur2(float dutyCycleG, float dutyCycleD)
{
	//Cas Moteur Gauche
	
	if (dutyCycleG > 0.01)
	{
		OCR1B=(uint16_t)((float)ICR1*dutyCycleG);
		PORTD |= (1 << DIR_G1);
		PORTD &= ~(1 << DIR_G2);
	}
	
	else if (dutyCycleG < 0.01)
	{
		OCR1B=(uint16_t)((float)ICR1*(-dutyCycleG));
		PORTD |= (1 << DIR_G2);
		PORTD &= ~(1 << DIR_G1);
	}

	else 
	{
		PORTD |= (0 << DIR_D2) | (0 << DIR_D1) ;
	}

	//Cas Moteur Droit

	if (dutyCycleD > 0.01)
	{
		OCR1A=(uint16_t)((float)ICR1*dutyCycleD);
		PORTD |= (1 << DIR_D1);
		PORTD &= ~(1 << DIR_D2);
	}
	
	else if (dutyCycleD < 0.01)
	{
		OCR1A=(uint16_t)((float)ICR1*(-dutyCycleD));
		PORTD |= (1 << DIR_D2);
		PORTD &= ~(1 << DIR_D1);
	}

	else 
	{
		PORTD |= (0 << DIR_D2) | (0 << DIR_D1);
	}

}
