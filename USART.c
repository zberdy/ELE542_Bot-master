#include "USART.h"

extern volatile uint8_t debug;
extern char debugChaine[TAILLE_CHAINE];
volatile uint8_t compteur=TAILLE_CHAINE, donneRecue;
extern volatile uint8_t etat;
extern volatile float vitesseCommande;
extern volatile float angleCommande;
void USART_Init()
{
	UBRRH = 0x00;
	UBRRL = UART_BAUD_SELECT; //(uint8_t)(F_CPU / 153600) - (uint8_t)1;
	//- vitesse = 9600 Baud - 1 bit d’arrêt - Mode : XXXXXXXXXXX
	//- 8 bits de données - pas de parité
	UCSRA = (0 << RXC) | (0 << TXC)	| (1 << UDRE) | (0 << FE) | (0 << DOR) | (0 << PE) | (0 << U2X)	| (0 << MPCM);
	UCSRB = (1 << RXCIE) | (0 << TXCIE) | (0 << UDRIE) | (1 << RXEN)  |  (1 << TXEN) | (0 << UCSZ2)  | (0 << RXB8)	| (0 << TXB8);
	UCSRC = (1 << URSEL) | (0 << UMSEL) | (0 << UPM1) | (0 << UPM0) | (0 << USBS) | (1 << UCSZ1) | (1 << UCSZ0) | (0 << UCPOL);	 
}

//Intéruption pour la reception des trames de commandes
ISR(USART_RXC_vect)
{
	donneRecue=UDR;
	//PORTB^=0x01;
	if (debug==0)
	{
		//USART_Envoie(donneRecue);
		UCSRB|=(1<<UDRIE);
	}
		
	switch (etat)
		{

		case 1:
			{
				if (donneRecue == 0xF1) // réception d'une commande normale
				{
					etat = 2;
					break;
				}
			}

		case 2:
			{
				vitesseCommande = (float)donneRecue;
				etat = 3;
				break;
			}

		case 3:
			{
			
				angleCommande = (float)donneRecue;
				etat = 0;
				break;
			}
		}
}


//Intéruption pour l'envoi des données via l'USART
ISR(USART_UDRE_vect)
{
	if (debug==1)
	{
		if (compteur==TAILLE_CHAINE)
		{
			UDR=DEBUT_DEBUG;
			compteur=0;
		}
		else
		{
			if (debugChaine[compteur]!='\0')
			{
				UDR=debugChaine[compteur];
				compteur++;
			}
			else
			{
				UDR=FIN_DEBUG;
				UCSRB &= ~(1 << UDRIE);
				compteur=TAILLE_CHAINE;
				debug=0;
			}
		}
		
	}
	else
	{
		UDR=donneRecue;
		UCSRB &= ~(1 << UDRIE);
	}
	
}








