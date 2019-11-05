/**
 * @brief MAX7219驱动程序头文件
 * @fn max7219.h
 * @author shenxf 380406785@@qq.com
 * @version V1.1.0
 */ 
#ifndef _MAX7219_H__
#define _MAX7219_H__
#include "spi.h"
#include <inttypes.h>
#include <avr/io.h>

#define NoOp   (0x00U)  /**<空操作寄存器*/
#define Digit0 (0x01U)  /**<数码管1寄存器 */
#define Digit1 (0x02U)  /**<数码管2寄存器 */
#define Digit2 (0x03U)  /**<数码管3寄存器 */
#define Digit3 (0x04U)  /**<数码管4寄存器 */
#define Digit4 (0x05U)  /**<数码管5寄存器 */
#define Digit5 (0x06U)  /**<数码管6寄存器 */
#define Digit6 (0x07U)  /**<数码管7寄存器 */
#define Digit7 (0x08U)  /**<数码管8寄存器 */

#define DecodeModeReg  (0x0900U)  /**<译码模式寄存器*/
#define IntensityReg   (0x0a00U)  /**<亮度寄存器*/
#define ScanLimitReg   (0x0b00U)  /**<扫描位数寄存器*/
#define ShutDownReg    (0x0c00U)  /**<低功耗模式寄存器*/
#define DisplayTestReg (0x0f00U)  /**<显示测试寄存器*/

#define ShutdownMode    (0x00U)  /**<低功耗方式*/
#define NormalOperation (0x01U)  /**<正常操作方式*/

#define ScanDigit       (0x07U)  /**<扫描位数设置*/
#define DecodeDigit     (0xffU)  /**<译码设置*/
#define IntensityGrade  (0x0aU)  /**<亮度级别模式*/

#define TestMode        (0x01U)  /**<显示测试模式*/
#define TextEnd         (0x00U)  /**<显示测试结束*/

uint16_t SetDispMode(uint8_t mode);
uint16_t SetDispIntensity(uint8_t level);
uint16_t SetScanLimit(uint8_t digits);
void DispDigit(uint8_t ch, uint8_t pos);
void DispDigits(uint8_t str[]);
void DispTestStart(void);
void DispTestEnd(void);
void DispOnOff(uint8_t on);
#endif /* End of _MAX7219_H__ */
