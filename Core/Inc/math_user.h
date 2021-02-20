#ifndef __MATH_USER_H
#define __MATH_USER_H

#include <stdint.h>

typedef enum
{
    false = 0,
    true = 1
}bool;

bool IsEqual( float f1, float f2, float eps );
float LittleEndingShortToFloat( uint8_t *buf, uint8_t scale );
void FloatToLittleEndingShort( uint8_t *buf, float data, uint16_t scale );
void PrintBuff( uint8_t *buf, uint8_t len );
uint8_t HexToDec(uint32_t *buf);    //16进制数换位为10进制对应的角度

#endif

