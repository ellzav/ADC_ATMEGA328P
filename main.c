/*
 * basic.c
 *
 * Created: 5/17/2020 10:29:57 AM
 * Author : Daniel
 */ 


#define F_CPU 16000000 // hz de cristal interno


//---Bibliotecas auxiliares
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#define Led (PORTB5) //Define o led no PORTB5
#define BUAD 9600
#define BRC ((F_CPU/16/BUAD)-1)
#define TX_BUFFER_SIZE 128

char serialBuffer[TX_BUFFER_SIZE];

uint8_t serialReadPos = 0; //inteiro sem sinal de 8 bits

uint8_t serialWritePos = 0;



#define set_bit(reg,bit) (reg |= (1<<bit)) // Define o bit a 1 
#define reset_bit(reg,bit) (reg &= ~(1<<bit)) // Define o bit a 0 

char buffer [30];
void appendSerial(char c){

	serialBuffer[serialWritePos] =c;

	serialWritePos++;

	

	if (serialWritePos >= TX_BUFFER_SIZE){

		serialWritePos = 0;

	}

}
void serialWrite(char c[]){

	for (uint8_t i=0; i< strlen(c); i++){

		appendSerial(c[i]); //a cada carater soma os anteriores e o proprio

	}

	if (UCSR0A & (1 << UDRE0)){

		UDR0 = 0; //envia um carater off

	}

}



void serialStart(){
	
	//=========================Para inicializar o monitor porta serial

	UBRR0H = (BRC >> 8); //Put BRC to UBRR0H and move it right 8 bits.

	UBRR0L = BRC;

	UCSR0B |= (1 << TXEN0) | (1 << TXCIE0);

	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); //8 BIT 

	//=========================Para inicializar o monitor porta serial



}
void startConversion(){
	
	//Mudar este bit para 1 para habilitar a conversão

	ADCSRA |= (1 << ADSC);
}
void startADC(){
	
	ADMUX=0b11000000; //ADMUX AREF INTERNO ADC0
	ADCSRA|=(1 << ADEN) |(1 << ADIE); // Ligar ADC e Ligar Interrupção ADC
	/*
	O prescaler deve ser o mais elevado porque normalmente os ADC trabalham entre 50kHz a 200KHz
	*/
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2); //128 prescaler
	
	//Desligar este pino mode digital para evitar erros de leitura
	
	DIDR0 |= (1 << ADC0D);
	startConversion();
}
int main(void)
{
	//Ativar interrupções

	SREG |= (1 << 7);

startADC();
    while (1) 
    {
		itoa(ADCL, buffer, 2);

		serialWrite("ADCL value is ");

		serialWrite(buffer);

		serialWrite("\n");



		itoa(ADCH, buffer, 2);

		serialWrite("ADCH value is ");

		serialWrite(buffer);

		serialWrite("\n");



		_delay_ms(1500);
	}
}

ISR(ADC_vect)// A cada interrupção de leitura ele reinicia a conversão

{

	startConversion();

}


ISR(USART_TX_vect){
// a cada transmissão bem efecutada 
	if(serialReadPos != serialWritePos){

		UDR0 = serialBuffer[serialReadPos];

		serialReadPos++;

		

		if(serialReadPos >= TX_BUFFER_SIZE){

			serialReadPos=0;

		}

	}

}