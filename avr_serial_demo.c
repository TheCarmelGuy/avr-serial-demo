/************************************************************************/
/* ECE 138 Lab 11
* Komel Merchant
* Christian Wadsworth                                                                     */
/************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>

/************************************************************************/
/* Transmission stuff                                                   */
/************************************************************************/
volatile char tbuffer[7]; //transmit buffer
volatile unsigned char tptr;	//pointer in transmit buffer1
//initialization
volatile unsigned char command  = 0;

volatile unsigned char done = 0;
/************************************************************************/
/* Counter stuff                                                        */
/************************************************************************/
volatile int counter_flag;
volatile int16_t counter = 0;
volatile unsigned char second = 0;
volatile unsigned char disable_timer = 0;


//Simply send a single byte
ISR(USART_RX_vect) {

	//might actually have to be an array value
	command = UDR;
	done = 1;
}
//transmission complete interrupt
ISR(USART_TX_vect)
{
	if(tptr == 7) {
		UCSRB = UCSRB & 0xf7;	//disable transmitter
		tptr = 0;
		//done = 1;
	}
	else
	{
		UDR = tbuffer[tptr];
		tptr++;
		//done = 1;
	}
}

//Counter overflow interrupt
ISR(TIMER0_OVF_vect)
{
	counter ++;				//increment counter
	if (counter == 7) {		//half a second has passed
		PORTC = 0x00 ^ 0xff;	//set led off
		
		
	}
	 else if (counter ==15) {	//a second has passed
		TCNT0 = 0;				//reset counter register
		counter = 0;			//reset counter variable
		if (disable_timer == 0) {	
			PORTC = 0x01 ^ 0xff;
			second = 1;
		}
	
	}
	/*} else if (counter == 30) {
		TCNT0 = 0;
		counter = 0;
		PORTC = 0x00 ^ 0xff;
		
	}*/
	//done = 1;

}

int main(void) {
	
	cli();
	
	/************************************************************************/
	/* Test code                                                                     */
	/************************************************************************/
	DDRC = 0xff; //setc as output
	PORTC = 0xff;
	
	
	
	int clock_count;
	
	char temp[5] = "00:00";
	int i;
	for (i = 0 ; i < 5 ; i++) {
		tbuffer[i] = temp[i];
		
	}
	
	/************************************************************************/
	/* UART stuff                                                            */

	UBRRL = 23;					//baud rate = 9600
	UBRRH = 0;

	UCSRB = 0xd8;				//allow transmit and recieve interupts interrupt

	/************************************************************************/


	/************************************************************************/
	/* Clock stuff                                                          */
	TIMSK = 0x02;
	TCNT0 = 0;

	TCCR0 = 5; //set clock to 1024
	/************************************************************************/


	tbuffer[5] = 0x0a;			//append LF
	tbuffer[6] = 0x0d;			//append CR
	sei();




	
	while(1){
		// The block of code in the if(second) statement sets the 
		// output in terms of minutes and seconds based on the contents 
		// of tbuffer, which holds the count. Output string is in 
		// format mm:ss.
		if(second) {						
			if(tbuffer[4] == '9') {
				if (tbuffer[3] == '5') {
					if(tbuffer[1] =='9') {
						if(tbuffer[0] == '5') {
							tbuffer[4] = '0';
							tbuffer[3] = '0';
							tbuffer[1] = '0';
							tbuffer[0] = '0';
						} 
						else {
							tbuffer[4] = '0';
							tbuffer[3] = '0';
							tbuffer[1] = '0';
							tbuffer[0] += 0x01;
						}
					} else {
						tbuffer[4] = '0';
						tbuffer[3] = '0';
						tbuffer[1] += 0x01;	
					}
				}
				else {
					tbuffer[4]= '0';
					tbuffer[3] += 0x01;	
				}
			}
			else {
				tbuffer[4] += 0x01;
				
			}
			second = 0;		
		}

		if(command == 's')	//if the command is 's', stop the count
		{
				if (disable_timer == 1) {
					second = 0; //turn second off
					disable_timer = 0; //disable all future seconds from being counted
				}
		}

		if(command == 'S')	//if the command is 'S', start the count
		{
				disable_timer = 1;  //start counting seconds again
		}

		if(command == 'r')	//if the command is 'r', read the count
		{
			UCSRB = UCSRB | 0x08;		//allow transmission
			UDR = tbuffer[0];			//send the 0th byte
			tptr = 1;					//index of the next byte to send
			done = 0;
		}
		if (command == 'R') {	//if the command is 'R', reset the count and stop counting
			disable_timer = 1;	//stop counting
			tbuffer[4] = '0';	//reset count
			tbuffer[3] = '0';
			tbuffer[1] = '0';
			tbuffer[0] = '0';
			
			
		}
		while(done ==0);	//wait for next command
		
		
	}
	return 0;
}
