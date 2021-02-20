/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
#include "user.h"
#include "stdio.h"
#include "stick.h"
#include "adc.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern CAN_HandleTypeDef hcan1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim7;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles ADC1, ADC2 and ADC3 global interrupts.
  */
void ADC_IRQHandler(void)
{
  /* USER CODE BEGIN ADC_IRQn 0 */

  /* USER CODE END ADC_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC_IRQn 1 */

  /* USER CODE END ADC_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX0 interrupts.
  */
void CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

  /* USER CODE END CAN1_RX0_IRQn 0 */
  HAL_CAN_IRQHandler(&hcan1);
  /* USER CODE BEGIN CAN1_RX0_IRQn 1 */

  /* USER CODE END CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */
	if((HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == SET)&&(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == RESET))
	{
		if(KeyStatus[LEFT_DOWN].OutTimeFlag == 0)
		{
			KeyStatus[LEFT_DOWN].KeyDownLastTime_ms++;
		}
	}
	else
	{
		KeyStatus[LEFT_DOWN].OutTimeFlag = 0;
	}
	/*********/
	if((HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == SET)&&(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == RESET))
	{
		if(KeyStatus[RIGHT_DOWN].OutTimeFlag == 0)
		{
			KeyStatus[RIGHT_DOWN].KeyDownLastTime_ms++;
		}
	}
	else
	{
		KeyStatus[RIGHT_DOWN].OutTimeFlag = 0;
	}
	/**********/
	if((HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_15) == SET)&&(HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_14) == SET))
	{
		if(KeyStatus[LEFT_RIGHT_DOWN].OutTimeFlag == 0)
		{
			KeyStatus[LEFT_RIGHT_DOWN].KeyDownLastTime_ms++;
		}
	}
	else
	{
		KeyStatus[LEFT_RIGHT_DOWN].OutTimeFlag = 0;
	}
	
  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */
	static uint8_t RxCount_3 = 0;
	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_RXNE)!= RESET) // 接收中断：接收到数据
	{
		uint8_t data;	
		data=READ_REG(huart3.Instance->DR); // 读取数据
		if(RxCount_3==0) // 如果是重新接收到数据帧，开启串口空闲中断
		{
			__HAL_UART_CLEAR_FLAG(&huart3,UART_FLAG_IDLE); // 清除空闲中断标志
		  __HAL_UART_ENABLE_IT(&huart3,UART_IT_IDLE);     // 使能空闲中断	 
		}
		if(RxCount_3<UART3_RX_BUFFER_SIZE)    // 判断接收缓冲区未满
		{
			Uart3RxBuffer[RxCount_3]=data;  // 保存数据
			RxCount_3++;                // 增加接收字节数计数
		}
	}
	else	if(__HAL_UART_GET_FLAG(&huart3,UART_FLAG_IDLE)!= RESET) // 串口空闲中断
	{		                                 // 数据帧置位，标识接收到一个完整数据帧
//		if(Usart3RxBuffQueue != NULL)  //消息队列创建成功
//		{
//			BaseType_t xHigherPriorityTaskWoken;
//			xQueueSendFromISR(Usart3RxBuffQueue,Uart3RxBuffer,&xHigherPriorityTaskWoken);//向队列中发送数据
//			memset(Uart3RxBuffer,0,RxCount3);
			RxCount_3 = 0;
			__HAL_UART_CLEAR_FLAG(&huart3,UART_FLAG_IDLE); // 清除空闲中断标志
			__HAL_UART_DISABLE_IT(&huart3,UART_IT_IDLE);    // 关闭空闲中断
			UnpackRemoteCmd(Uart3RxBuffer,12);
//			Uart3RxCpltFlag=1;
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
//		}
//		else
//		{
//			
//		}
		
	}
  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */
	static uint16_t RxCount_4=0; 
	if(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_RXNE)!= RESET) // 接收中断：接收到数据
	{
		uint8_t data;
		   // 已接收到的字节数
		data=READ_REG(huart4.Instance->DR); // 读取数据
		if(RxCount_4==0) // 如果是重新接收到数据帧，开启串口空闲中断
		{
			__HAL_UART_CLEAR_FLAG(&huart4,UART_FLAG_IDLE); // 清除空闲中断标志
			__HAL_UART_ENABLE_IT(&huart4,UART_IT_IDLE);     // 使能空闲中断	 
		}
		if(RxCount_4<RX_MAX_COUNT)    // 判断接收缓冲区未满
		{
			Uart4RxBuffer[RxCount_4]=data;  // 保存数据
			RxCount_4++;                // 增加接收字节数计数
		}
	}
	else if(__HAL_UART_GET_FLAG(&huart4,UART_FLAG_IDLE)!= RESET) // 串口空闲中断
	{		                                 // 数据帧置位，标识接收到一个完整数据帧
//		if(Usart4RxBuffQueue != NULL)  //消息队列创建成功
//		{
//			BaseType_t xHigherPriorityTaskWoken;
//			xQueueSendFromISR(Usart4RxBuffQueue,Uart4RxBuffer,&xHigherPriorityTaskWoken);//向队列中发送数据
//			memset(Uart4RxBuffer,0,RxCount_4);
			__HAL_UART_CLEAR_FLAG(&huart4,UART_FLAG_IDLE); // 清除空闲中断标志
			__HAL_UART_DISABLE_IT(&huart4,UART_IT_IDLE);    // 关闭空闲中断
			RxCount_4 = 0;
			UnpackRemoteCmd(Uart4RxBuffer,12);
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);//如果需要的话进行一次任务切换
//		}
//		else
//		{
//			
//		}
		
	}
  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
	int MotorIndex;
    HAL_ADC_Start_IT(&hadc1); //定时器中断里面开启ADC中断转换，1s开启一次采集 
	if(CanStartRxFlag != 0)
	{
		for(MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
		{
			CanMotorAngles[MotorIndex].SpaceTimes++;
			if(CanMotorAngles[MotorIndex].SpaceTimes >= 5)
			{
				CanMotorAngles[MotorIndex].SpaceTimes = 5;
			}
		}
	}
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_IRQn 0 */

  /* USER CODE END TIM7_IRQn 0 */
  HAL_TIM_IRQHandler(&htim7);
  /* USER CODE BEGIN TIM7_IRQn 1 */

  /* USER CODE END TIM7_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
