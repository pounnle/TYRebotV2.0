/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user.h"
#include "stick.h"
#include "task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
QueueHandle_t Usart3RxBuffQueue;         //串口消息队列
QueueHandle_t Usart4RxBuffQueue;         //串口消息队列
SemaphoreHandle_t BinarySemaphore;	         //二值信号量句柄
EventGroupHandle_t EventGroupHandler;	   //事件标志组句柄
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId CanTxDataTaskHandle;
osThreadId UartDataProTaskHandle;
osThreadId CanDataProTaskHandle;
osThreadId GetVoltageTaskHandle;
osThreadId KeyScanTaskHandle;
osThreadId UpLoadDataTaskHandle;
osThreadId SystemCheckTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void CanTxDataPro(void const * argument);
void UartDataPro(void const * argument);
void CanDataPro(void const * argument);
void GetVoltagePro(void const * argument);
void KeyScanPro(void const * argument);
void UpLoadDataPro(void const * argument);
void SystemCheckPro(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
//	Can1RxBuffQueue = xQueueCreate(1,CAN_DATA_BUFFER_SIZE);    //创建消息CanBuffer,队列项长度是串口接收缓冲区长度
	Usart3RxBuffQueue = xQueueCreate(1,UART3_RX_BUFFER_SIZE);  //创建消息串口3Buffer,队列项长度是串口接收缓冲区长度
	Usart4RxBuffQueue = xQueueCreate(1,UART4_RX_BUFFER_SIZE);  //创建消息串口3Buffer,队列项长度是串口接收缓冲区长度
	EventGroupHandler = xEventGroupCreate();	 //创建事件标志组
	BinarySemaphore   = xSemaphoreCreateBinary();
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of CanTxDataTask */
  osThreadDef(CanTxDataTask, CanTxDataPro, osPriorityAboveNormal, 0, 256);
  CanTxDataTaskHandle = osThreadCreate(osThread(CanTxDataTask), NULL);

  /* definition and creation of UartDataProTask */
  osThreadDef(UartDataProTask, UartDataPro, osPriorityNormal, 0, 128);
  UartDataProTaskHandle = osThreadCreate(osThread(UartDataProTask), NULL);

  /* definition and creation of CanDataProTask */
  osThreadDef(CanDataProTask, CanDataPro, osPriorityNormal, 0, 128);
  CanDataProTaskHandle = osThreadCreate(osThread(CanDataProTask), NULL);

  /* definition and creation of GetVoltageTask */
  osThreadDef(GetVoltageTask, GetVoltagePro, osPriorityLow, 0, 128);
  GetVoltageTaskHandle = osThreadCreate(osThread(GetVoltageTask), NULL);

  /* definition and creation of KeyScanTask */
  osThreadDef(KeyScanTask, KeyScanPro, osPriorityHigh, 0, 128);
  KeyScanTaskHandle = osThreadCreate(osThread(KeyScanTask), NULL);

  /* definition and creation of UpLoadDataTask */
  osThreadDef(UpLoadDataTask, UpLoadDataPro, osPriorityLow, 0, 516);
  UpLoadDataTaskHandle = osThreadCreate(osThread(UpLoadDataTask), NULL);

  /* definition and creation of SystemCheckTask */
  osThreadDef(SystemCheckTask, SystemCheckPro, osPriorityRealtime, 0, 128);
  SystemCheckTaskHandle = osThreadCreate(osThread(SystemCheckTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_CanTxDataPro */
/**
  * @brief  Function implementing the CanTxDataTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_CanTxDataPro */
void CanTxDataPro(void const * argument)
{
  /* USER CODE BEGIN CanTxDataPro */
  /* Infinite loop */
  for(;;)
  {
	
	TransExternalCmdToInternalCmd();
	GetMotorObjectAngleAndSpeed();
	taskENTER_CRITICAL();           //进入临界区
	CtrlMotor();
	taskEXIT_CRITICAL();            //退出临界区
    osDelay(1);
  }
  /* USER CODE END CanTxDataPro */
}

/* USER CODE BEGIN Header_UartDataPro */
/**
* @brief Function implementing the UartDataProTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UartDataPro */
void UartDataPro(void const * argument)
{
  /* USER CODE BEGIN UartDataPro */
	uint8_t Uart3Data[UART3_RX_BUFFER_SIZE] = {0};
	uint8_t Uart4Data[UART4_RX_BUFFER_SIZE] = {0};
  /* Infinite loop */
  for(;;)
  {
	if(Usart3RxBuffQueue != NULL) 
	{
		if(xQueueReceive(Usart3RxBuffQueue,Uart3Data,1))   //队列创建成功
		{
			UnpackRemoteCmd(Uart3Data,UART3_RX_BUFFER_SIZE);
		}
	}
	if(Usart4RxBuffQueue != NULL) 
	{
		if(xQueueReceive(Usart4RxBuffQueue,Uart4Data,1))   //队列创建成功
		{
			UnpackRemoteCmd(Uart4Data,UART4_RX_BUFFER_SIZE);
		}
	}
    osDelay(5);
  }
  /* USER CODE END UartDataPro */
}

/* USER CODE BEGIN Header_CanDataPro */
/**
* @brief Function implementing the CanDataProTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CanDataPro */
void CanDataPro(void const * argument)
{
  /* USER CODE BEGIN CanDataPro */
  /* Infinite loop */
  BaseType_t err=pdFALSE;
  for(;;)
  {
	  if(BinarySemaphore!=NULL)
	  {
		err=xSemaphoreTake(BinarySemaphore,10);	//获取信号量
		if(err==pdTRUE)										//获取信号量成功
		{
			CANDataRxPro();
			GetAllMotorState();
			GetWalkPhase();
		}
	  }
	  else
	  {
		 osDelay(1);
	  }
  }
  /* USER CODE END CanDataPro */
}

/* USER CODE BEGIN Header_GetVoltagePro */
/**
* @brief Function implementing the GetVoltageTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_GetVoltagePro */
void GetVoltagePro(void const * argument)
{
  /* USER CODE BEGIN GetVoltagePro */
  /* Infinite loop */
  for(;;)
  {
	
	//GetBatteryVoltage();
	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_1);HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2);HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_5);
    osDelay(500);
  }
  /* USER CODE END GetVoltagePro */
}

