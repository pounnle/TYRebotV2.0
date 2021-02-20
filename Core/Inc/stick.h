#ifndef __STICK_H  
#define __STICK_H  
#include "user.h"

typedef enum
{
	LEFT_DOWN = 0,
	RIGHT_DOWN,
	LEFT_RIGHT_DOWN,
	KEY_NUMS,
}KEY_ENUM;

typedef enum
{
	KEY_NONE_OPERATION = 0, //无操作
	KEY_SHORT_PRESS ,//点按
	KEY_LONG_PRESS , //长按 
		
}KEY_TYPE_ENUM;

typedef struct
{
	uint16_t KeyDownLastTime_ms ;  //按下时间计数
	KEY_TYPE_ENUM KeyType;
	uint8_t OutTimeFlag;
}StickComponent_t;

extern StickComponent_t KeyStatus[KEY_NUMS];
void KeyTypeJudge(void);
//#define KEY_UP 1
//#define KEY_DOWN 0
//#define GetLeftKeyStatus()   HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15)
//#define GetRightKeyStatus()  HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14)

//extern uint8_t Flag ;

//extern uint16_t KeyDownLastTime_ms ;  //按下时间计数
//extern uint16_t KeyUpLastTime_ms ;    //松开时间计数
//extern PRESS_TYPE_ENUM  KeyType;

//typedef struct
//{
//	uint16_t KeyDownLastTime_ms ;  //按下时间计数
//	uint16_t KeyUpLastTime_ms ;    //松开时间计数
//	uint8_t  NowKeyStatus ;      //现在的按键状态
//	uint8_t  OldKeyStatus ;      //上一个按键状态
//}StickComponent_t;

//typedef enum
//{
//	LEFT_KEY = 0,
//	RIGHT_KEY,
//	KEY_NUMS,
//}KEY_LIST;


//extern StickComponent_t KeyStatusBuff[KEY_NUMS];
#endif

