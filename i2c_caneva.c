/******************************************************************************
 *
 *
 *****************************************************************************/


#include "i2c_caneva.h"

/******************************************************************************
* Initialisation de la vitesse du TWI
******************************************************************************/
void I2C_Init(void)
{
	TWBR=198;
	TWSR=(0 << TWPS1) | (1 << TWPS0);
}


/******************************************************************************
 * Insérer dans le buffer out
 *****************************************************************************/
void putDataOutBuf(uint8_t data){

	CircularBufferOutEnd++;
	CircularBufferOutEnd %= CIRCULAR_BUFFER_SIZE;
	CircularBufferOut[CircularBufferOutEnd] = data;

}


/******************************************************************************
 * Retirer du buffer out
 *****************************************************************************/
uint8_t getDataOutBuf(void){

	CircularBufferOutIndex++;
	CircularBufferOutIndex %= CIRCULAR_BUFFER_SIZE;
	return (uint8_t)CircularBufferOut[CircularBufferOutIndex];

}


/******************************************************************************
 * Insérer dans le buffer in
 *****************************************************************************/
void putDataInBuf(uint8_t *ptr){

	CircularBufferInEnd++;
	CircularBufferInEnd %= CIRCULAR_BUFFER_SIZE;
	CircularBufferIn[CircularBufferInEnd] = ptr;

}


/******************************************************************************
 * Retirer du buffer in
 *****************************************************************************/
uint8_t *getDataInBuf(void){

	CircularBufferInIndex++;
	CircularBufferInIndex %= CIRCULAR_BUFFER_SIZE;
	return CircularBufferIn[CircularBufferInIndex];	

}


/******************************************************************************
 * Écrire sur le bus twi
 *****************************************************************************/
void twiWrite(uint8_t address, uint8_t registre, uint8_t data){
		
	cli();

	putDataOutBuf(address);
	putDataOutBuf(registre);
	putDataOutBuf(data);
	
	TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);

	sei();

	/*

	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);

	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);

	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);
	*/
	
	

}

/******************************************************************************
 * lire sur le bus
 *****************************************************************************/
void twiRead(uint8_t address, uint8_t registre, uint8_t *ptr){

	cli();

	putDataOutBuf(address);
	putDataOutBuf(registre);
	putDataOutBuf(address+1);
	putDataInBuf(ptr);

	TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE);

	sei();

	/*
	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);

	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);

	sprintf(debugChaine, "%d", getDataOutBuf());
	debug=1;
	UCSRB|=(1<<UDRIE);
	_delay_ms(200);
	*/

}


/******************************************************************************
 *
 *****************************************************************************/
ISR(TWI_vect) {
	
	uint8_t volatile status  = TWSR & 0xF8;
	uint8_t volatile indexSuivant;

	//PORTB=status;
	//_delay_ms(2000);
		
	switch (status) {
		case	0x08: /* Start Condition */
		case	0x10: /* Restart Condition */
			
			/* 
				Si  nous avons un start ou un restart condition alors il faut envoyer l'addr 
				qui est dans le buffer Out et Activer le bus sans start/stop 
			*/
			testI2C=getDataOutBuf();
			TWDR=testI2C;			
			TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
			
			break;

		case	0x18: /* Address Write Ack */
		case	0x28: /* Data Write Ack */
		case	0x30: /* Date Write NoAck */
			
			/* 
				Si  nous avons un data ou une addr d'écrit sur le bus, ensuite il peut y avoir un autre data, 
				un stop ou un restart. Il faut donc lire le buffer pour savoir quoi faire et configure 
				le bus en conséquence 
			*/				

			if (CircularBufferOutEnd==CircularBufferOutIndex)//Si on est a la fin de notre buffer
			{
				TWCR = (1 << TWINT) | (1 << TWEN)| (1 << TWSTO); //On envoit une commande de stop
			}
			else
			{
				if (I2CReStart==1)//Si precedememt on a detecte un restart
				{
					I2CReStart=0;
					TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE); //On declanche un restart
				}

				else //Sinon c'est une donne a envoyer
				{

					TWDR=getDataOutBuf(); //On envoit la donne
					
					indexSuivant= (CircularBufferOutIndex+1)%CIRCULAR_BUFFER_SIZE; //On verifie si on n'aura besoin d'un restart apres l'envoit de cette data
					if(CircularBufferOut[indexSuivant]==0xE0 || CircularBufferOut[indexSuivant]==0xE1 
						|| CircularBufferOut[indexSuivant]==0xE2 || CircularBufferOut[indexSuivant]==0xE3)
					{
						I2CReStart=1;
					}
					
					TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
				}

			}
			
			break;

		case	0x50: /* Data Read Ack */
		case	0x58: /* Data Read NoAck */

			/* 
				Une lecture à été effectué sur le bus, il faut donc la récupérer 
			*/
			*getDataInBuf()=TWDR;
			if (CircularBufferOutEnd==CircularBufferOutIndex)//Si on est a la fin de notre buffer
			{
				TWCR = (1 << TWINT) | (1 << TWEN)| (1 << TWSTO); //On envoit une commande de stop
				PORTB=0x0F;
			}
			else
			{
				I2CReStart=0;
					TWCR = (1 << TWINT)|(1 << TWSTA)|(1 << TWEN)|(1 << TWIE); //On declanche un restart
			}

			break;



		case	0x40: /* Address Read Ack */

			/* 
				Puisqu'il n'y a pas de break dans les deux case 0x50 et 0x58, quand nous sommes ici
				nous avons soit lue la donnée ou envoyé l'addr à lire, il peut donc y avoir un stop, un
				start ou encore il faut placer le bus en mode lecture 
			*/

					TWCR = (1 << TWINT)|(1 << TWEN)|(1 << TWIE);
	
			break;

		case	0x48: /* Address Read NoAck */
		case	0x20: /* Address Write NoAck */

			/* 
				Ici l'un des deux sonars n'a pas répondu, il faut donc tout stoper ou faire un restart
			    pour la prochaine trame qui peut être dans le buffer 
			*/
			
			
			break;

		default : 
			/*
				Cette partie de code ne devrait pas être utile :)
			*/
			break;
	}
}
