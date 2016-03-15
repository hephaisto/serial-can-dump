#include <can.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

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

int main(void)
{
	OUT_DDR |= (1<<OUT_BIT);
	IN_DDR &= ~(1<<IN_BIT);
	OUT_PORT |= (1<<OUT_BIT);
	IN_PORT |= (1<<IN_BIT);

	can_init(BITRATE_500_KBPS);
	can_static_filter(can_filter);

	OUT_PORT &= ~(1<<OUT_BIT);

	uint8_t last_state = IN_PIN & (1<<IN_BIT);

	while (1)
	{
		if(can_check_message())
		{
			can_t inmsg;
			if(can_get_message(&inmsg))
			{
				OUT_PORT ^= (1<<OUT_BIT);
				if(inmsg.id == CAN_ID)
				{
				}
			}
		}
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
