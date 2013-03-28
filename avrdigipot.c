/* 
* Copyright (C) 2013 Andrzej Surowiec <emeryth@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
*
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL 
#include <util/delay.h>
#include "avrdigipot.h"


#define ENCA PD2
#define ENCB PD3
#define ENCSW PD4
#define PRECSW 
#define LEDR PC2
#define LEDG PC3
#define LEDB PC1
#define CS PB2
#define PWR_OUT PB0
#define TX PD1
#define RX PD0
#define UART_BUFFER_SIZE 16


#define VOLUME_STEPS 256

volatile int16_t volume=0;
volatile uint8_t power=0;
volatile uint8_t pressed=0;
volatile uint8_t command_ready=0;
volatile uint8_t precision_adj_on=0;

volatile uint8_t pwm_step=0;

volatile uint8_t portc=0;

volatile uint16_t red=0;
volatile uint16_t green=0;
volatile uint16_t blue=0;

volatile uint8_t uart_rx_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_rx_buffer_idx=0;

void send_pot_value(int8_t value){
	
	PORTB=(0<<CS)|(power<<PWR_OUT);
	SPDR=0x03; //write to wiper register A & B
	while(!(SPSR & (1<<SPIF))); 
	SPDR=value;
	while(!(SPSR & (1<<SPIF)));	
	PORTB=(1<<CS)|(power<<PWR_OUT);
}

inline void set_led_color(uint8_t r, uint8_t g, uint8_t b){
	red=r;
	green=g;
	blue=b;	
}

inline void change_led_color(int8_t r, int8_t g, int8_t b){
	red+=r;
	green+=g;
	blue+=b;
	
	if (red<0)
	red=0;
	else if (red>255)
	red=255;
	
	if (green<0)
	green=0;
	else if (green>255)
	green=255;
	
	if (blue<0)
	blue=0;
	else if (blue>255)
	blue=255;
}

uint8_t color_fade_func(uint16_t step){
	step&=0xff;
	
	if (step<85)
		return 0;
	else if (step<127)
		return (step-85)*6;
	else if (step<212)
		return 255;
	else if (step<255)
		return 255-(step-212)*6;
	else 
		return 0;
	
}

void rainbow(uint8_t step){

uint8_t	r=color_fade_func(step+169);
uint8_t	g=color_fade_func(step+85);
uint8_t	b=color_fade_func(step);
	
	set_led_color(r,g,b);
}

void set_volume(){
	if (power==1){
	rainbow(volume);
	send_pot_value(volume);
	}	
}

inline void write_uart(char chr){
	while( !( UCSRA & (1<<UDRE)) );
			UDR=chr;
}

void write_uart_str(char *str,uint8_t size){
	uint8_t i;
	
	for (i=0;i<size;i++){
	while( !( UCSRA & (1<<UDRE)) );
			UDR=str[i];			
	}

}

void exec_command(){
	command_ready=0;
	
	uint8_t vol=0;
	if (uart_rx_buffer[0]=='v'){//change volume
		vol=(uart_rx_buffer[1]&0xF)*100;
		vol+=(uart_rx_buffer[2]&0xF)*10;
		vol+=(uart_rx_buffer[3]&0xF);
		volume=vol&255;
		set_volume();
		write_uart_str("OK\r\n",4);
	}
	else if (uart_rx_buffer[0]=='o'){
		if (uart_rx_buffer[1]=='n'){
			power_on();
			write_uart_str("OK\r\n",4);
		}			
		else if (uart_rx_buffer[1]=='f'&&uart_rx_buffer[2]=='f'){
			power_off();
			write_uart_str("OK\r\n",4);
		}			
	}
	else if (uart_rx_buffer[0]=='g'&&uart_rx_buffer[1]=='e'&&uart_rx_buffer[2]=='t'){
			
			write_uart('O');
			write_uart('K');
			write_uart((volume/100)+'0');
			write_uart(((volume/10)%10)+'0');
			write_uart((volume%10)+'0');
			write_uart('\r');
			write_uart('\n');
	}		
	else{
		write_uart_str("NO\r\n",4);
	}
	
	uart_rx_buffer_idx=0;
	uart_rx_buffer[0]=0;
}


//UART receive interrupt
ISR(USART_RXC_vect){
	char chr;
			chr=UDR;
			write_uart(chr);
			if (chr=='\r'||chr=='\n'){
				command_ready=1;
			}
			else if (uart_rx_buffer_idx<UART_BUFFER_SIZE){
				uart_rx_buffer[uart_rx_buffer_idx]=chr;
				uart_rx_buffer_idx++;
			}					
}
	

//External interrupt when encoder turns
ISR(INT0_vect){
	if (power==1){
	if (PIND&(1<<ENCB)){//Encoder B output high - CW rotation
		if (PIND&(1<<ENCSW))
			volume+=4;
		else{//Encoder button pressed - precision adjust
		precision_adj_on=1;
		volume+=1;
		}		
	}
	else{
		if (PIND&(1<<ENCSW))
			volume-=4;
		else{
			precision_adj_on=1;
			volume-=1;
		}
		
	}
	
	
	if (volume<0)
	volume=0;
	else if (volume>VOLUME_STEPS-1)
	volume=VOLUME_STEPS-1;
	
	set_volume();
	}
}	


//Timer interrupt for software PWM
ISR(TIMER0_OVF_vect){
	portc=0;
	pwm_step++;
	
	if (pwm_step<red)
		portc|=(1<<LEDR);
	if (pwm_step<green)
		portc|=(1<<LEDG);
	if (pwm_step<blue)
		portc|=(1<<LEDB);
	
	PORTC=portc;
}

inline void power_on(){
	power=1;
	set_volume();
	PORTB=(1<<CS)|(power<<PWR_OUT);	
}

inline void power_off(){
	power=0;
	set_led_color(0,0,0);
	PORTB=(1<<CS)|(power<<PWR_OUT);	
}

int main(void)
{
	//Setup IO ports
	PORTD=(1<<ENCA)|(1<<ENCB)|(1<<ENCSW);
	DDRC=(1<<LEDR)|(1<<LEDG)|(1<<LEDB);
	DDRB=(1<<PB2)|(1<<PB3)|(1<<PB5)|(1<<PWR_OUT);
	
	//Setup Timer 0
	TCCR0=(1<<CS00);//Timer 0 on, clk/1
	TIMSK=(1<<TOIE0); //Timer 0 interrupt enabled
	
	//Setup SPI
	SPCR=(1<<SPE)|(1<<MSTR);
	
	//Setup external interrupts
	MCUCR=(1<<ISC00)|(1<<ISC01); //Enable interrupt on INT0 rising edge
	GICR=(1<<INT0); //Enable INT0 external interrupt
	sei(); //Enable interrupts
	
	//Setup UART
	UBRRH=0;
	UBRRL=51; //For 9600bps
	
	
	UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE); //Enable rx,tx and rx interrupt
	UCSRC=(1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); //8-bit character size
	
	PORTB=(1<<CS)|(power<<PWR_OUT);
	
	send_pot_value(volume);
    while(1)
    {
         _delay_ms(1);
		if (!(PIND&(1<<ENCSW))){//Encoder button pressed
			if (pressed==0){
			pressed=1;	
			}				
		}
		else if (pressed==1){//Button released
			pressed=0;
			if (precision_adj_on==0){
			if (power==0){
				power_on();
			}
			else{
				power_off();
				
			}
			}
			precision_adj_on=0;
		}		

		if (command_ready)
			exec_command();
			
    }
}