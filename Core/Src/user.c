#include "user.h"
#include "math_user.h"
#include "stick.h"
#include "usart.h"
#include "flash.h"
#include "math.h"
float STEP_FOOT_KGJ_TIMES=1.3;//2.4;//1.5;//1.3; //1.55  //1 KUAI
float STEP_FOOT_XGJ_TIMES=2.0;//3.6;//2.75;//1.7;  //1.95 //1.4
float STEP_FOOT_1_KGJ_TIMES=2.1;//4.0;//2.50;//1.7;//1.95 //1.4
float STEP_FOOT_1_XGJ_TIMES =1.3;//;//1.25;//1.3;//1.55//1
float STEP_FOOT_1_KGJ_ANGLE =40.0;
float STEP_FOOT_1_XGJ_ANGLE =60;
float SPEED_EPS =10.0;
float ANGLE_EPS =4.0;


/*全局变量*/
volatile MotorData_t NowMotorData;
volatile MotorData_t ObjetMotorData;
volatile CtrlCmd_t ExternalCtrlCmd_g;
volatile JointStatus_t 	JointStatus[4];
CANFrameData_t CanMotorAngles[4];
WALK_PHASE_ENUM LastWalkPhase_g = ERROR_WALK_PHASE;
WALK_PHASE_ENUM NowWalkPhase_g = ERROR_WALK_PHASE;
WALK_PHASE_ENUM NextWalkPhase_g = ERROR_WALK_PHASE ;
uint8_t  UploadMotorDataLeft[14] = {0};  //上传左腿电机数据
uint8_t  UploadMotorDataRight[14]= {0};  //上传右腿点击数据
uint8_t  UploadCellData[14] = {0};  //上传右腿点击数据
uint32_t WriteFlashBuffer[WRITE_FLASH_ID_SIZE]= {0};   //16进制
uint32_t ReadFlashBuffer[WRITE_FLASH_DATA_SIZE] = {0};
uint8_t CmdFalg = CMD_NULL;
uint8_t OldCmd = 0xff;
uint8_t CanStartRxFlag = 0;


/**********************************************
*步相与角度的对应关系
{+，-}
{-，+}
**********************************************/
MotorAnglesOfWalkPhase_t Maps_MotorAnglesOfWalkPhase[] = 
{
    { SITDOWNED,       { ERECT_TO_SITDOWN_KGJ_ANGLE,     -ERECT_TO_SITDOWN_KGJ_ANGLE,                            
                         -ERECT_TO_SITDOWN_XGJ_ANGLE,     ERECT_TO_SITDOWN_XGJ_ANGLE } },
																											
    { ERECTED,         { SITDOWN_TO_ERECT_XGJ_ANGLE,  SITDOWN_TO_ERECT_KGJ_ANGLE,  
                         SITDOWN_TO_ERECT_XGJ_ANGLE,  SITDOWN_TO_ERECT_XGJ_ANGLE } },
																											
    { LEFT_FOOT_STEP,  { STEP_FOOT_KGJ_ANGLE,  SITDOWN_TO_ERECT_KGJ_ANGLE,  
                         -STEP_FOOT_XGJ_ANGLE, SITDOWN_TO_ERECT_XGJ_ANGLE } },
												
    { RIGHT_FOOT_STEP, { -SITDOWN_TO_ERECT_KGJ_ANGLE,  -STEP_FOOT_KGJ_ANGLE,  
                         SITDOWN_TO_ERECT_XGJ_ANGLE,  STEP_FOOT_XGJ_ANGLE } },  
	
	{ TURNED,          { TURNED_KGJ_ANGLE,  -SITDOWN_TO_ERECT_KGJ_ANGLE,  
                         -TURNED_XGJ_ANGLE,   SITDOWN_TO_ERECT_KGJ_ANGLE } },
};


const uint16_t CanIdsOfMotorBackData[] =
{
    MOTOR_BACK_CAN_ID + LEFT_KGJ_NODE_ID  ,
    MOTOR_BACK_CAN_ID + RIGHT_KGJ_NODE_ID ,
    MOTOR_BACK_CAN_ID + LEFT_XGJ_NODE_ID  ,
    MOTOR_BACK_CAN_ID + RIGHT_XGJ_NODE_ID ,
};

const uint16_t CanIdsOfMotorBackSyncho[] =
{
    MOTOR_SYNCHRO_CAN_ID + LEFT_KGJ_NODE_ID  ,
    MOTOR_SYNCHRO_CAN_ID + RIGHT_KGJ_NODE_ID ,
    MOTOR_SYNCHRO_CAN_ID + LEFT_XGJ_NODE_ID  ,
    MOTOR_SYNCHRO_CAN_ID + RIGHT_XGJ_NODE_ID ,
};
const uint16_t CanIdsOfCtrlMotor[] =
{
    MOTOR_CTRL_CAN_ID + LEFT_KGJ_NODE_ID  ,
    MOTOR_CTRL_CAN_ID + RIGHT_KGJ_NODE_ID ,
    MOTOR_CTRL_CAN_ID + LEFT_XGJ_NODE_ID  ,
    MOTOR_CTRL_CAN_ID + RIGHT_XGJ_NODE_ID ,
};

char * MotorPosStrs[] = 
{
    "left  kgj motor", 
    "right kgj motor", 
    "left  xgj motor", 
    "right xgj motor", 
};



/*函数声明*/

bool IsAllMotorStop( void );