/* USER CODE BEGIN Header_KeyScanPro */
/**
* @brief Function implementing the KeyScanTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_KeyScanPro */
void KeyScanPro(void const * argument)
{
  /* USER CODE BEGIN KeyScanPro */
  /* Infinite loop */
  for(;;)
  {
	KeyTypeJudge();
    osDelay(5);
  }
  /* USER CODE END KeyScanPro */
}

/* USER CODE BEGIN Header_UpLoadDataPro */
/**
* @brief Function implementing the UpLoadDataTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_UpLoadDataPro */
void UpLoadDataPro(void const * argument)
{
  /* USER CODE BEGIN UpLoadDataPro */
  /* Infinite loop */
  for(;;)
  {
	UpLoadAppDataPro();
    osDelay(1000);
  }
  /* USER CODE END UpLoadDataPro */
}

/* USER CODE BEGIN Header_SystemCheckPro */
/**
* @brief Function implementing the SystemCheckTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SystemCheckPro */
void SystemCheckPro(void const * argument)
{
  /* USER CODE BEGIN SystemCheckPro */
  /* Infinite loop */
  for(;;)
  {
		STATUS_ENUM SystemState;
		SystemState = CheckStatus();
		if(SystemState == STATUS_NORMAL)
		{
			//HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET);
			vTaskResume(CanTxDataTaskHandle);
			
		}
		else
		{
			//HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);
			NowWalkPhase_g = ERROR_WALK_PHASE;
			NextWalkPhase_g = ERROR_WALK_PHASE;
			LastWalkPhase_g = ERROR_WALK_PHASE;
			SetExternalCtrlCmd( TO_KEEP );
			vTaskSuspend(CanTxDataTaskHandle);
		}
		osDelay(10);
  }
  /* USER CODE END SystemCheckPro */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
