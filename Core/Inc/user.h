#ifndef __USER_H
#define __USER_H

#include "stm32f4xx_hal.h"
#include <main.h>
#include <string.h>
#include <stdio.h>
#include "can.h"
#include "cmsis_os.h"
/***************************************************************************
*    功能宏定义    *
****************************************************************************/

/*算出数组大小*/
#define ElementOfArray(array) ( sizeof(array)/sizeof(array[0]) )
	
/***************************************************************************
*  用户参数宏定义  *
****************************************************************************/
	
/*CAN 基ID配置*/
#define MOTOR_CTRL_CAN_ID  0x180
#define MOTOR_BACK_CAN_ID  0x300
#define MOTOR_HEART_CAN_ID  0x700
#define MOTOR_RESPONSE_CAN_ID  0x200
#define MOTOR_SYNCHRO_CAN_ID 0x400
/*CAN 电机返回数据放大系数*/
#define MOTOR_BACK_DATA_SCALE 100
#define MotorBackDataScale 100

/*角度、速度阈值配置*/
//#define SPEED_EPS 1.0
//#define ANGLE_EPS 2.0

/*************************************************
----------------------------------
        步相关节角度宏配置
----------------------------------
**************************************************/
//#define INIT_MAX_ANGLE_OF_KGJ 90
//#define INIT_MAX_ANGLE_OF_XGJ 90
//#define INIT_MAX_ANGLE_OF_WALK_KGJ 35      //行走时髋关节关节默认最大角度
//#define INIT_MAX_ANGLE_OF_WALK_XGJ 60       //行走时膝关节默认最大角度
//#define INIT_MAX_ANGLE_OF_WALK_KB 20        //跨步髋关节角度
//#define INIT_MAX_ANGLE_OF_ERECTED 0         //直立角度
//#define INIT_MAX_ANGLE_OF_KGJ_FORWARD 105		//起立时髋关节运动角度
//#define INIT_MAX_ANGLE_OF_XGJ_FORWARD 105 	//起立时膝关节运动角度
//#define INIT_MAX_ANGLE_OF_LAY_XGJ  10       //行走放腿时膝关节角度
//#define ININT_ANGLE_LAY_TO_ERECTED_KGJ  30
//#define ININT_ANGLE_LAY_TO_ERECTED_XGJ  50

/*
-----------坐下到直立
*/
/**************角度****************/
#define SITDOWN_TO_ERECT_KGJ_ANGLE 0
#define SITDOWN_TO_ERECT_XGJ_ANGLE 0
#define SITDOWN_TO_ERECT_1_KGJ_ANGLE 105		
#define SITDOWN_TO_ERECT_1_XGJ_ANGLE 105 
/**************时间****************/
#define SITDOWN_TO_ERECT_KGJ_TIMES 5.5
#define SITDOWN_TO_ERECT_XGJ_TIMES 6.5
#define SITDOWN_TO_ERECT_1_KGJ_TIMES 1.5		
#define SITDOWN_TO_ERECT_1_XGJ_TIMES 2.0

#define ERROR_TO_ERECT_KGJ_TIMES 2.0
#define ERROR_TO_ERECT_XGJ_TIMES 2.0

#define ERROR_TO_ERECT_KGJ_BIG_TIMES 4.0
#define ERROR_TO_ERECT_XGJ_BIG_TIMES 4.0
/*
-----------直立到坐下
*/
/**************角度****************/
#define ERECT_TO_SITDOWN_KGJ_ANGLE 90
#define ERECT_TO_SITDOWN_XGJ_ANGLE 90

#define ERECT_TO_SITDOWN_1_KGJ_ANGLE 30
//#define ERECT_TO_SITDOWN_1_XGJ_ANGLE 90
/**************时间****************/
#define ERECT_TO_SITDOWN_KGJ_TIMES 5.5
#define ERECT_TO_SITDOWN_XGJ_TIMES 5.5

#define ERECT_TO_SITDOWN_KGJ_1_TIMES 3
//#define ERECT_TO_SITDOWN_XGJ_1_TIMES 2

