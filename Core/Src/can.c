/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "user.h"
#include "string.h"

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 6;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_7TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */
  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_Config(void)
{
  CAN_FilterTypeDef  sFilterConfig;

  /*##-2- Configure the CAN Filter ###########################################*/
  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;
  while (HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig) != HAL_OK) { }

  /*##-3- Start the CAN peripheral ###########################################*/
  while (HAL_CAN_Start(&hcan1) != HAL_OK) { }

  /*##-4- Activate CAN RX notification #######################################*/
  while (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) { }

}

//can1发送一组数据
//len:数据长度(最大为8)				     
//msg:数据指针,最大为8个字节.
//返回值:0,成功;
//		 其他,失败;
int32_t CAN1_SendMsg(uint16_t CANId, uint8_t* msg, uint8_t len)
{	
	uint32_t TxMailbox = 0;
    CAN_TxHeaderTypeDef	TxHeader;      //

    if( msg == NULL )
        return -1;

    if( len>8 )
        len = 8;

    TxHeader.StdId = CANId;        //标准标识符
    TxHeader.ExtId = 0x12;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = len;                
    TxHeader.TransmitGlobalTime = DISABLE;
    if(HAL_CAN_AddTxMessage(&hcan1, &TxHeader, msg, &TxMailbox) != HAL_OK)//发送
	{
		return -2;
	}

	while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 3) {}

    return 0;
}

//CAN1口接收数据
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	uint8_t RxData[8] = {0};
	CAN_RxHeaderTypeDef	RxHeader = {0};
	CanStartRxFlag = 1;
	BaseType_t xHigherPriorityTaskWoken;
	if(hcan->Instance == CAN1)
	{		
		HAL_CAN_GetRxMessage(&hcan1, CAN_FILTER_FIFO0, &RxHeader, RxData);
		switch(RxHeader.StdId)
		{
			case 0x302:
				CanMotorAngles[LEFT_KGJ].Id = RxHeader.StdId;
				CanMotorAngles[LEFT_KGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[LEFT_KGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[LEFT_KGJ].SpaceTimes = 0;
				break;
			case 0x303:
				CanMotorAngles[LEFT_XGJ].Id = RxHeader.StdId;
				CanMotorAngles[LEFT_XGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[LEFT_XGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[LEFT_XGJ].SpaceTimes = 0;
				break;				
			case 0x304:
				CanMotorAngles[RIGHT_KGJ].Id = RxHeader.StdId;
				CanMotorAngles[RIGHT_KGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[RIGHT_KGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[RIGHT_KGJ].SpaceTimes = 0;
				break;				
			case 0x305:
				CanMotorAngles[RIGHT_XGJ].Id = RxHeader.StdId;
				CanMotorAngles[RIGHT_XGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[RIGHT_XGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[RIGHT_XGJ].SpaceTimes = 0;
				break;
			case 0x402:
				CanMotorAngles[LEFT_KGJ].Id = RxHeader.StdId;
				CanMotorAngles[LEFT_KGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[LEFT_KGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[LEFT_KGJ].SpaceTimes = 0;
				break;
			case 0x403:
				CanMotorAngles[LEFT_XGJ].Id = RxHeader.StdId;
				CanMotorAngles[LEFT_XGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[LEFT_XGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[LEFT_XGJ].SpaceTimes = 0;
				break;
			case 0x404:
				CanMotorAngles[RIGHT_KGJ].Id = RxHeader.StdId;
				CanMotorAngles[RIGHT_KGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[RIGHT_KGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[RIGHT_KGJ].SpaceTimes = 0;
				break;
			case 0x405:
				CanMotorAngles[RIGHT_XGJ].Id = RxHeader.StdId;
				CanMotorAngles[RIGHT_XGJ].Len = RxHeader.DLC;
				memcpy(CanMotorAngles[RIGHT_XGJ].Data,RxData,RxHeader.DLC);
				CanMotorAngles[RIGHT_XGJ].SpaceTimes = 0;
				break;
			default :
				break ;
		}
		HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
		if(BinarySemaphore!=NULL)
		{
			xSemaphoreGiveFromISR(BinarySemaphore,&xHigherPriorityTaskWoken);	//释放二值信号量
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
			
		}
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