void inituser(void)
{
	GetAllMotorState();
	GetWalkPhase();
	KeyTypeJudge();
	TransExternalCmdToInternalCmd();
	GetMotorObjectAngleAndSpeed();
	CtrlMotor();
	HAL_Delay(20);
}
void UserInit( void )
{
    int i = 0;

    NowWalkPhase_g = ERROR_WALK_PHASE;
    NextWalkPhase_g = ERROR_WALK_PHASE;

    //初始化全局变量
    SetExternalCtrlCmd( TO_KEEP );
    for( i=0; i<ElementOfArray(NowMotorData.MotorAngles); i++ )
    { 
        NowMotorData.MotorAngles[i] = 0.0;
    }
    for( i=0; i<ElementOfArray(NowMotorData.MotorSpeeds); i++ )
    { 
        NowMotorData.MotorSpeeds[i] = 0.0;
    }
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6,GPIO_PIN_SET); //蓝牙模块透传模式
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15,GPIO_PIN_RESET);
//	SetInitDataFlash();
//	PrintfUserFlash();
}

/*该函数用于获取电机的实时数据*/
void GetAllMotorState(void)
{
	static uint8_t Rxtimes= 0 ;
    int MotorIndex;
    for (MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
    {				
        if ((CanMotorAngles[MotorIndex].Len == 8) && (CanIdsOfMotorBackData[MotorIndex] == CanMotorAngles[MotorIndex].Id))
        {
			Rxtimes++;
			if(Rxtimes >= 40)   //放弃前40次接收得数据
			{
				Rxtimes = 100;
				NowMotorData.MotorAngles[MotorIndex] =
						LittleEndingShortToFloat((uint8_t *)CanMotorAngles[MotorIndex].Data, MOTOR_BACK_DATA_SCALE);  //计算出电机当前的角度信息
				NowMotorData.MotorSpeeds[MotorIndex] =
						LittleEndingShortToFloat((uint8_t *)CanMotorAngles[MotorIndex].Data+2, MOTOR_BACK_DATA_SCALE);  //计算出电机当前的速度信息
			}

		}
		if ((CanMotorAngles[MotorIndex].Len == 8) && (CanIdsOfMotorBackSyncho[MotorIndex] == CanMotorAngles[MotorIndex].Id))
		{
			NowMotorData.MotorSynAngle[MotorIndex] = LittleEndingShortToFloat((uint8_t *)CanMotorAngles[MotorIndex].Data, MOTOR_BACK_DATA_SCALE);
		}
    }
}

/*该函数用于判断电机是否停止*/
bool IsAllMotorStop( void )         
{
    bool ret = true;
    int i = 0; 
    for( i=0; i<ElementOfArray(NowMotorData.MotorSpeeds); i++ )
    {
        if( IsEqual( NowMotorData.MotorSpeeds[i], 0.0, SPEED_EPS ) )
        {
            NowMotorData.MotorStatus[i] = MOTOR_STOPPING;
        }
		else
		{
			NowMotorData.MotorStatus[i] = MOTOR_MOVING;
		}
    }
    for( i=0; i<ElementOfArray(NowMotorData.MotorSpeeds); i++ )
    {
        if( NowMotorData.MotorStatus[i] == MOTOR_MOVING )
        {
            ret = false;
            break;
        }
    }
    return ret;
}

/*该函数用于获取实时步相*/
void GetWalkPhase(void)
{
    WALK_PHASE_ENUM WalkPhase = SITDOWNED;
    int i = 0;
    bool hasNotEqual = false;
	if(NextWalkPhase_g == ERROR_WALK_PHASE)   //开机
	{
        NowWalkPhase_g = ERROR_WALK_PHASE;
		return;
    }
	if( !IsAllMotorStop() )
	{
		NowWalkPhase_g = MOVING;
		return ;
    }
    for( WalkPhase=SITDOWNED; WalkPhase<MOVING; WalkPhase++ )
    {
        hasNotEqual = false;		
        for( i=0; i<ElementOfArray(NowMotorData.MotorAngles); i++ )
        {         
           if( !IsEqual( NowMotorData.MotorAngles[i], Maps_MotorAnglesOfWalkPhase[WalkPhase].MotorAngles[i], ANGLE_EPS ) )
           {
               hasNotEqual = true;
               break;
           }
        }
        //没有不相等的，即全部相等，则处于该步相
        if( !hasNotEqual )
        {
            NowWalkPhase_g =  WalkPhase;
			LastWalkPhase_g = NowWalkPhase_g;
			if(NowWalkPhase_g == NextWalkPhase_g)
			{
				NextWalkPhase_g = KEEPED;
			}
				return;
        }
    } 
    NowWalkPhase_g = ERROR_WALK_PHASE;
	
    return;
}

void GetMotorObjectAngleAndSpeed(void)
{
	if(NextWalkPhase_g ==  KEEPED)
	{
		CmdFalg = CMD_NULL;
		return  ;
	}		
	switch((uint8_t)LastWalkPhase_g)
	{		
/*****************当前步相为 迈左腿***************************/
		case LEFT_FOOT_STEP:
			if(NextWalkPhase_g ==  ERECTED)  //命令是 直立
			{
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_ERECT_3;
						break;					
					case CMD_TO_ERECT_3:
						if( NowMotorData.MotorAngles[LEFT_KGJ] >  (STEP_FOOT_TO_ERECT_KGJ_1_ANGLE - ANGLE_EPS) && 
							NowMotorData.MotorAngles[LEFT_XGJ] < -(STEP_FOOT_TO_ERECT_XGJ_1_ANGLE - ANGLE_EPS) &&  
							IsAllMotorStop())
						{
							CmdFalg = CMD_TO_ERECT;
						}
						break;
					case CMD_TO_ERECT:
						if(NowWalkPhase_g == ERECTED)
						{
							CmdFalg = CMD_NULL;
						}
						break;
					default :
						break ;
					
				}
			}
			if(NextWalkPhase_g == RIGHT_FOOT_STEP)  //命令是 迈右腿
			{
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_STEP_RIGHT_FOOT_1;
						break;
					case CMD_TO_STEP_RIGHT_FOOT_1:
						if(NowMotorData.MotorAngles[RIGHT_XGJ] > (STEP_FOOT_1_XGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[RIGHT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_RIGHT_FOOT_2;
						}
						break;
					case CMD_TO_STEP_RIGHT_FOOT_2:
						if(NowMotorData.MotorAngles[RIGHT_KGJ] < -(STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[RIGHT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_RIGHT_FOOT;
						}
					case CMD_TO_STEP_RIGHT_FOOT:
						if(NowWalkPhase_g == RIGHT_FOOT_STEP)
						{
							CmdFalg = CMD_NULL;
						}
					break;
					default:
						break;
				}
			}
			break;	
/*******************当前步相为 迈右腿******************************/
		case RIGHT_FOOT_STEP:
			if(NextWalkPhase_g ==  ERECTED)  //命令是 直立
			{
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_ERECT_4;
						break;
					case CMD_TO_ERECT_4:
						if( NowMotorData.MotorAngles[RIGHT_KGJ] <-(STEP_FOOT_TO_ERECT_KGJ_1_ANGLE - ANGLE_EPS) && 
							NowMotorData.MotorAngles[RIGHT_XGJ] > (STEP_FOOT_TO_ERECT_XGJ_1_ANGLE - ANGLE_EPS) &&  
							IsAllMotorStop())
						{
							CmdFalg = CMD_TO_ERECT;
						}
						break;
					case CMD_TO_ERECT:
						if(NowWalkPhase_g == ERECTED)
						{
							CmdFalg = CMD_NULL;
						}
						break;
					default :
						break ;
					
				}
			}
			if(NextWalkPhase_g == LEFT_FOOT_STEP)  //命令是 迈左腿
			{
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_STEP_LEFT_FOOT_1;
						break;
					case CMD_TO_STEP_LEFT_FOOT_1:
						if( NowMotorData.MotorAngles[LEFT_XGJ] < -(STEP_FOOT_1_XGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[LEFT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT_2;
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT_2:
						if( NowMotorData.MotorAngles[LEFT_KGJ] > (STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) &&NowMotorData.MotorStatus[LEFT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT;
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT:
						if(NowWalkPhase_g == LEFT_FOOT_STEP)
						{
							CmdFalg = CMD_NULL;
						}
						break;
					default :
						break;
				}				
			}
			break;			
/**********************当前步相为直立************************/
		case ERECTED:
			if(NextWalkPhase_g ==  SITDOWNED)  //命令是 坐下
			{
				switch(CmdFalg)
				{
					case CMD_NULL:	
						CmdFalg = CMD_TO_ERECT_5;
					break;
					case CMD_TO_ERECT_5:
						if(	NowMotorData.MotorAngles[LEFT_KGJ] > (ERECT_TO_SITDOWN_1_KGJ_ANGLE - ANGLE_EPS)&&
							NowMotorData.MotorAngles[RIGHT_KGJ] < -(ERECT_TO_SITDOWN_1_KGJ_ANGLE - ANGLE_EPS)&&
							IsAllMotorStop())
						{
							osDelay(500);
							CmdFalg = CMD_TO_SITDOWN;
						}
						break ;
					case CMD_TO_SITDOWN:	
						if(NowWalkPhase_g == SITDOWNED)
						{
							CmdFalg = CMD_NULL;
						}
					break;
					default:
						break;
				}
			}
			if(NextWalkPhase_g == LEFT_FOOT_STEP)  //命令是 迈左腿
			{					
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_STEP_LEFT_FOOT_1;
						break;					
					case CMD_TO_STEP_LEFT_FOOT_1:
						if( NowMotorData.MotorAngles[LEFT_XGJ] < -(STEP_FOOT_1_XGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[LEFT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT_2;
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT_2:
						if( NowMotorData.MotorAngles[LEFT_KGJ] > (STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) &&NowMotorData.MotorStatus[LEFT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT;
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT:
						if( NowWalkPhase_g == LEFT_FOOT_STEP)
						{
							CmdFalg = CMD_NULL;
						}			
					break;
					default :
						break;
				}
			}
			if(NextWalkPhase_g ==  TURNED)
			{
				CmdFalg = CMD_TO_TURNED;
			}
			break;
/**************************当前步相为 坐下****************************/
		case SITDOWNED:
			if(NextWalkPhase_g ==  ERECTED)  //命令是 直立
			{
				switch(CmdFalg)
				{
					case CMD_NULL:
						CmdFalg = CMD_TO_ERECT_1;
						break;
					case CMD_TO_ERECT_1:
						if(  NowMotorData.MotorAngles[LEFT_XGJ] <-(SITDOWN_TO_ERECT_1_XGJ_ANGLE - ANGLE_EPS) && 
							 NowMotorData.MotorAngles[RIGHT_XGJ]> (SITDOWN_TO_ERECT_1_XGJ_ANGLE - ANGLE_EPS) &&  
							IsAllMotorStop()      )   //弯腰收腿是否到位
						{
							osDelay(500);
							CmdFalg = CMD_TO_ERECT_2;	
						}
						break;
					case CMD_TO_ERECT_2:
						if(NowMotorData.MotorAngles[LEFT_KGJ] > (SITDOWN_TO_ERECT_1_KGJ_ANGLE - ANGLE_EPS) && 
							 NowMotorData.MotorAngles[RIGHT_KGJ]<-(SITDOWN_TO_ERECT_1_KGJ_ANGLE - ANGLE_EPS) &&
							 NowMotorData.MotorStatus[LEFT_KGJ] == MOTOR_STOPPING &&
							 NowMotorData.MotorStatus[RIGHT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_ERECT;
						}							
						break ;
					case CMD_TO_ERECT:
						if(NowWalkPhase_g == ERECTED)
						{
							CmdFalg = CMD_NULL;
						}
					break;
					default:
						break;
				}
			}
			break;
		case TURNED:
			if(NextWalkPhase_g ==  ERECTED)  //命令是 直立
			{
				CmdFalg = CMD_TO_ERECT;
			}
			break;
/***************当前步相为 非法步相****************************/
		case ERROR_WALK_PHASE:
			if(NextWalkPhase_g ==  ERECTED)  //命令是 直立
			{
				if(NowMotorData.MotorAngles[LEFT_KGJ] <= 45 &&     //姿态接近站立时有效
				   NowMotorData.MotorAngles[RIGHT_KGJ] >= -45 )
				{
					CmdFalg = CMD_TO_ERECT;
				}
				else
				{
					CmdFalg = CMD_NULL;
				}
			}
			if(NextWalkPhase_g == SITDOWNED)  //命令是 坐下
			{
				if(NowMotorData.MotorAngles[LEFT_KGJ] > 45 &&    //姿态接近坐下时有效
				   NowMotorData.MotorAngles[RIGHT_KGJ] < -45 )
				{
					CmdFalg = CMD_TO_SITDOWN;
				}
				else
				{
					CmdFalg = CMD_NULL;
				}
			}
			break;
		
	    /*NULL*/
		default :
			break;
	}
//	int MotorIndex = 0;
	if(OldCmd != CmdFalg)
	{
		switch(CmdFalg)
		{
			case CMD_NULL:
				break;
			case CMD_TO_STEP_LEFT_FOOT:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  STEP_FOOT_KGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  STEP_FOOT_KGJ_TIMES;

				break;
			case CMD_TO_STEP_LEFT_FOOT_1:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  STEP_FOOT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = SITDOWN_TO_ERECT_KGJ_ANGLE;//-SITDOWN_TO_ERECT_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -STEP_FOOT_1_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_KGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  STEP_FOOT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ] =  STEP_FOOT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  STEP_FOOT_1_XGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ] =  STEP_FOOT_1_XGJ_TIMES;
				break;
			case CMD_TO_STEP_LEFT_FOOT_2:
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -STEP_FOOT_XGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  STEP_FOOT_XGJ_TIMES;
				break;
			case CMD_TO_STEP_RIGHT_FOOT:
				ObjetMotorData.MotorAngles[RIGHT_KGJ]   =  -STEP_FOOT_KGJ_ANGLE;

				ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  STEP_FOOT_KGJ_TIMES;

				break;
			case CMD_TO_STEP_RIGHT_FOOT_1:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  -SITDOWN_TO_ERECT_KGJ_ANGLE;//SITDOWN_TO_ERECT_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -STEP_FOOT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  STEP_FOOT_1_XGJ_ANGLE;
				
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  STEP_FOOT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  STEP_FOOT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  STEP_FOOT_1_XGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  STEP_FOOT_1_XGJ_TIMES;
				break;
			case CMD_TO_STEP_RIGHT_FOOT_2:
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  = STEP_FOOT_XGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ]= STEP_FOOT_XGJ_TIMES;			
				break;
			case CMD_TO_SITDOWN:
					ObjetMotorData.MotorAngles[LEFT_KGJ]   =  ERECT_TO_SITDOWN_KGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -ERECT_TO_SITDOWN_KGJ_ANGLE;
					ObjetMotorData.MotorAngles[LEFT_XGJ]   = -ERECT_TO_SITDOWN_XGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  ERECT_TO_SITDOWN_XGJ_ANGLE;
					//直立到坐下
					if(LastWalkPhase_g == ERECTED)
					{
						ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERECT_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERECT_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  ERECT_TO_SITDOWN_XGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  ERECT_TO_SITDOWN_XGJ_TIMES;
					}
					//错误步相到坐下
					else
					{
						for(int i = 0;i<4;i++)
						{
							if(fabs(NowMotorData.MotorAngles[i]-ERECT_TO_SITDOWN_KGJ_ANGLE) < 45)
							{
								ObjetMotorData.MotorRunTimes[i] =  ERROR_TO_SITDOWN_KGJ_TIMES;
							}
							else
							{
								ObjetMotorData.MotorRunTimes[i] =  ERROR_TO_SITDOWN_KGJ_BIG_TIMES;
							}
						}
					}
				break;
			case CMD_TO_ERECT_1:
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_1_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_1_XGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  SITDOWN_TO_ERECT_1_XGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  SITDOWN_TO_ERECT_1_XGJ_TIMES;
				break;
			case CMD_TO_ERECT_2:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  SITDOWN_TO_ERECT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -SITDOWN_TO_ERECT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_XGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  SITDOWN_TO_ERECT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  SITDOWN_TO_ERECT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  SITDOWN_TO_ERECT_XGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  SITDOWN_TO_ERECT_XGJ_TIMES;
				break;
			case CMD_TO_ERECT_3:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   = STEP_FOOT_TO_ERECT_KGJ_1_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]  =  -STEP_FOOT_TO_ERECT_XGJ_1_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  STEP_FOOT_TO_ERECT_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  STEP_FOOT_TO_ERECT_XGJ_TIMES;
				break;
			case CMD_TO_ERECT_4:
				ObjetMotorData.MotorAngles[RIGHT_KGJ]   = -STEP_FOOT_TO_ERECT_KGJ_1_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  STEP_FOOT_TO_ERECT_XGJ_1_ANGLE;
			
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ] =  STEP_FOOT_TO_ERECT_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ] =  STEP_FOOT_TO_ERECT_XGJ_TIMES;
				break;
			case CMD_TO_ERECT_5:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =   ERECT_TO_SITDOWN_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  =  -ERECT_TO_SITDOWN_1_KGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERECT_TO_SITDOWN_KGJ_1_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERECT_TO_SITDOWN_KGJ_1_TIMES;
				break;
			case CMD_TO_ERECT:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  SITDOWN_TO_ERECT_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -SITDOWN_TO_ERECT_KGJ_ANGLE;
				//坐下到直立
				if(LastWalkPhase_g == SITDOWNED)
				{
					ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  SITDOWN_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  SITDOWN_TO_ERECT_KGJ_TIMES;
				}
				//从错误步相到直立
				else if(LastWalkPhase_g == ERROR_WALK_PHASE) 
				{
					ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_XGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_XGJ_ANGLE;
					for(int i = 0; i<4; i++)
					{
						if(fabs(NowMotorData.MotorAngles[i]-SITDOWN_TO_ERECT_XGJ_ANGLE) < 45)
						{
							ObjetMotorData.MotorRunTimes[i] =  ERROR_TO_ERECT_KGJ_TIMES;
						}
						else
						{
							ObjetMotorData.MotorRunTimes[i] =  ERROR_TO_ERECT_KGJ_BIG_TIMES;
						}
					}
				}
				//迈腿步相到直立
				else
				{
					ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_XGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_XGJ_ANGLE;
					
					ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERROR_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERROR_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  ERROR_TO_ERECT_XGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  ERROR_TO_ERECT_XGJ_TIMES;
				}
				break;
			case CMD_TO_TURNED:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  TURNED_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   =  -TURNED_XGJ_ANGLE;
				
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  TURNED_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  TURNED_XGJ_TIMES;
				break;
			default:
				break;
		}

//		for( MotorIndex=LEFT_KGJ; MotorIndex<=RIGHT_XGJ; MotorIndex++ )
//		{
//			 printf("%s current position angle is %f, object position angle is %f, assigned rotate time is %fs, rotate speed is %f\n",
//					MotorPosStrs[MotorIndex], 
//					NowMotorData.MotorAngles[MotorIndex], 
//					ObjetMotorData.MotorAngles[MotorIndex], 
//					ObjetMotorData.MotorRunTimes[MotorIndex] , 
//					ObjetMotorData.MotorSpeeds[MotorIndex]);
//		}
		OldCmd = CmdFalg;
	}
}

//将外部控制指令转换为内部控制指令
void TransExternalCmdToInternalCmd(void)
{
    if( NowWalkPhase_g == MOVING || CmdFalg != CMD_NULL) 
	{
		SetExternalCtrlCmd( TO_KEEP );
		return;
	}
    if( ExternalCtrlCmd_g.Cmd == TO_WALK )
    {
        //直立情况先迈左脚
        if( (NowWalkPhase_g == ERECTED) || (NowWalkPhase_g == RIGHT_FOOT_STEP) )
        {
            NextWalkPhase_g = LEFT_FOOT_STEP;
        }
        else if( NowWalkPhase_g == LEFT_FOOT_STEP )
        {
			NextWalkPhase_g = RIGHT_FOOT_STEP;
        }
    }
	if(ExternalCtrlCmd_g.Cmd == TO_ERECT)
	{
		NextWalkPhase_g = ERECTED;
	}
	if(ExternalCtrlCmd_g.Cmd == TO_SITDOWN)
	{
		NextWalkPhase_g = SITDOWNED;
	}
	if(ExternalCtrlCmd_g.Cmd == TO_TURN)
	{
		NextWalkPhase_g = TURNED;
	}
    //在稳定态，外部指令转换为内部指令后即复原
    SetExternalCtrlCmd( TO_KEEP );
    return;
}

void PackMotorCtrlMsg( uint8_t *MotorCtrlMsg, MOTOR_POS_ENUM MotorIndex )
{
    FloatToLittleEndingShort( MotorCtrlMsg, ObjetMotorData.MotorAngles[MotorIndex], MOTOR_BACK_DATA_SCALE );
    FloatToLittleEndingShort( MotorCtrlMsg+2, ObjetMotorData.MotorSpeeds[MotorIndex], MOTOR_BACK_DATA_SCALE );
    FloatToLittleEndingShort( MotorCtrlMsg+4, 0.0, MOTOR_BACK_DATA_SCALE  ); //角加速度，默认为0，由电驱板自行决定
    FloatToLittleEndingShort( MotorCtrlMsg+6, ObjetMotorData.MotorRunTimes[MotorIndex] , MOTOR_BACK_DATA_SCALE); //最大时间，默认为0
}

void CtrlMotor(void)
{
    uint8_t SendCanMsg[8];
    uint8_t SendCanLen = sizeof(SendCanMsg);
    uint16_t SendCanId = 0x000;
    int MotorIndex;
	for( MotorIndex=LEFT_KGJ; MotorIndex<=RIGHT_XGJ; MotorIndex++ )
	{
		uint8_t retrytimes = 3;
		int ret = -1;
		if(ObjetMotorData.MotorRunTimes[MotorIndex] != 0 )
		{
			SendCanId = CanIdsOfCtrlMotor[MotorIndex];
			memset( SendCanMsg, 0x00, SendCanLen );
			PackMotorCtrlMsg( SendCanMsg, (MOTOR_POS_ENUM)MotorIndex );
			while( (ret<0) && ((retrytimes--)>0) )
			{
				ret = CAN1_SendMsg( SendCanId, SendCanMsg, SendCanLen );
			}
			if( ret == 0 )
			{
					printf("Send can message to %s successfully! Can id is 0x%03x, " 
								 "data len is %d, context is : ", 
								 MotorPosStrs[MotorIndex], SendCanId, SendCanLen );
					PrintBuff( SendCanMsg, SendCanLen );
					ObjetMotorData.MotorRunTimes[MotorIndex] = 0;
			}
		} 

	} 
}

#define REMOTE_CMD_HEAD 0x68
#define REMOTE_CMD_TAIL 0x16
#define POS_OF_LEFT_OR_RIGHT 7
#define POS_OF_PRESS_TYPE 8
#define POS_OF_SUM 10
#define REMOTE_CMD_TYPE 0xC1   //指令类型
#define WRITE_DATA_FLASH_TYPE 0xC2
#define WRITE_ID_FLASH_TYPE 0xC3
#define RIGHT_TO_LEFT_TYPE 0xC4
#define READ_FLASH_TYPE 0xC5
#define LEFT 0x11
#define RIGHT 0x22
#define LEFT_AND_RIGHT 0x33

/*该函数用于解包从其他地方发送的数据*/
void UnpackRemoteCmd( uint8_t *buf, uint8_t len )
{
	uint32_t  Cmd_Type = 0; 
	if( (buf[0]==REMOTE_CMD_HEAD) && (buf[len-1]==REMOTE_CMD_TAIL) )
	{
		Cmd_Type = buf[2];
		switch(Cmd_Type)
		{
/*******************拐杖指令********************/
			case REMOTE_CMD_TYPE: //C1
				switch( buf[POS_OF_LEFT_OR_RIGHT] )
				{
					case LEFT:
						break;
					case RIGHT:
						if(buf[POS_OF_PRESS_TYPE] == 0x0D)  //右边点按
						{
							ExternalCtrlCmd_g.Cmd  = TO_WALK; 
						}
						if(buf[POS_OF_PRESS_TYPE] == 0x0C)  //右边点按
						{
							ExternalCtrlCmd_g.Cmd  = TO_TURN; 
						}
						break;
					case LEFT_AND_RIGHT:
						if(buf[POS_OF_PRESS_TYPE] == 0x0D)  //两边点按
						{
							ExternalCtrlCmd_g.Cmd  = TO_ERECT; 
						}
						if(buf[POS_OF_PRESS_TYPE] == 0x0C)  //两边长按
						{
							ExternalCtrlCmd_g.Cmd  = TO_SITDOWN; 

						}
						break;
					default:
						ExternalCtrlCmd_g.Cmd = TO_WALK;
						break;
				}
			break;
/*********写数据Flash命令*********/
//			case WRITE_DATA_FLASH_TYPE:  //C2
//				printf("***Writing Data to Flash,Please Wait.....!***\n");
//				for(int i = 0; i<ElementOfArray(ReadFlashBuffer); i++)
//				{
//					WriteFlashBuffer[i] = buf[i+3];        							
//					Dec_User_Data[i] = HexToDec(&WriteFlashBuffer[i]);  //根据输入的16进制数转为10进制方便比较
//				}
//				SetDataJudge();
//				WriteFlashData(FLASH_DATA_START_ADDR,FLASH_DATA_END_ADDR,WriteFlashBuffer,WRITE_FLASH_DATA_SIZE);  //等待写入完成
//				printf("***Writing OK! Sum Check is 0x%x\n",WriteFlashBuffer[7] );
//				SetInitDataFlash();
//				break;
//			
/*********写设备编号Flash命令*********/
			case WRITE_ID_FLASH_TYPE:  //C3
				printf("***Writing ID to Flash,Please Wait.....!***\n");
				for(int i = 0; i<WRITE_FLASH_ID_SIZE; i++)
				{
					WriteFlashBuffer[i] = buf[i+3];        							
				}
				WriteFlashData(FLASH_ID_START_ADDR,FLASH_ID_END_ADDR,WriteFlashBuffer,WRITE_FLASH_ID_SIZE);  //等待写入完成
				printf("***Writing OK! Sum Check is 0x%x\n",WriteFlashBuffer[7] );
				break;			
/*********读Flash命令*********/				
			case READ_FLASH_TYPE:  //C5
				PrintfUserFlash();
				break;				
		}
	}  
	else 
	{
		memset(buf,0,12);
	}
}
void GetAppData(void)
{

}
void UpLoadAppDataPro(void)
{
	//左腿电机数据
//	printf("\nLeft Motor Data:\n");
	UploadMotorDataLeft[0] = 0x68;
	UploadMotorDataLeft[1] = 0x0E;
	UploadMotorDataLeft[2] = 0xAA;
	UploadMotorDataLeft[3] = CanMotorAngles[LEFT_KGJ].Data[0];//NowMotorData.MotorAngles[LEFT_KGJ];
	UploadMotorDataLeft[4] = CanMotorAngles[LEFT_KGJ].Data[1];//NowMotorData.MotorAngles[LEFT_XGJ];
	UploadMotorDataLeft[5] = CanMotorAngles[LEFT_XGJ].Data[0];
	UploadMotorDataLeft[6] = CanMotorAngles[LEFT_XGJ].Data[1];
	UploadMotorDataLeft[7] = 0x00;  //力矩
	UploadMotorDataLeft[8] = 0x00;
	UploadMotorDataLeft[9] = 0xD0;  //温度
	UploadMotorDataLeft[10] = 0x07;
	UploadMotorDataLeft[11] = CanMotorAngles[LEFT_KGJ].Data[6];  //故障码
	UploadMotorDataLeft[12] = CanMotorAngles[LEFT_XGJ].Data[6];
	UploadMotorDataLeft[13] = 0x16;
	HAL_UART_Transmit(&huart4,UploadMotorDataLeft,14,1000);


	//printf("\nRight Motor Data:\n");
	//右腿电机数据
	UploadMotorDataRight[0] = 0x68;
	UploadMotorDataRight[1] = 0x0E;
	UploadMotorDataRight[2] = 0xBB;
	UploadMotorDataRight[3] = CanMotorAngles[RIGHT_KGJ].Data[0];//0xd8; //KGJ角度  -90°
	UploadMotorDataRight[4] = CanMotorAngles[RIGHT_KGJ].Data[1];
	UploadMotorDataRight[5] = CanMotorAngles[RIGHT_XGJ].Data[0]; //XGJ角度  90°
	UploadMotorDataRight[6] = CanMotorAngles[RIGHT_XGJ].Data[1];
	UploadMotorDataRight[7] = 0x00; //力矩     0
	UploadMotorDataRight[8] = 0x00;
	UploadMotorDataRight[9] = 0xD0; //温度     20℃
	UploadMotorDataRight[10] = 0x07;
	UploadMotorDataRight[11] = CanMotorAngles[RIGHT_XGJ].Data[6];
	UploadMotorDataRight[12] = CanMotorAngles[RIGHT_XGJ].Data[6]; 
	UploadMotorDataRight[13] = 0x16;
	HAL_UART_Transmit(&huart4,UploadMotorDataRight,14,1000);

	//电池数据
//	printf("\nCell Data:\n");
	UploadCellData[0] = 0x68;
	UploadCellData[1] = 0x0E;
	UploadCellData[2] = 0xCC;
	UploadCellData[3] = 0x10;  //电量
	UploadCellData[4] = 0x27;
	UploadCellData[5] = 0x10;  //电压
	UploadCellData[6] = 0x0E;
	UploadCellData[7] = 0xF4;  //电流
	UploadCellData[8] = 0x01;
	UploadCellData[9] = 0xD0;  //温度
	UploadCellData[10] = 0x07;
	UploadCellData[11] = 0x00;
	UploadCellData[12] = 0xFF;
	UploadCellData[13] = 0x16;
	
	HAL_UART_Transmit(&huart4,UploadCellData,14,1000);

}

#define MAX_WARNING_ANGLE 120.0
#define MIN_WARNING_ANGLE 15.0
void SoftwareLimit(void)   //角度限位判断
{
	/*左边髋关节*/ 
	if((float)MAX_WARNING_ANGLE<NowMotorData.MotorAngles[LEFT_KGJ] || NowMotorData.MotorAngles[LEFT_KGJ]<-(float)MIN_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[LEFT_KGJ].AngleStatus = STATUS_ERROR;

	}
	else
	{
		JointStatus[LEFT_KGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*右边髋关节*/
	if((float)MIN_WARNING_ANGLE<NowMotorData.MotorAngles[RIGHT_KGJ] || NowMotorData.MotorAngles[RIGHT_KGJ]<-(float)MAX_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[RIGHT_KGJ].AngleStatus = STATUS_ERROR;

	}
	else
	{
		JointStatus[RIGHT_KGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*左边膝关节*/
	if((float)MIN_WARNING_ANGLE<NowMotorData.MotorAngles[LEFT_XGJ] || NowMotorData.MotorAngles[LEFT_XGJ]<-(float)MAX_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[LEFT_XGJ].AngleStatus = STATUS_ERROR;
	}
	else
	{
		JointStatus[LEFT_XGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*右边膝关节*/
	if((float)MAX_WARNING_ANGLE<NowMotorData.MotorAngles[RIGHT_XGJ] || NowMotorData.MotorAngles[RIGHT_XGJ]<-(float)MIN_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[RIGHT_XGJ].AngleStatus = STATUS_ERROR;
	}
	else
	{
		JointStatus[RIGHT_XGJ].AngleStatus = STATUS_NORMAL;
	}
}
void CanRxErrorCheck(void)
{
	int MotorIndex ;
  for(MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
	{
		if(CanMotorAngles[MotorIndex].SpaceTimes > 1)
		{
			JointStatus[MotorIndex].CanStatus = STATUS_ERROR;
		}
		else
		{
			JointStatus[MotorIndex].CanStatus = STATUS_NORMAL;
		}
	}
}
STATUS_ENUM CheckStatus(void) //自检状态
{
	STATUS_ENUM SystemStatus = STATUS_NORMAL;
	int MotorIndex ;
	SoftwareLimit();   //软件限位
	CanRxErrorCheck();  //Can通讯是否正常
	CompareTargetAngle(); //比较目标角度
	for(MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
	{
		// 判断角度
		if(JointStatus[MotorIndex].AngleStatus != STATUS_NORMAL)
		{
			SystemStatus = STATUS_ERROR;
			JointStatus[MotorIndex].NowsystemStauts = STATUS_ERROR;
			if(JointStatus[MotorIndex].OldsystemStauts == STATUS_NORMAL && JointStatus[MotorIndex].NowsystemStauts ==  STATUS_ERROR)
			{
				JointStatus[MotorIndex].OldsystemStauts = STATUS_ERROR;
				printf("AngleStatus of %s is Error!\nPlease Check!\n",MotorPosStrs[MotorIndex]);
			}	
		}

		//判断Can
		else if(JointStatus[MotorIndex].CanStatus != STATUS_NORMAL)
		{
			SystemStatus = STATUS_ERROR;
			JointStatus[MotorIndex].NowsystemStauts = STATUS_ERROR;
			if(JointStatus[MotorIndex].OldsystemStauts == STATUS_NORMAL && JointStatus[MotorIndex].NowsystemStauts ==  STATUS_ERROR)
			{
				JointStatus[MotorIndex].OldsystemStauts = STATUS_ERROR;
				printf("CanStatus of %s is Error!\nPlease Check!\n",MotorPosStrs[MotorIndex]);
			}
		}
		else //系统正常了
		{
			JointStatus[MotorIndex].NowsystemStauts = STATUS_NORMAL;
	 		if(JointStatus[MotorIndex].OldsystemStauts == STATUS_ERROR && JointStatus[MotorIndex].NowsystemStauts ==  STATUS_NORMAL)
			{
				JointStatus[MotorIndex].OldsystemStauts = STATUS_NORMAL;
				printf("%s is Normal!\n",MotorPosStrs[MotorIndex]);
			}

		}
	}
	return SystemStatus;
}

/*获取电池电量*/
#define VOLTAGE_MIN 28    
#define VOLTAGE_MAX 42    
#define VOLTAGE_SCALE  35.8  //硬件分压比
#define VOLTAGE_WARN 30.8   //((VOLTAGE_MAX-VOLTAGE_MIN)*0.2 + VOLTAGE_MIN) 报警电压20%  30.8v
#define VOLTAGE_EPS 0.3
volatile uint16_t ADC1Value = 0; //

void GetBatteryVoltage(void)
{
	static float Voltage_Reality = 0;  //电池实际电压
	static float TempVoltageMin = 0;   //采集到的最小电压
	float temp_V = 0;
	if(ADC1Value!=0)
	{
		Voltage_Reality = ADC1Value*VOLTAGE_SCALE*3.3f/4096;
		
		if(TempVoltageMin > Voltage_Reality||TempVoltageMin < VOLTAGE_MIN) //采集到的电压比以前小的电压才认为是有效电压。
		{
			temp_V = TempVoltageMin;
			TempVoltageMin = Voltage_Reality;		
 			if( !IsEqual( Voltage_Reality, temp_V, (float)VOLTAGE_EPS ))   //如果跳动值大于VOLTAGE_EPS，则放弃此次采样
			{
				TempVoltageMin = temp_V;
				if(temp_V  == 0) //第一次进入后给最小电压赋值
				{
					TempVoltageMin = Voltage_Reality;
				}
			}
		} 
		if(TempVoltageMin < (float)VOLTAGE_WARN)
		{
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,GPIO_PIN_RESET);
		}
		else
		{
			//HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,GPIO_PIN_SET);
		}		
		printf("ADC1 Reading : %d \r\n",ADC1Value);
		printf("PB0 Voltage : %.4f \r\n",ADC1Value*3.3f/4096);
		printf("Reality Voltage : %.4f \r\n",Voltage_Reality);
		printf("\n\n");
		ADC1Value = 0;
	}
}

void CompareTargetAngle(void)
{
	int MotorIndex = 0;
	for(MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ ; MotorIndex++)
	{
		if(NowMotorData.MotorSynAngle[MotorIndex] != ObjetMotorData.MotorAngles[MotorIndex])
		{
			JointStatus[MotorIndex].CanErrorTimes++;
			if(JointStatus[MotorIndex].CanErrorTimes >= 10)
			{
				taskENTER_CRITICAL();           //进入临界区
				OldCmd = 0xff;
				GetMotorObjectAngleAndSpeed();
				for(int i = 0; i<4; i++)
				{
					if(i != MotorIndex)
					{
						ObjetMotorData.MotorRunTimes[i] = 0;
					}
				}
				CtrlMotor();
				taskEXIT_CRITICAL();            //退出临界区
				JointStatus[MotorIndex].CanErrorTimes = 0;
			}
			
		}
		else
		{
			JointStatus[MotorIndex].CanErrorTimes = 0;
		}
	}
}
extern uint8_t RxData[8];
extern CAN_RxHeaderTypeDef	RxHeader;
void CANDataRxPro(void)
{
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
		/*****-----------*****/
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
}
void SetExternalCtrlCmd( CTRL_CMD_ENUM Cmd )
{
    ExternalCtrlCmd_g.Cmd = Cmd;
    //ExternalCtrlCmd_g.escapeTime = 0;
}
