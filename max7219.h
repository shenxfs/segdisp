#ifndef _MAX7219_H__
#define _MAX7219_H__
#include "spi.h"
#include <inttypes.h> 
#include <avr/io.h>
#define NoOp   0x00  //�ղ����Ĵ���
#define Digit0 0x01  //�����1�Ĵ�������ͬ
#define Digit1 0x02
#define Digit2 0x03
#define Digit3 0x04
#define Digit4 0x05
#define Digit5 0x06
#define Digit6 0x07
#define Digit7 0x08

#define DecodeModeReg  0x0900  //����ģʽ�Ĵ���
#define IntensityReg   0x0a00  //���ȼĴ���
#define ScanLimitReg   0x0b00  //ɨ��λ���Ĵ���
#define ShutDownReg    0x0c00  //�͹���ģʽ�Ĵ���
#define DisplayTestReg 0x0f00  //��ʾ���ԼĴ���

#define ShutdownMode 0x00     //�͹��ķ�ʽ
#define NormalOperation 0x01  //����������ʽ

#define ScanDigit 0x07        //ɨ��λ������
#define DecodeDigit 0xff      //��������
#define IntensityGrade 0x0a   //���ȼ���ģʽ

#define TestMode 0x01    //��ʾ����ģʽ
#define TextEnd 0x00    //��ʾ���Խ���
uint16_t SetDispMode(uint8_t mode);
uint16_t SetDispIntensity(uint8_t level);
uint16_t SetScanLimit(uint8_t digits);
void DispDigit(uint8_t ch,uint8_t pos);
void DispDigits(uint8_t str[]);
void DispTestStart(void);
void DispTestEnd(void);
void DispOnOff(uint8_t on);
#endif
								