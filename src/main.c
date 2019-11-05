/**
 * @brief  四位七段数码管显示器
 * @fn main.c
 * @author shenxf 380406785@@qq.com
 * @version V1.1.0
 * 
 * 按特定通讯协议接收串行命令并解析显示相应数据
*/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include "max7219.h"

#define     COMM_CMD_OV         (0x01U) /**<命令字节，表示显示溢出("1---")*/
#define     COMM_CMD_DATA       (0x02U) /**<命令字节，表示需显示的数据*/
#define     COMM_CMD_ZERO       (0x03U) /**<命令字节，表示显示"0."*/
#define     COMM_CMD_TESTON     (0x04U) /**<命令字节，表示开始自测试*/
#define     COMM_CMD_TESTOFF    (0x05U) /**<命令字节，表示关闭自测试*/
#define     COMM_STA_IDL        (0x01U) /**<状态字节，表示接收硬件标识字节*/
#define     COMM_STA_HAND       (0x02U) /**<状态字节，表示接收特征字节*/
#define     COMM_STA_CMD        (0x03U) /**<状态字节，表示接收命令字节*/
#define     COMM_STA_DATA       (0x04U) /**<状态字节，表示接收数据块*/
#define     COMM_STA_CHK        (0x05U) /**<状态字节，表示接收校验字节*/
#define     COMM_ACK_OK         (0x01U) /**<应答字节，表示接收正常*/
#define     COMM_ACK_CHK        (0x02U) /**<应答字节，表示接收校验错误*/
#define     COMM_ACK_COM        (0x03U) /**<应答字节，表示接收命令字节错误*/
#define     COMM_ACK_DATA       (0x04U) /**<应答字节，表示数据块错误*/

#ifdef prog_uint16_t
    static prog_uint16_t prog_baud = 0x67U; /**<存储在flash波特率分频数（baud = 9600，F_CPU=16M）*/
    static prog_uint8_t prog_parity = 0x00U;/**<存储在flash校验位编码，0无校验，1奇校验，2偶校验*/
#else
    static __flash const uint16_t prog_baud = 0x67U;/**<存储在flash波特率分频数（baud = 9600，F_CPU=16M）*/
    static __flash const uint8_t prog_parity = 0x00U;/**<存储在flash校验位编码，0无校验，1奇校验，2偶校验*/
#endif

static uint16_t eeprom_baud EEMEM = 0x67U;/**<存储在eeprom波特率分频数（baud = 9600，F_CPU=16M）*/
static uint8_t eeprom_parity EEMEM = 0x00U;/**<存储在eeprom校验位编码，0无校验，1奇校验，2偶校验*/

uint16_t cntbuf[42];/**<接收缓冲区*/
uint8_t rxbuf[16];/**<数据字串*/

static inline void uart_setbyte(int c)__attribute__((always_inline));
void SendAck(uint8_t ack, uint8_t id);

/**
 * @brief  串口发送数据内联函数
 * @param  c: 预发送的字节
 * @retval None
 */
void uart_setbyte(int c)
{
    loop_until_bit_is_set(UCSRA, UDRE);
    UDR = c;
}

/**
 * @brief  发送应答信息
 * @param  cmd: 应答信息编码
 * @param  id: 硬件标识字节
 * @retval None
 */
void SendAck(uint8_t cmd, uint8_t id)
{
    int8_t chk = 0;
    uart_setbyte(id);
    chk ^= id;
    uart_setbyte(0x55);
    chk ^= 0x55;
    uart_setbyte(cmd);
    chk ^= cmd;
    uart_setbyte(0x00);
    uart_setbyte(-chk);
}

/**
 * @brief  主函数
 */
