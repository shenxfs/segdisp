#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "max7219.h"

#define 	COMM_CMD_OV			0x01
#define 	COMM_CMD_DATA		0x02
#define     COMM_CMD_ZERO		0x03
#define		COMM_CMD_TESTON		0x04
#define		COMM_CMD_TESTOFF	0x05
#define		COMM_STA_IDL		0x01
#define 	COMM_STA_HAND		0x02
#define		COMM_STA_CMD		0x03
#define		COMM_STA_DATA		0x04
#define		COMM_STA_CHK		0x05
#define		COMM_ACK_OK			0x01
#define		COMM_ACK_CHK		0x02
#define		COMM_ACK_COM		0x03
#define		COMM_ACK_DATA		0x04

#ifdef prog_uint16_t
static prog_uint16_t prog_baud=0x67;
static prog_uint8_t prog_parity=0x00;
#else
static __flash const uint16_t prog_baud=0x67;
static __flash const uint8_t prog_parity=0x00;
#endif

static uint16_t eeprom_baud EEMEM=0x67;
static uint8_t eeprom_parity EEMEM=0x00;

uint16_t cntbuf[42];
uint8_t rxbuf[16];

static inline void uart_setbyte(int c)__attribute__((always_inline));
uint16_t Qequation(uint16_t ad,uint16_t qk,uint16_t qb);
uint8_t *NumberStr(uint8_t str[],uint16_t Qnum8);
void SendAck(uint8_t ack,uint8_t id);
void uart_setbyte(int c)
{
  loop_until_bit_is_set(UCSRA, UDRE);
  UDR = c;
}
void SendAck(uint8_t cmd,uint8_t id)
{
	int8_t chk=0;
	uart_setbyte(id);
	chk^=id;
	uart_setbyte(0x55);
	chk^=0x55;
	uart_setbyte(cmd);
	chk^=cmd;
	uart_setbyte(0x00);
	uart_setbyte(-chk);		
}
uint8_t *NumberStr(uint8_t str[],uint16_t Qnum8)
{
	uint8_t i=2,ch;
	uint16_t num;
	Qnum8++;
	Qnum8-=0x0a00;
	num=Qnum8>>8;
	do
	{
		ch=num%10;
		ch=0x30+num%10;
		str[i--]=ch;
		num/=10;
	}
	while(num);
	str[3]='.';
	i=4;
	num=Qnum8%256;
	do
	{
		num*=10;
		ch=(uint8_t)(num>>8);
		str[i++]=0x30+ch;
		num&=0x00ff;
	}
	while(i<6);
	str[i]='\0';
	i=0;
	while(!str[i]) 
	{
		i++;
	}	
	return &str[i];
}