#define ERROR_TO_SITDOWN_KGJ_TIMES 2.0
#define ERROR_TO_SITDOWN_XGJ_TIMES 2.0
#define ERROR_TO_SITDOWN_KGJ_BIG_TIMES 4.0
#define ERROR_TO_SITDOWN_XGJ_BIG_TIMES 4.0
/*
-----------WALK
*/
/**************角度****************/
#define STEP_FOOT_KGJ_ANGLE 18
#define STEP_FOOT_XGJ_ANGLE 10
//#define STEP_FOOT_1_KGJ_ANGLE 40
//#define STEP_FOOT_1_XGJ_ANGLE 60
#define STEP_FOOT_TO_ERECT_KGJ_ANGLE 0
#define STEP_FOOT_TO_ERECT_XGJ_ANGLE 0
#define STEP_FOOT_TO_ERECT_KGJ_1_ANGLE 30
#define STEP_FOOT_TO_ERECT_XGJ_1_ANGLE 50
#define STEP_FOOT_KB_KGJ_ANGLE 5
/**************时间****************/

//#define STEP_FOOT_KGJ_TIMES 1.4
//#define STEP_FOOT_XGJ_TIMES 1.7
//#define STEP_FOOT_1_KGJ_TIMES 1.7
//#define STEP_FOOT_1_XGJ_TIMES 1.3
#define STEP_FOOT_TO_ERECT_KGJ_TIMES 1.5
#define STEP_FOOT_TO_ERECT_XGJ_TIMES 1.5

/*
**********转弯-----	
*/

#define TURNED_KGJ_ANGLE 25
#define TURNED_XGJ_ANGLE 30

#define TURNED_KGJ_TIMES 2
#define TURNED_XGJ_TIMES 2


/*命令宏*/
#define CMD_NULL                 0xFF
#define CMD_TO_SITDOWN 	         0x10
#define CMD_TO_ERECT_1           0x11
#define CMD_TO_ERECT_2           0x12
#define CMD_TO_ERECT_3           0x13		//迈左腿到直立
#define CMD_TO_ERECT_4           0x14		//迈右腿到直立
#define CMD_TO_ERECT_5           0x15		//迈右腿到直立
#define CMD_TO_ERECT             0x20

#define CMD_TO_STEP_LEFT_FOOT    0x30
#define CMD_TO_STEP_LEFT_FOOT_1  0x31
#define CMD_TO_STEP_LEFT_FOOT_2  0x32
#define CMD_TO_STEP_LEFT_FOOT_3  0x33
#define CMD_TO_STEP_RIGHT_FOOT   0x40
#define CMD_TO_STEP_RIGHT_FOOT_1 0x41
#define CMD_TO_STEP_RIGHT_FOOT_2 0x42
#define CMD_TO_STEP_RIGHT_FOOT_3 0x43
#define CMD_TO_TURNED            0x50

/*串口缓存BUFF大小*/
#define UART4_RX_BUFFER_SIZE 12
#define UART3_RX_BUFFER_SIZE 12
#define RX_MAX_COUNT         12
#define WRITE_FLASH_ID_SIZE   8
#define WRITE_FLASH_DATA_SIZE 8

/***************************************************************************
*  枚举定义  *
****************************************************************************/
/*CAN 子ID枚举*/
enum MotorCanNodeId
{
    LEFT_KGJ_NODE_ID = 0x02,
    LEFT_XGJ_NODE_ID = 0x03,
    RIGHT_KGJ_NODE_ID = 0x04,
    RIGHT_XGJ_NODE_ID = 0x05,
};

/*电机编号枚举*/
typedef enum
{
    LEFT_KGJ = 0, 
    RIGHT_KGJ = 1,
    LEFT_XGJ = 2, 
    RIGHT_XGJ = 3,
}MOTOR_POS_ENUM;   

/*电机运动状态枚举*/
typedef enum     
{
	MOTOR_STOPPING = 0,
	MOTOR_MOVING = 1, 
}MOTOR_STATUS_ENUM;    

/*步相枚举*/
typedef enum
{
    SITDOWNED = 0, 
	ERECTED,		//直立
    LEFT_FOOT_STEP,  //迈左腿
    RIGHT_FOOT_STEP, //迈右腿
	TURNED,  //转弯
    MOVING,
	KEEPED,
    ERROR_WALK_PHASE,
}WALK_PHASE_ENUM;          // 步相枚举

