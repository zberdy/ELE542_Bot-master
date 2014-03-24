/*
 * twi.c
 *
 * Created: 20/03/2014 22:38:33
 *  Author: Hadrien
 */ 

#include "twi.h"
#include "USART.h"

//Variable USART
extern volatile uint8_t debug;
extern char debugChaine[TAILLE_CHAINE];
extern volatile uint8_t etat;

//Variable I2C
extern volatile uint16_t lectureI2C;
extern volatile uint8_t I2CLectureTerminee, I2CEcritureTerminee;
volatile uint8_t etatI2C=0, adresseI2C=0, i=0;
volatile uint8_t lecture[4], stop[4], I2CRx[4], I2CTx[4];

void TWI_Init(void)
{
	TWBR=198;
	TWSR=(0 << TWPS1) | (1 << TWPS0);
}

void I2C_Commande (uint8_t ADRI2C, uint8_t registreI2C, uint8_t dataI2C)
{
	etatI2C=1;
	adresseI2C=ADRI2C;
	lecture[0]=0;
	lecture[1]=0;
	stop[0]=0;
	stop[1]=1;
	I2CTx[0]=registreI2C; //Num regitre gain du sonar
	I2CTx[1]=dataI2C;
	TWCR=(1 << TWINT)|(1 << TWEN)|(1 << TWIE)|(1 << TWSTA); //On active l'interuption et on lance un start
}

void I2C_Mesure (uint8_t ADRI2C)
{
	etatI2C=1;
	adresseI2C=ADRI2C;
	lecture[0]=0;
	lecture[1]=1;
	lecture[2]=0;
	lecture[3]=1;
	stop[0]=0;
	stop[1]=0;
	stop[2]=0;
	stop[3]=1;
	I2CTx[0]=0x02;
	I2CTx[2]=0x03;
	TWCR=(1 << TWINT)|(1 << TWEN)|(1 << TWIE)|(1 << TWSTA);
} 

ISR (TWI_vect)
{
	switch (etatI2C)
	{
		//Start bit-------------------------------------------------
		case 0:
		TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);
		//On envoit START et on baisse le drapeau
		etatI2C=1;
		break;
		
		//Verification de la reception du start bit-----------------
		case 1:
		/*
		sprintf(debugChaine, "%x", (TWSR & 0xF8));
		debug=1;
		UCSRB|=(1<<UDRIE);
		*/

		if (((TWSR & 0xF8) != I2C_START) && ((TWCR & 0xF8) != I2C_RESTART))
		{
			//Si le START/RESTART n'a pas été reçu on le renvoit
			etatI2C=0;
			//TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		} 
		else
		{
			// Si le START/RESTART a été reçue on peut envoyer l'adresse
			etatI2C=2;
			//TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		break;
		
		//Envoit de l'adresse de l'esclave--------------------------
		case 2:
		if (lecture[i] == 0)
		{
			TWDR = adresseI2C; //Ex: TWDR=0xE0
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			etatI2C=3;
		} 
		else
		{
			TWDR = (adresseI2C) | (0x01); //Ex: TWDR=0xE1
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			etatI2C=4;
			
		}
		break;
		
		//Verification de la reception de l'addresse----------------
		case 3:
		
		sprintf(debugChaine, "%x", (TWSR & 0xF8));
		debug=1;
		UCSRB|=(1<<UDRIE);

		if ((TWSR & 0xF8) != I2C_ADRW_ACK)
		{
			//Si l'adresse n'a pas été reçue on la renvoie
			PORTB=1;
			etatI2C=2;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		else
		{
			//Si l'adresse a été reçue on envoit la donnée
			etatI2C=5;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		break;
		
		//Envoi de la donnée à écrire-------------------------------
		case 5:
		TWDR = I2CTx[i];
		TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		etatI2C=6;
		break;
		
		//Vérification de l'écriture de la donnée-------------------
		case 6:
		
		if ((TWSR & 0xF8) != I2C_DATA_ACK)
		{
			//Si la donnée n'a pas été correctement écrite, on la renvoie
			etatI2C=5;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		else
		{
			if (stop[i])
			{
				etatI2C=7; //S'il n'y a plus de lecture ou d'écriture à faire on libère la ligne
				I2CEcritureTerminee=1;
				TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			} 
			else
			{
				etatI2C=8; //S'il on veut écrire une nouvelle donnée ou lire une donnée on effectue un restart
				TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
				i++;
			}
		}
		break;
		
		//STOP------------------------------------------------------
		case 7:
		TWCR = (1 << TWINT) | (1 << TWEN)| (1 << TWSTO); //On envoit le bit de stop et on désactive les intéruptions
		etatI2C=0;
		i=0;
		break;
		
		//ReSTART---------------------------------------------------
		case 8:
		TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);
		etatI2C=1;
		break;
		
		//Vérification de la reception de l'adresse de lecture------
		case 4:
		if ((TWCR & 0xF8) != I2C_ADRR_ACK)
		{
			//Si l'adresse n'a pas été reçue on la renvoie
			etatI2C=2;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		else
		{
			//Si l'adresse a été reçue on commence la lecture
			etatI2C=9;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);//Lecture NACK car on lit qu'un byte
		}
		break; 
		
		//Lecture des données---------------------------------------
		case 9:
		I2CRx[i]=TWDR;
		if (stop[i])
		{
			etatI2C=7; //S'il n'y a plus de lecture ou d'écriture à faire on libère la ligne
			lectureI2C=(I2CRx[1] << 8);
			lectureI2C|=I2CRx[3];
			I2CLectureTerminee=1;
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
		}
		else
		{
			etatI2C=8; //S'il on veut écrire une nouvelle donnée ou lire une donnée on effectue un restart
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			i++;
		}		
		break;
		
	}
	
}
