#include "stick.h"

StickComponent_t KeyStatus[KEY_NUMS];

void KeyTypeJudge(void)
{
	if(KeyStatus[LEFT_DOWN].KeyDownLastTime_ms>100&&KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms == 0)
	{
		//��߳���
		if(KeyStatus[LEFT_DOWN].KeyDownLastTime_ms>2000)
		{
//			printf("LEFT LONG\n");
			KeyStatus[LEFT_DOWN].OutTimeFlag = 1;
			KeyStatus[LEFT_DOWN].KeyDownLastTime_ms= 0;
		}
		//��ߵ㰴
		if(KeyStatus[LEFT_DOWN].KeyDownLastTime_ms<=2000&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == RESET)
		{
			//printf("LEFT SHORT\n");
			KeyStatus[LEFT_DOWN].KeyDownLastTime_ms= 0;
		}
	}
	else if(KeyStatus[LEFT_DOWN].KeyDownLastTime_ms<=100&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == RESET)
	{
		KeyStatus[LEFT_DOWN].KeyDownLastTime_ms = 0;
	}
	
	if(KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms>100&&KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms == 0)
	{
		//�ұ߳���
		if(KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms>2000)
		{
			SetExternalCtrlCmd( TO_TURN );
			printf("\nControl commnad is to turn round\n");
			//xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);        //��������Ϣ���־�¼�	
			KeyStatus[RIGHT_DOWN].OutTimeFlag = 1;
			KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms= 0;
		}
		//�ұߵ㰴
		if(KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms<=2000&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == RESET)
		{
			SetExternalCtrlCmd( TO_WALK );
			printf("\nControl commnad is to walk\n");
			//xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);        //��������Ϣ���־�¼�
			KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms= 0;
		}
	}		
	else if(KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms<=100&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == RESET)
	{
		KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms = 0;
	}
	
	if(KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms > 30)
	{
		//���ѳ���
		if(KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms>2000)
		{
			SetExternalCtrlCmd( TO_SITDOWN );
			printf("\nControl commnad is to sit down\n");
			//xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);        //��������Ϣ���־�¼�
			KeyStatus[LEFT_RIGHT_DOWN].OutTimeFlag = 1;
			KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms= 0;
			KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms= 0;
			KeyStatus[LEFT_DOWN].KeyDownLastTime_ms= 0;
			
		}
		//���ߵ㰴
		if(KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms<=2000&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == RESET&&HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == RESET)
		{
			SetExternalCtrlCmd( TO_ERECT );
			printf("\nControl commnad is to erect\n");
			//xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);        //��������Ϣ���־�¼�
			KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms= 0;
			KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms= 0;
			KeyStatus[LEFT_DOWN].KeyDownLastTime_ms= 0;
		}		
	}
}
