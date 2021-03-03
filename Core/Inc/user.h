#ifndef __USER_H
#define __USER_H

#include "stm32f4xx_hal.h"
#include <main.h>
#include <string.h>
#include <stdio.h>
#include "can.h"
#include "cmsis_os.h"
/***************************************************************************
*    ���ܺ궨��    *
****************************************************************************/

/*��������С*/
#define ElementOfArray(array) ( sizeof(array)/sizeof(array[0]) )
	
/***************************************************************************
*  �û������궨��  *
****************************************************************************/
	
/*CAN ��ID����*/
#define MOTOR_CTRL_CAN_ID  0x180
#define MOTOR_BACK_CAN_ID  0x300
#define MOTOR_HEART_CAN_ID  0x700
#define MOTOR_RESPONSE_CAN_ID  0x200
#define MOTOR_SYNCHRO_CAN_ID 0x400
/*CAN ����������ݷŴ�ϵ��*/
#define MOTOR_BACK_DATA_SCALE 100
#define MotorBackDataScale 100

/*�Ƕȡ��ٶ���ֵ����*/
//#define SPEED_EPS 1.0
//#define ANGLE_EPS 2.0

/*************************************************
----------------------------------
        ����ؽڽǶȺ�����
----------------------------------
**************************************************/
//#define INIT_MAX_ANGLE_OF_KGJ 90
//#define INIT_MAX_ANGLE_OF_XGJ 90
//#define INIT_MAX_ANGLE_OF_WALK_KGJ 35      //����ʱ�Źؽڹؽ�Ĭ�����Ƕ�
//#define INIT_MAX_ANGLE_OF_WALK_XGJ 60       //����ʱϥ�ؽ�Ĭ�����Ƕ�
//#define INIT_MAX_ANGLE_OF_WALK_KB 20        //�粽�ŹؽڽǶ�
//#define INIT_MAX_ANGLE_OF_ERECTED 0         //ֱ���Ƕ�
//#define INIT_MAX_ANGLE_OF_KGJ_FORWARD 105		//����ʱ�Źؽ��˶��Ƕ�
//#define INIT_MAX_ANGLE_OF_XGJ_FORWARD 105 	//����ʱϥ�ؽ��˶��Ƕ�
//#define INIT_MAX_ANGLE_OF_LAY_XGJ  10       //���߷���ʱϥ�ؽڽǶ�
//#define ININT_ANGLE_LAY_TO_ERECTED_KGJ  30
//#define ININT_ANGLE_LAY_TO_ERECTED_XGJ  50

/*
-----------���µ�ֱ��
*/
/**************�Ƕ�****************/
#define SITDOWN_TO_ERECT_KGJ_ANGLE 0
#define SITDOWN_TO_ERECT_XGJ_ANGLE 0
#define SITDOWN_TO_ERECT_1_KGJ_ANGLE 105		
#define SITDOWN_TO_ERECT_1_XGJ_ANGLE 105 
/**************ʱ��****************/
#define SITDOWN_TO_ERECT_KGJ_TIMES 5.5
#define SITDOWN_TO_ERECT_XGJ_TIMES 6.5
#define SITDOWN_TO_ERECT_1_KGJ_TIMES 1.5		
#define SITDOWN_TO_ERECT_1_XGJ_TIMES 2.0

#define ERROR_TO_ERECT_KGJ_TIMES 2.0
#define ERROR_TO_ERECT_XGJ_TIMES 2.0

#define ERROR_TO_ERECT_KGJ_BIG_TIMES 4.0
#define ERROR_TO_ERECT_XGJ_BIG_TIMES 4.0
/*
-----------ֱ��������
*/
/**************�Ƕ�****************/
#define ERECT_TO_SITDOWN_KGJ_ANGLE 90
#define ERECT_TO_SITDOWN_XGJ_ANGLE 90

#define ERECT_TO_SITDOWN_1_KGJ_ANGLE 30
//#define ERECT_TO_SITDOWN_1_XGJ_ANGLE 90
/**************ʱ��****************/
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
/**************�Ƕ�****************/
#define STEP_FOOT_KGJ_ANGLE 18
#define STEP_FOOT_XGJ_ANGLE 10
//#define STEP_FOOT_1_KGJ_ANGLE 40
//#define STEP_FOOT_1_XGJ_ANGLE 60
#define STEP_FOOT_TO_ERECT_KGJ_ANGLE 0
#define STEP_FOOT_TO_ERECT_XGJ_ANGLE 0
#define STEP_FOOT_TO_ERECT_KGJ_1_ANGLE 30
#define STEP_FOOT_TO_ERECT_XGJ_1_ANGLE 50
#define STEP_FOOT_KB_KGJ_ANGLE 5
/**************ʱ��****************/

//#define STEP_FOOT_KGJ_TIMES 1.4
//#define STEP_FOOT_XGJ_TIMES 1.7
//#define STEP_FOOT_1_KGJ_TIMES 1.7
//#define STEP_FOOT_1_XGJ_TIMES 1.3
#define STEP_FOOT_TO_ERECT_KGJ_TIMES 1.5
#define STEP_FOOT_TO_ERECT_XGJ_TIMES 1.5

