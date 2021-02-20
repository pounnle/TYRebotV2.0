/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
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
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "user.h"
#include "stdio.h"
void UnpackRemoteCmd( uint8_t *buf, uint8_t len );
uint8_t Uart3RxBuffer[UART3_RX_BUFFER_SIZE] = {0};
uint8_t Uart4RxBuffer[UART4_RX_BUFFER_SIZE] = {0};
uint8_t Uart3RxCpltFlag = 0;
uint8_t Uart4RxCpltFlag = 0;

/* USER CODE END 0 */

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart3;

/* UART4 init function */
void MX_UART4_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */
    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**UART4 GPIO Configuration
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* UART4 interrupt Init */
    HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4)
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();

    /**UART4 GPIO Configuration
    PC10     ------> UART4_TX
    PC11     ------> UART4_RX
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

//�ض���fputc����������ʹ�printf���������
int fputc(int ch, FILE *f)
{ 	
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
	USART3->DR = (uint8_t) ch;      
	return ch;
}

//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if(huart->Instance==USART3)
//	{
//		UnpackRemoteCmd(Uart3RxBuffer,12);
//		HAL_UART_Receive_IT(&huart3 ,Uart3RxBuffer,12);
//	}
//	if(huart->Instance==UART4)
//	{
//		UnpackRemoteCmd(Uart4RxBuffer,12);
//		HAL_UART_Receive_IT(&huart4 ,Uart4RxBuffer,12);
//	}
//}
/***����3����
�ж�***/
///***����3�����ж�***/
//void USART3_IRQHandler(void)
//{
//	static uint8_t RxCount3 = 0;
//	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_RXNE)!= RESET) // �����жϣ����յ�����
//	{
//		uint8_t data;	
//		data=READ_REG(huart3.Instance->DR); // ��ȡ����
//		if(RxCount3==0) // ��������½��յ�����֡���������ڿ����ж�
//		{
//			__HAL_UART_CLEAR_FLAG(&huart3,UART_FLAG_IDLE); // ��������жϱ�־
//		  __HAL_UART_ENABLE_IT(&huart3,UART_IT_IDLE);     // ʹ�ܿ����ж�	 
//		}
//		if(RxCount3<UART3_RX_BUFFER_SIZE)    // �жϽ��ջ�����δ��
//		{
//			Uart3RxBuffer[RxCount3]=data;  // ��������
//			RxCount3++;                // ���ӽ����ֽ�������
//		}
//	}
//	else	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)!= RESET) // ���ڿ����ж�
//	{		                                 // ����֡��λ����ʶ���յ�һ����������֡
////		if(Usart3RxBuffQueue != NULL)  //��Ϣ���д����ɹ�
////		{
////			BaseType_t xHigherPriorityTaskWoken;
////			xQueueSendFromISR(Usart3RxBuffQueue,Uart3RxBuffer,&xHigherPriorityTaskWoken);//������з�������
////			memset(Uart3RxBuffer,0,RxCount3);
//			RxCount3 = 0;
//			__HAL_UART_CLEAR_FLAG(&huart3,UART_FLAG_IDLE); // ��������жϱ�־
//			__HAL_UART_DISABLE_IT(&huart3,UART_IT_IDLE);    // �رտ����ж�
//			UnpackRemoteCmd(Uart3RxBuffer,12);
//			Uart3RxCpltFlag=1;
////			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//�����Ҫ�Ļ�����һ�������л�
////		}
////		else
////		{
////			
////		}
//		
//	}

//}
///***����4�����ж�***/
//void UART4_IRQHandler(void)
//{
//	if(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_RXNE)!= RESET) // �����жϣ����յ�����
//	{
//		uint8_t data;
////		data=Uart4RxByte1[0]; // ��ȡ����	
//		data=READ_REG(huart4.Instance->DR); // ��ȡ����
//		if(RxCount==0) // ��������½��յ�����֡���������ڿ����ж�
//		{
//			__HAL_UART_CLEAR_FLAG(&huart4,UART_FLAG_IDLE); // ��������жϱ�־
//			__HAL_UART_ENABLE_IT(&huart4,UART_IT_IDLE);     // ʹ�ܿ����ж�	 
//		}
//		if(RxCount<RX_MAX_COUNT)    // �жϽ��ջ�����δ��
//		{
//			Uart4RxBuffer[RxCount]=data;  // ��������
//			RxCount++;                // ���ӽ����ֽ�������
//		}
//	}
//	else	if(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_IDLE)!= RESET) // ���ڿ����ж�
//	{		                                 // ����֡��λ����ʶ���յ�һ����������֡
////		if(Usart4RxBuffQueue != NULL)  //��Ϣ���д����ɹ�
////		{
////			BaseType_t xHigherPriorityTaskWoken;
////			xQueueSendFromISR(Usart4RxBuffQueue,Uart4RxBuffer,&xHigherPriorityTaskWoken);//������з�������
////			memset(Uart4RxBuffer,0,RxCount);
//			__HAL_UART_CLEAR_FLAG(&huart4,UART_FLAG_IDLE); // ��������жϱ�־
//			__HAL_UART_DISABLE_IT(&huart4,UART_IT_IDLE);    // �رտ����ж�
//			Uart4RxCpltFlag=1;
//			RxCount = 0;
//			UnpackRemoteCmd(Uart4RxBuffer,12);
////			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//�����Ҫ�Ļ�����һ�������л�
////		}
////		else
////		{
////			
////		}
//		
//	}

//}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
