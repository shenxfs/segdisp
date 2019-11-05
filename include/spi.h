/**
 *@brief SPI接口驱动头文件
 *@fn   spi.h
 *@author shenxf 380406785@@qq.com
 *@version V1.1.0
 */  
#ifndef __SPI_H__
#define __SPI_H__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stddef.h>

#if defined(__AVR_ATmega8__)|defined(__AVR_ATmega48__)|\
    defined(__AVR_ATmega88__)|defined(__AVR_ATmega168__)
    #define SPIPORT PORTB
    #define SPIDDR  DDRB
    #define SCK  5
    #define MISO 4
    #define MOSI 3
    #define SS   2
#elif defined(__AVR_ATmega64__)|defined(__AVR_ATmega128__)
    #define SPIPORT PORTB
    #define SPIDDR  DDRB
    #define SCK  1
    #define MISO 2
    #define MOSI 3
    #define SS   0
#elif defined(__AVR_ATmega16__)|defined(__AVR_ATmega32__)
    #define SPIPORT PORTB
    #define SPIDDR  DDRB
    #define SCK  7
    #define MISO 6
    #define MOSI 5
    #define SS   4
#else
    #warning "device type not defined"
#endif /*End of defined...*/

void spi_transmit_block(void* den, const void* src, size_t n);
void spi_transmit_three(void*  den, const void*  src);
uint16_t spi_transmit_word(uint16_t data);
static inline void slaveEn(void)__attribute__((always_inline));
void slaveEn(void)
{
    SPIPORT &= ~_BV(SS);
}
static inline void slaveDis(void)__attribute__((always_inline));
void slaveDis(void)
{
    SPIPORT |= _BV(SS);
}

static inline void spi_init(void) __attribute__((always_inline));
void spi_init(void)
{
#ifdef SLAVSPI
    SPIDDR |= _BV(MISO);
    SPCR |= _BV(SPE);
#else
    SPIDDR |= _BV(MOSI) | _BV(SCK) | _BV(SS);
    SPCR |= _BV(MSTR) | _BV(SPE) | _BV(SPR1);
#endif /*End of SLAVESPI*/
}

static inline uint8_t spi_transmit_byte(uint8_t data) __attribute__((always_inline));
uint8_t spi_transmit_byte(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & _BV(SPIF)))
    {
        wdt_reset();
    }
    return SPDR;
}
#endif /*End of __SPI_H__*/