int main(void)
{
    int i;
    uint8_t parity, err, chksum;
    uint8_t id;
    static uint16_t baud = 0, tmp = 0;
    int16_t ad_diff;
    PORTD |= _BV(PD6);
    wdt_enable(WDTO_2S);
    spi_init();
    SetDispMode(1);
    SetDispIntensity(12);
    SetScanLimit(4);
    DispOnOff(1);
    DispTestStart();
    ADCSRA = _BV(ADEN) | _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);
    ADMUX = _BV(REFS0);
    wdt_reset();
    for (i = 0; i < 32; i++)
    {
        ADCSRA |= _BV(ADSC);
        while (!(ADCSRA & _BV(ADIF)));
        ADCSRA |= _BV(ADIF);
        cntbuf[i] = ADC;
        _delay_ms(10);
    }
    tmp = cntbuf[1];
    for (i = 1; i < 32; i++)
    {
        ad_diff = (int)(tmp - cntbuf[i]);
    }
    ad_diff /= 16;
    if (ad_diff & 0x01)
    {
        ad_diff += 2;
    }
    ad_diff /= 2;
    tmp += ad_diff;
    if (tmp > 460 || baud < 307)
    {
        id = 0xaa;
    }
    else if (tmp > 358)
    {
        id = 0xbb;
    }
    else
    {
        id = 0xcc;
    }
    wdt_reset();
    TCCR1B = _BV(ICNC1) | _BV(CS11);
    baud = 0;
    TCCR1B &= ~(_BV(ICES1) | _BV(ICNC1) | _BV(CS11));
    if (baud)
    {
        if ((baud % 4) > 1)
        {
            baud += 4;
        }
        baud /= 4;
        baud--;
        if (baud != eeprom_read_word(&eeprom_baud))
        {
            eeprom_write_word(&eeprom_baud, baud);
        }
        parity = 0;
        do
        {
            UCSRB &= ~(_BV(RXEN) | _BV(TXEN));
            if (0x00 == parity)
            {
                UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
            }
            else if (0x01 == parity)
            {
                UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1) | _BV(UPM0);
            }
            else if (0x02 == parity)
            {
                UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1);
            }
            UBRRL = baud;
            UBRRH = baud >> 8;
            UCSRB = _BV(RXEN) | _BV(TXEN);
            i = 0;
            tmp = 0;
            TIFR |= _BV(TOV1);
            TCNT1 = 0;
            TCCR1B |= _BV(CS11);
            err = 1;
            do
            {
                if (UCSRA & _BV(RXC))
                {
                    if (!(UCSRA & (_BV(FE) | _BV(PE))))
                    {
                        err = 0;
                    }
                    UDR;
                    i++;
                }
                else if (TIFR & _BV(TOV1))
                {
                    TIFR |= _BV(TOV1);
                    wdt_reset();
                    tmp++;
                }
                else if (tmp > 140)
                {
                    err = 2;
                    break;
                }
            }
            while (i < 4);
            if (err != 1)
            {
                break;
            }
            parity++;
        }
        while (parity < 3);
        TCCR1B &= ~_BV(CS11);
        if (parity != eeprom_read_byte(&eeprom_parity))
        {
            eeprom_write_byte(&eeprom_parity, parity);
        }
        UCSRB &= ~(_BV(RXEN) | _BV(TXEN));
        if (0x00 == parity)
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
        }
        else if (0x01 == parity)
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1) | _BV(UPM0);
        }
        else if (0x02 == parity)
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1);
        }
        UCSRB = _BV(RXEN) | _BV(TXEN);
    }
    else
    {
        baud = eeprom_read_word(&eeprom_baud);
        parity = eeprom_read_byte(&eeprom_parity);
        if (0xffff == baud)
        {
            baud = pgm_read_word(&prog_baud);
            eeprom_write_word(&eeprom_baud, baud);
        }
        UBRRL = baud;
        UBRRH = baud >> 8;
        if (0xff == parity)
        {
            parity = pgm_read_byte(&prog_parity);
            eeprom_write_byte(&eeprom_parity, parity);
        }
        if (0x00 == parity)
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
        }
        else if (0x02 == parity)
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1);
        }
        else
        {
            UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0) | _BV(UPM1) | _BV(UPM0);
        }
        UCSRB = _BV(RXEN) | _BV(TXEN);
    }
    DispTestEnd();
    DispDigits((uint8_t*)"P   ");
    parity = COMM_STA_IDL;
    chksum = 0;
    i = 0;
    uart_setbyte(0x55);
    while (1)
    {
        wdt_reset();
        if (UCSRA & _BV(RXC))
        {
            err = UDR;
            if ((parity == COMM_STA_IDL) && (err == id))
            {
                parity = COMM_STA_HAND;
                chksum ^= err;
            }
            else if ((parity == COMM_STA_HAND) && (err == 0x55))
            {
                parity = COMM_STA_CMD;
                chksum ^= err;
            }
            else if (parity == COMM_STA_CMD)
            {
                parity = COMM_STA_DATA;
                chksum ^= err;
                rxbuf[i++] = err;
            }
            else if (parity == COMM_STA_DATA)
            {
                rxbuf[i++] = err;
                chksum ^= err;
                if (i >= 16)
                {
                    i = 0;
                    chksum = 0;
                    parity = COMM_STA_IDL;
                    SendAck(COMM_ACK_DATA, id);
                }
                else if (0x00 == err)
                {
                    parity = COMM_STA_CHK;
                }
                else
                {
                }
            }
            else if (parity == COMM_STA_CHK)
            {
                if ((chksum == 0x56) || (chksum == 0x45) || (chksum == 0x34))
                {
                    chksum++;
                }
                if ((uint8_t)(chksum + err) == 0)
                {
                    err = rxbuf[0];
                    if (err == COMM_CMD_OV)
                    {
                        DispDigits((uint8_t*)"1     ");
                        SendAck(COMM_ACK_OK, id);
                    }
                    else if (err == COMM_CMD_DATA)
                    {
                        DispDigits(&rxbuf[1]);
                        SendAck(COMM_ACK_OK, id);
                    }
                    else if (err == COMM_CMD_ZERO)
                    {
                        DispDigits((uint8_t*)"0.");
                        SendAck(COMM_ACK_OK, id);
                    }
                    else if (err == COMM_CMD_TESTON)
                    {
                        DispTestStart();
                        SendAck(COMM_ACK_OK, id);
                    }
                    else if (err == COMM_CMD_TESTOFF)
                    {
                        DispTestEnd();
                        SendAck(COMM_ACK_OK, id);
                    }
                    else
                    {
                        SendAck(COMM_ACK_COM, id);
                    }
                }
                else
                {
                    DispDigits((uint8_t*)"-----");
                    SendAck(COMM_ACK_CHK, id);
                }
                i = 0;
                chksum = 0;
                parity = COMM_STA_IDL;
            }
            else
            {
                parity = COMM_STA_IDL;
                i = 0;
                chksum = 0;
            }
        }
        wdt_reset();
    }
    return 0;
}