/*控制指令枚举*/
typedef enum
{
    TO_KEEP = 0, //维持原状态
    TO_WALK,
    TO_SITDOWN,
    TO_ERECT,
    TO_STEP_LEFT_FOOT,
    TO_STEP_RIGHT_FOOT,
	TO_TURN,
    MAX_CTRL_CMD,
}CTRL_CMD_ENUM;        //控制命令枚举

/*步相图枚举*/
typedef struct 
{
    WALK_PHASE_ENUM NowState;
    CTRL_CMD_ENUM   Cmd;
    WALK_PHASE_ENUM NextState;
    CTRL_CMD_ENUM   NextCmd;
    //从现在状态动作到目标状态安排的时间，相比跨单步所用时间的比例
    float  ScaleOfSingleStepTime; 
}StateMachine_t;

/*按键类型枚举*/
typedef enum
{
    NONE_OPERATION = 0, //无操作
    SHORT_PRESS = 0x0D,//点按
    LONG_PRESS = 0x0C, //长按 
}PRESS_TYPE_ENUM;


/*关节状态枚举*/
typedef enum     
{
	STATUS_NORMAL = 0,
	STATUS_ERROR,
}STATUS_ENUM;


/*--------------------------------------*/
/*错误状态结构体*/
typedef struct   
{
	
	uint8_t     StatusCode ;
	STATUS_ENUM OldsystemStauts;
	STATUS_ENUM NowsystemStauts;
	STATUS_ENUM AngleStatus;
	STATUS_ENUM CanStatus ;
	uint8_t     CanErrorTimes;
}JointStatus_t;



/*电机信息结构体*/
typedef struct 
{
    float MotorAngles[4];
    float MotorSpeeds[4];
	float MotorRunTimes[4];
	float MotorSynAngle[4];
	MOTOR_STATUS_ENUM MotorStatus[4];
}MotorData_t;

/*CAN中断接收结构体*/
typedef struct 
{
	uint16_t Id;
	uint8_t  Len;
	uint8_t  Data[8];
	uint16_t SpaceTimes;
}CANFrameData_t;

/*步相与角度对应结构体*/
typedef struct 
{
    WALK_PHASE_ENUM WalkPhase;
    float MotorAngles[4];
}MotorAnglesOfWalkPhase_t;

/*外部指令结构体*/
typedef struct
{
    CTRL_CMD_ENUM Cmd;
    uint16_t escapeTime;
}CtrlCmd_t;



extern uint8_t Uart3RxBuffer[UART3_RX_BUFFER_SIZE];
extern uint8_t Uart4RxBuffer[UART4_RX_BUFFER_SIZE];
extern CANFrameData_t CanMotorAngles[4];
extern volatile uint16_t ADC1Value;
extern uint8_t CanStartRxFlag;
extern WALK_PHASE_ENUM LastWalkPhase_g ;
extern WALK_PHASE_ENUM NowWalkPhase_g ;
extern WALK_PHASE_ENUM NextWalkPhase_g ;
extern uint32_t WriteFlashBuffer[WRITE_FLASH_ID_SIZE];   //16进制
extern uint32_t ReadFlashBuffer[WRITE_FLASH_DATA_SIZE] ;
extern EventGroupHandle_t EventGroupHandler;	//事件标志组句柄
extern QueueHandle_t Usart3RxBuffQueue;         //串口消息队列
extern QueueHandle_t Usart4RxBuffQueue;         //串口消息队列
extern SemaphoreHandle_t BinarySemaphore;	   
void inituser(void);
void SetExternalCtrlCmd( CTRL_CMD_ENUM Cmd );
void UserInit( void );
void TransExternalCmdToInternalCmd(void);
void CtrlMotor(void);
void GetMotorObjectAngleAndSpeed(void);
void GetWalkPhase(void);
void UnpackRemoteCmd( uint8_t *buf, uint8_t len );
void GetAllMotorState(void);
void CompareTargetAngle(void);
void UpLoadAppDataPro(void);
void GetBatteryVoltage(void);
//void CANDataRxPro(void);
STATUS_ENUM CheckStatus(void); //自检状态;
#endif