/*
**********ת��-----	
*/

#define TURNED_KGJ_ANGLE 25
#define TURNED_XGJ_ANGLE 30

#define TURNED_KGJ_TIMES 2
#define TURNED_XGJ_TIMES 2


/*�����*/
#define CMD_NULL                 0xFF
#define CMD_TO_SITDOWN 	         0x10
#define CMD_TO_ERECT_1           0x11
#define CMD_TO_ERECT_2           0x12
#define CMD_TO_ERECT_3           0x13		//�����ȵ�ֱ��
#define CMD_TO_ERECT_4           0x14		//�����ȵ�ֱ��
#define CMD_TO_ERECT_5           0x15		//�����ȵ�ֱ��
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

/*���ڻ���BUFF��С*/
#define UART4_RX_BUFFER_SIZE 12
#define UART3_RX_BUFFER_SIZE 12
#define RX_MAX_COUNT         12
#define WRITE_FLASH_ID_SIZE   8
#define WRITE_FLASH_DATA_SIZE 8

/***************************************************************************
*  ö�ٶ���  *
****************************************************************************/
/*CAN ��IDö��*/
enum MotorCanNodeId
{
    LEFT_KGJ_NODE_ID = 0x02,
    LEFT_XGJ_NODE_ID = 0x03,
    RIGHT_KGJ_NODE_ID = 0x04,
    RIGHT_XGJ_NODE_ID = 0x05,
};

/*������ö��*/
typedef enum
{
    LEFT_KGJ = 0, 
    RIGHT_KGJ = 1,
    LEFT_XGJ = 2, 
    RIGHT_XGJ = 3,
}MOTOR_POS_ENUM;   

/*����˶�״̬ö��*/
typedef enum     
{
	MOTOR_STOPPING = 0,
	MOTOR_MOVING = 1, 
}MOTOR_STATUS_ENUM;    

/*����ö��*/
typedef enum
{
    SITDOWNED = 0, 
	ERECTED,		//ֱ��
    LEFT_FOOT_STEP,  //������
    RIGHT_FOOT_STEP, //������
	TURNED,  //ת��
    MOVING,
	KEEPED,
    ERROR_WALK_PHASE,
}WALK_PHASE_ENUM;          // ����ö��

/*����ָ��ö��*/
typedef enum
{
    TO_KEEP = 0, //ά��ԭ״̬
    TO_WALK,
    TO_SITDOWN,
    TO_ERECT,
    TO_STEP_LEFT_FOOT,
    TO_STEP_RIGHT_FOOT,
	TO_TURN,
    MAX_CTRL_CMD,
}CTRL_CMD_ENUM;        //��������ö��

/*����ͼö��*/
typedef struct 
{
    WALK_PHASE_ENUM NowState;
    CTRL_CMD_ENUM   Cmd;
    WALK_PHASE_ENUM NextState;
    CTRL_CMD_ENUM   NextCmd;
    //������״̬������Ŀ��״̬���ŵ�ʱ�䣬��ȿ絥������ʱ��ı���
    float  ScaleOfSingleStepTime; 
}StateMachine_t;

/*��������ö��*/
typedef enum
{
    NONE_OPERATION = 0, //�޲���
    SHORT_PRESS = 0x0D,//�㰴
    LONG_PRESS = 0x0C, //���� 
}PRESS_TYPE_ENUM;


/*�ؽ�״̬ö��*/
typedef enum     
{
	STATUS_NORMAL = 0,
	STATUS_ERROR,
}STATUS_ENUM;


/*--------------------------------------*/
/*����״̬�ṹ��*/
typedef struct   
{
	
	uint8_t     StatusCode ;
	STATUS_ENUM OldsystemStauts;
	STATUS_ENUM NowsystemStauts;
	STATUS_ENUM AngleStatus;
	STATUS_ENUM CanStatus ;
	uint8_t     CanErrorTimes;
}JointStatus_t;



/*�����Ϣ�ṹ��*/
typedef struct 
{
    float MotorAngles[4];
    float MotorSpeeds[4];
	float MotorRunTimes[4];
	float MotorSynAngle[4];
	MOTOR_STATUS_ENUM MotorStatus[4];
}MotorData_t;

/*CAN�жϽ��սṹ��*/
typedef struct 
{
	uint16_t Id;
	uint8_t  Len;
	uint8_t  Data[8];
	uint16_t SpaceTimes;
}CANFrameData_t;

/*������Ƕȶ�Ӧ�ṹ��*/
typedef struct 
{
    WALK_PHASE_ENUM WalkPhase;
    float MotorAngles[4];
}MotorAnglesOfWalkPhase_t;

/*�ⲿָ��ṹ��*/
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
extern uint32_t WriteFlashBuffer[WRITE_FLASH_ID_SIZE];   //16����
extern uint32_t ReadFlashBuffer[WRITE_FLASH_DATA_SIZE] ;
extern EventGroupHandle_t EventGroupHandler;	//�¼���־����
extern QueueHandle_t Usart3RxBuffQueue;         //������Ϣ����
extern QueueHandle_t Usart4RxBuffQueue;         //������Ϣ����
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
STATUS_ENUM CheckStatus(void); //�Լ�״̬;
#endif