uint16_t GetBaud(void)
{
	uint16_t tmp;
	int16_t tcn=0;
	uint8_t st=0,rep=0,index=0;
	TIFR|=_BV(ICF1);
	TCCR1B&=~_BV(ICES1);
	do
	{
		if(TIFR&_BV(ICF1))
		{
			TIFR|=_BV(ICF1);
			TCCR1B|=_BV(ICES1);
			if(st==0)
			{
				st=1;
				tcn=0;
				TCNT1=0;
			}
			else if (st>=5)
			{
				TCCR1B&=~_BV(ICES1);
				TCNT1=0;
				tmp=ICR1;
				cntbuf[index++]=tmp;
				st=0;
				tcn=0;
				rep++;
			}
			else 
			{
				tmp=ICR1;
				cntbuf[index++]=tmp;
				st++;
			}
		}
		else if(TIFR&_BV(TOV1))
		{
			TIFR|=_BV(TOV1);
			wdt_reset();
			tcn++;
		}
		else if(tcn>=140)
		{
			break;
		}
	}	
	while(rep<8);
	if(rep)
	{
		tmp=cntbuf[3]-cntbuf[0];
		tcn=0;
		for(st=1;st<rep;st++)
		{
			tcn+=(int16_t)(tmp-(cntbuf[st*5+3]-cntbuf[st*5]));			
		}
		if(tcn)
		{
			if((tcn%rep)>rep/2)
			{
				tcn+=rep;
			}
			else if((-tcn%rep)>rep/2)
			{
				tcn-=rep;
			}
			tmp+=tcn/rep;
		}
		if((tmp%3)>1)
		{
			tmp+=3;
		}
		tmp/=3;
	}
	else
	{
		tmp=0;
	}
	return tmp;
}
int main(void)
{
	int i;
	uint8_t parity,err,chksum; 
	uint8_t id;
	static uint16_t baud=0,tmp=0;
	int16_t ad_diff;
	PORTD |=_BV(PD6);
	wdt_enable(WDTO_2S);
	spi_init();
	SetDispMode(1);
	SetDispIntensity(12);
	SetScanLimit(4);
	DispOnOff(1);
	DispTestStart();
	ADCSRA =_BV(ADEN)|_BV(ADPS0)|_BV(ADPS1)|_BV(ADPS2);
	ADMUX =_BV(REFS0);
	wdt_reset();
	for(i=0;i<32;i++)
	{
		ADCSRA |=_BV(ADSC);
		while(!(ADCSRA&_BV(ADIF)));
		ADCSRA|=_BV(ADIF);
		cntbuf[i]=ADC;
		_delay_ms(10);
	}
	tmp=cntbuf[1];
	for(i=1;i<32;i++)
	{
		ad_diff=(int)(tmp-cntbuf[i]);
	}
	ad_diff/=16;
	if(ad_diff&0x01)
	{
		ad_diff+=2;
	}
	ad_diff/=2;	
	tmp+=ad_diff;
	if(tmp>460||baud<307)
	{
		id=0xaa;
	}
	else if(tmp>358)
	{
		id=0xbb;
	}
	else
	{
		id=0xcc;
	}
	wdt_reset();
	TCCR1B=_BV(ICNC1)|_BV(CS11);
	baud=0;
	TCCR1B&=~(_BV(ICES1)|_BV(ICNC1)|_BV(CS11));
	if(baud)
	{
		if((baud%4)>1)
		{
			baud += 4;
		}
		baud /= 4;
		baud--;
		if(baud!=eeprom_read_word(&eeprom_baud))
		{
			eeprom_write_word(&eeprom_baud,baud);
		}
		parity=0;	
		do
		{
			UCSRB &=~(_BV(RXEN)|_BV(TXEN));
			if(0x00==parity)
			{
				UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0);
			}
			else if(0x01==parity)
			{
				UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1)|_BV(UPM0);	
			}
			else if(0x02==parity)
			{
				UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1);
			}
			UBRRL = baud;
			UBRRH = baud>>8;
			UCSRB =_BV(RXEN)|_BV(TXEN);
			i=0;
			tmp=0;
			TIFR|=_BV(TOV1);
			TCNT1=0;
			TCCR1B|=_BV(CS11);
			err=1;
			do
			{
				if(UCSRA&_BV(RXC))
				{
					if(!(UCSRA&(_BV(FE)|_BV(PE))))
					{
						err=0;
					}
					UDR;
					i++;
				}
				else if(TIFR&_BV(TOV1))
				{
					TIFR|=_BV(TOV1);
					wdt_reset();
					tmp++;
				}
				else if(tmp>140)
				{
					err=2;
					break;
				}	
			}
			while(i<4);
			if(err!=1)
			{
				break;
			}
			parity++;
		}
		while(parity<3);
		TCCR1B&=~_BV(CS11);
		if(parity!=eeprom_read_byte(&eeprom_parity))
		{
			eeprom_write_byte(&eeprom_parity,parity);
		}
		UCSRB &=~(_BV(RXEN)|_BV(TXEN));
		if(0x00==parity)
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0);
		}
		else if(0x01==parity)
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1)|_BV(UPM0);	
		}
		else if(0x02==parity)
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1);
		}
		UCSRB =_BV(RXEN)|_BV(TXEN);
	}
	else
	{
		baud=eeprom_read_word(&eeprom_baud);
		parity=eeprom_read_byte(&eeprom_parity);
		if(0xffff==baud)
		{
			baud=pgm_read_word(&prog_baud);
			eeprom_write_word(&eeprom_baud,baud);
		}
		UBRRL = baud;
		UBRRH = baud>>8;
		if(0xff==parity)
		{
			parity=pgm_read_byte(&prog_parity);
			eeprom_write_byte(&eeprom_parity,parity);
		}
		if(0x00==parity)
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0);
		}
		else if(0x02==parity)
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1);
		}
		else 
		{
			UCSRC =_BV(URSEL)|_BV(UCSZ1)|_BV(UCSZ0)|_BV(UPM1)|_BV(UPM0);	
		}
		UCSRB =_BV(RXEN)|_BV(TXEN);
	}
	DispTestEnd();
	DispDigits((uint8_t*)"P   ");
	parity=COMM_STA_IDL;
	chksum=0;
	i=0;
	uart_setbyte(0x55);
	while(1)
	{
		wdt_reset();
		if(UCSRA&_BV(RXC))
		{
			err=UDR;
			if((parity==COMM_STA_IDL)&&(err==id))
			{
				parity=COMM_STA_HAND;
				chksum^=err;
			}
			else if((parity==COMM_STA_HAND)&&(err==0x55))
			{
				parity=COMM_STA_CMD;
				chksum^=err;
			}
			else if(parity==COMM_STA_CMD)
			{
				parity=COMM_STA_DATA;
				chksum^=err;
				rxbuf[i++]=err;
			}
			else if(parity==COMM_STA_DATA)
			{
				rxbuf[i++]=err;
				chksum^=err;
				if(i>=16)
				{
					i=0;
					chksum=0;
					parity=COMM_STA_IDL;
					SendAck(COMM_ACK_DATA,id);
				}
				else if(0x00==err)
				{
					parity=COMM_STA_CHK;
				}
				else
				{
				}	
			}
			else if(parity==COMM_STA_CHK)
			{
				if((chksum==0x56)||(chksum==0x45)||(chksum==0x34))
				{
					chksum++;
				}
				if((uint8_t)(chksum+err)==0)
				{
					err=rxbuf[0];
					if(err==COMM_CMD_OV)
					{
						DispDigits((uint8_t*)"1     ");
						SendAck(COMM_ACK_OK,id);
					}
					else if(err==COMM_CMD_DATA)
					{
						DispDigits(&rxbuf[1]);
						SendAck(COMM_ACK_OK,id);
					}
					else if(err==COMM_CMD_ZERO)
					{
						DispDigits((uint8_t*)"0.");
						SendAck(COMM_ACK_OK,id);
					}
					else if(err==COMM_CMD_TESTON)
					{
						DispTestStart();
						SendAck(COMM_ACK_OK,id);
					}
					else if(err==COMM_CMD_TESTOFF)
					{
						DispTestEnd();
						SendAck(COMM_ACK_OK,id);
					}
					else
					{
						SendAck(COMM_ACK_COM,id);
					}
				}
				else
				{
					DispDigits((uint8_t*)"-----");
					SendAck(COMM_ACK_CHK,id);
				}
				i=0;
				chksum=0;
				parity=COMM_STA_IDL;
			}
			else
			{
				parity=COMM_STA_IDL;
				i=0;
				chksum=0;
			}
		}
		wdt_reset();
	}
	return 0;
}
