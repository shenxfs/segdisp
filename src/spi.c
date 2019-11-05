/**
 * @brief  SPI接口程序
 * @fn spi.c
 * @author shenxf 3804046785@@qq.com
 * @version V1.1.0
 */
#include "spi.h"

/**
 * @brief  SPI传输三个字节
 * @param  den: 目的指针
 * @param  src: 源指针
 * @retval None
 */
void spi_transmit_three(void*  den, const void*  src)
{
    uint8_t* temp, *tmp, n = 3;
    temp = (uint8_t*)src;
    tmp = (uint8_t*)den;
    temp += n;
    tmp += n;
    while (n--)
    {
        *(--tmp) = spi_transmit_byte(*(--temp));
    }
}

/**
 * @brief  SPI块传输
 * @param  den: 目的指针
 * @param  src: 源指针
 * @param  n: 传输数据字节长度
 * @retval None
 */
void spi_transmit_block(void*  den, const void*  src, size_t n)
{
    uint8_t* temp, *tmp;
    temp = (uint8_t*)src;
    tmp = (uint8_t*)den;
    while (n--)
    {
        *tmp++ = spi_transmit_byte(*(--temp));
    }
}

/**
 * @brief  
 * @note  SPI传输一个字 
 * @param  data: 字
 * @retval 返回一个字
 */
uint16_t spi_transmit_word(uint16_t data)
{
    uint16_t tmp;
    uint8_t byte;
    tmp = spi_transmit_byte((uint8_t)(data >> 8));
    byte = spi_transmit_byte((uint8_t)(data));
    return (tmp << 8) | (uint16_t)byte ;
}
