#ifndef _MAX7219_H__
#define _MAX7219_H__
#include "spi.h"
#include <inttypes.h> 
#include <avr/io.h>
#define NoOp   0x00  //空操作寄存器
#define Digit0 0x01  //数码管1寄存器，下同
#define Digit1 0x02
#define Digit2 0x03
#define Digit3 0x04
#define Digit4 0x05
#define Digit5 0x06
#define Digit6 0x07
#define Digit7 0x08

#define DecodeModeReg  0x0900  //译码模式寄存器
#define IntensityReg   0x0a00  //亮度寄存器
#define ScanLimitReg   0x0b00  //扫描位数寄存器
#define ShutDownReg    0x0c00  //低功耗模式寄存器
#define DisplayTestReg 0x0f00  //显示测试寄存器

#define ShutdownMode 0x00     //低功耗方式
#define NormalOperation 0x01  //正常操作方式

#define ScanDigit 0x07        //扫描位数设置
#define DecodeDigit 0xff      //译码设置
#define IntensityGrade 0x0a   //亮度级别模式

#define TestMode 0x01    //显示测试模式
#define TextEnd 0x00    //显示测试结束
uint16_t SetDispMode(uint8_t mode);
uint16_t SetDispIntensity(uint8_t level);
uint16_t SetScanLimit(uint8_t digits);
void DispDigit(uint8_t ch,uint8_t pos);
void DispDigits(uint8_t str[]);
void DispTestStart(void);
void DispTestEnd(void);
void DispOnOff(uint8_t on);
#endif
								