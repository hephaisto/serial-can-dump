#include <can.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "fleury/uart.h"

#define OUT_PORT PORTB
#define OUT_DDR DDRB
#define OUT_BIT 1

#define IN_PORT PORTB
#define IN_DDR DDRB
#define IN_BIT 0
#define IN_PIN PINB


const uint8_t can_filter[] PROGMEM = 
{
	MCP2515_FILTER(0),
	MCP2515_FILTER(0),
	
	MCP2515_FILTER_EXTENDED(0),
	MCP2515_FILTER_EXTENDED(0),
	MCP2515_FILTER_EXTENDED(0),
	MCP2515_FILTER_EXTENDED(0),
	
	MCP2515_FILTER(0),
	MCP2515_FILTER_EXTENDED(0),
};

#define CAN_ID 0x01

#define BIT_RTR 6
#define BIT_EXT 5

#define UART_BAUD_RATE 38400
void can_to_uart(can_t *msg)
{
	uint8_t buf[4];
	buf[0]=( (msg->id&(0xFF000000)) >> 24);
	buf[1]=( (msg->id&(0x00FF0000)) >> 16);
	buf[2]=( (msg->id&(0x0000FF00)) >> 8);
	buf[3]=( (msg->id&(0x000000FF)));

	buf[0] |= ((msg->flags.rtr & 0x01) << BIT_RTR);
	buf[0] |= ((msg->flags.extended & 0x01) << BIT_EXT);
	
	uint8_t i;
	for(i=0; i<4; i++)
		uart_putc(buf[i]);

	uart_putc(msg->length);

	for(i=0; i<msg->length; i++)
		uart_putc(msg->data[i]);
}

uint8_t read_uart_blocking()
{
	int result;
	while((result=uart_getc()) & UART_NO_DATA)
		;
	return result & 0xFF;
}

void uart_to_can(uint8_t first_buf)
{
	can_t msg;
	uint8_t buf[4];
	buf[0]=first_buf;
	uint8_t extended=(buf[0] & (1<< BIT_EXT)) >> BIT_EXT;
	uint8_t rtr=(buf[0] & (1<< BIT_RTR)) >> BIT_RTR;
	buf[1]=read_uart_blocking();
	buf[2]=read_uart_blocking();
	buf[3]=read_uart_blocking();
	if(extended)
	{
		msg.id=(((uint32_t) (buf[0]&(0x1F)))<<24) & (((uint32_t) buf[1])<<16) & (((uint32_t) buf[2]) <<8) & buf[3];
	}
	msg.flags.extended = extended;
	msg.flags.rtr=rtr;

	msg.length=read_uart_blocking();
	uint8_t i;
	for(i=0;i<msg.length;i++)
		msg.data[i]=read_uart_blocking();
	can_send_message(&msg);
}

void init_timer0()
{
	// original by david
	/*TCCR1A=1<<COM1A0;
	TCCR1B=1<<WGM12 | 1<<CS10;
	OCR1A=0;*/

	// timer0
	TCCR0A=(1<<COM1A0) | (1<<WGM01);
	TCCR0B=1<<CS10;
	OCR1A=0;
	DDRD|= 1<<6;
}

int main(void)
{
	init_timer0();
	OUT_DDR |= (1<<OUT_BIT);
	IN_DDR &= ~(1<<IN_BIT);
	OUT_PORT |= (1<<OUT_BIT);
	IN_PORT |= (1<<IN_BIT);

	//can_init(BITRATE_500_KBPS);
	can_init(BITRATE_1_MBPS); // double datarate because F_osc is 8MHz instead of 16 when using Timer0 as CLK
	can_static_filter(can_filter);

	OUT_PORT &= ~(1<<OUT_BIT);

	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) );
	sei();

	uint8_t last_state = IN_PIN & (1<<IN_BIT);

	while (1)
	{
		// read can messages
		if(can_check_message())
		{
			can_t inmsg;
			if(can_get_message(&inmsg))
			{
				can_to_uart(&inmsg);
				if(inmsg.id == CAN_ID)
				{
					OUT_PORT ^= (1<<OUT_BIT);
				}
			}
		}

		// read uart
		int uart_char=uart_getc();
		if(!(uart_char & UART_NO_DATA) ) // data available
		{
			uart_to_can(uart_char&0xFF);
		}

		// check gpio
		uint8_t now_state = IN_PIN & (1<<IN_BIT);
		if(now_state!=last_state)
		{
			OUT_PORT ^= (1<<OUT_BIT);
			last_state=now_state;

			can_t msg;

			msg.id = CAN_ID;
			msg.flags.rtr = 0;
			msg.flags.extended = 1;

			msg.length = 1;
			msg.data[0] = 0xff;

			can_send_message(&msg);
			_delay_ms(10);
		}
	}
}
