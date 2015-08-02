/**/
#include "max7219.h"
#include <string.h>
#include <ctype.h>
uint8_t g_ucDispDigits,g_ucDispMode;
	static uint8_t digits[5];
uint16_t SetDispMode(uint8_t mode)
{
	uint16_t data;
	slaveEn();
	if(mode)
	{
		data=0x09ff;
	}
	else
	{
		data=0x0900;
	}
	data=spi_transmit_word(data);
	slaveDis();
	g_ucDispMode=mode;
	return data;
}
uint16_t SetDispIntensity(uint8_t level)
{
	uint16_t data;
	slaveEn();
	data=IntensityReg;
	data|=(uint16_t)(level&0x0f);
	data=spi_transmit_word(data);
	slaveDis();
	return data;
}
uint16_t SetScanLimit(uint8_t digits)
{
	uint16_t data;
	slaveEn();
	data=ScanLimitReg;
	if(digits)
	{
		data|=(uint16_t)((digits-1)&0x7);
		g_ucDispDigits=digits;
	}
	data=spi_transmit_word(data);
	slaveDis();
	return data;
}
void DispDigit(uint8_t ch,uint8_t pos)
{
	uint16_t data;
	if(pos>g_ucDispDigits)
	{
		pos=0;
	}
	else
	{
		pos&=0x07;
	}
	data=(uint16_t)pos+1;
	data<<=8;
	if(g_ucDispMode)
	{
		if(isdigit(ch))
		{
			ch-=0x30;
		}
		else if(ch&0x80)
		{
			if(isdigit(ch&0x7f))
			{
				ch-=0x30;
			}
			else
			{
				return;
			}
		}
		else if(ch==' ')
		{
			ch=0x0f;
		}
		else if(ch=='-')
		{
			ch=0x0a;
		}
		else if(ch=='P')
		{
			ch=0x0e;
		}
		else	
		{
			return;
		}
		data|=(uint16_t)ch;
		slaveEn();
		spi_transmit_word(data);
		slaveDis();
	}
}	
void DispDigits(uint8_t str[])
{
	uint8_t i,dot,n,ch;
	ch=str[0];	
	digits[0]=0x30;
	n=0;
	i=0;
	dot=0;
	while(ch==0x30||ch==0x20)
	{
		i++;
		ch=str[i];
	}
	if(ch)
	{
		do
		{
			if(ch=='.')
			{
				if(dot==0)
				{
					if(n)
					{
						digits[n-1]|=0x80;
					}
					else
					{
						digits[0]=0xb0;
						n++;
					}
				}
				dot=1;			
			}
			else if(n>=g_ucDispDigits)
			{
				break;
			}
			else
			{
				digits[n++]=ch;
			}
			i++;
			ch=str[i];
		}
		while(ch);	
	}
	else
	{
		digits[0]=0xb0;
		n=1;
	}
	digits[n]='\0';
	if(n<g_ucDispDigits)
	{
		for(i=0;i<g_ucDispDigits-n;i++)
		{
			DispDigit(0x20,i);			
		}		
		n=g_ucDispDigits-n;
		for(i=n;i<g_ucDispDigits;i++)
		{
			DispDigit(digits[i-n],i);			
		}
	}
	else
	{
		for(i=0;i<g_ucDispDigits;i++)
		{
			DispDigit(digits[i],i);			
		}
	}
}
void DispTestStart(void)
{
	uint16_t data;
	slaveEn();
	data=0x0f01;
	spi_transmit_word(data);
	slaveDis();
}
void DispTestEnd(void)
{
	uint16_t data;
	slaveEn();
	data=0x0f00;
	spi_transmit_word(data);
	slaveDis();
}
void DispOnOff(uint8_t on)
{
	uint16_t data;
	slaveEn();
	if(on)
	{
		data=0x0C01;
	}
	else
	{
		data=0x0C00;
	}
	spi_transmit_word(data);
	slaveDis();
}

