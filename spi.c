#include "spi.h"

void spi_transmit_three(void  *den,const void  *src)
{
	uint8_t *temp,*tmp,n=3;
	temp=(uint8_t*)src;
	tmp=(uint8_t*)den;
	temp+=n;
	tmp+=n;
	while(n--) *(--tmp)=spi_transmit_byte(*(--temp));	
}


void spi_transmit_block( void  *den,const void  *src,size_t n)
{
	uint8_t *temp,*tmp;
	temp=(uint8_t*)src;
	tmp=(uint8_t*)den;
	while(n--) *tmp++=spi_transmit_byte(*(--temp));	
}

uint16_t spi_transmit_word(uint16_t data)
{
	uint16_t tmp;
	uint8_t byte;
	tmp= spi_transmit_byte((uint8_t)(data>>8));
	byte=spi_transmit_byte((uint8_t)(data));
	return (tmp<<8)|(uint16_t)byte ;
}
