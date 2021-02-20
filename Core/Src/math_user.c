#include "math_user.h"
#include <math.h>
#include <stdio.h>

bool IsEqual( float f1, float f2, float eps )
{
    if ( fabs(f1-f2) < eps ) 
        return true;  
    else 
        return false;
}

float LittleEndingShortToFloat( uint8_t *buf, uint8_t scale )
{
    int16_t tmpdata = buf[0] + (buf[1]<<8);
    float ret = tmpdata/scale;
    return ret;
}

void FloatToLittleEndingShort( uint8_t *buf, float data, uint16_t scale )
{
    int16_t tmpdata = (int16_t) (data*scale);
    buf[0] = tmpdata & 0xFF;
    buf[1] = (tmpdata>>8) & 0xFF;

    return;
}

void PrintBuff( uint8_t *buf, uint8_t len )
{
    int i = 0;
    for( i=0; i<len; i++ )
    {
        printf( "0x%02x ", buf[i] );
    }
    printf("\n");
}
uint8_t HexToDec(uint32_t *buf)    //16进制数换位为10进制对应的角度
{
	uint8_t HEX_low_four,HEX_high_four;
	uint8_t re;
	HEX_low_four = *buf & 0x0f;
	HEX_high_four = (*buf & 0xf0)>>4;
	re = HEX_low_four + HEX_high_four*10;
	return re;
	//printf("%d ",re);
}
