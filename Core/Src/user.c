#include "user.h"
#include "math_user.h"
#include "stick.h"
#include "usart.h"
#include "flash.h"


/*ȫ�ֱ���*/
volatile MotorData_t NowMotorData;
volatile MotorData_t ObjetMotorData;
volatile CtrlCmd_t ExternalCtrlCmd_g;
volatile JointStatus_t 	JointStatus[4];
CANFrameData_t CanMotorAngles[4];
WALK_PHASE_ENUM LastWalkPhase_g = ERROR_WALK_PHASE;
WALK_PHASE_ENUM NowWalkPhase_g = ERROR_WALK_PHASE;
WALK_PHASE_ENUM NextWalkPhase_g = ERROR_WALK_PHASE ;
uint8_t  UploadMotorDataLeft[13] = {0};  //�ϴ����ȵ������
uint8_t  UploadMotorDataRight[13]= {0};  //�ϴ����ȵ������
uint8_t  UploadCellData[13] = {0};  //�ϴ����ȵ������
uint32_t WriteFlashBuffer[WRITE_FLASH_ID_SIZE]= {0};   //16����
uint32_t ReadFlashBuffer[WRITE_FLASH_DATA_SIZE] = {0};
uint8_t CmdFalg = CMD_NULL;
uint8_t CanStartRxFlag = 0;


/**********************************************
*������ǶȵĶ�Ӧ��ϵ
{+��-}
{-��+}
**********************************************/
MotorAnglesOfWalkPhase_t Maps_MotorAnglesOfWalkPhase[] = 
{
    { SITDOWNED,       { INIT_MAX_ANGLE_OF_KGJ,     -INIT_MAX_ANGLE_OF_KGJ,                            
                         -INIT_MAX_ANGLE_OF_XGJ,     INIT_MAX_ANGLE_OF_XGJ } },
																											
    { ERECTED,         { INIT_MAX_ANGLE_OF_ERECTED,  INIT_MAX_ANGLE_OF_ERECTED,  
                         INIT_MAX_ANGLE_OF_ERECTED,  INIT_MAX_ANGLE_OF_ERECTED } },
																											
    { LEFT_FOOT_STEP,  { INIT_MAX_ANGLE_OF_WALK_KB,  INIT_MAX_ANGLE_OF_ERECTED,  
                         -INIT_MAX_ANGLE_OF_LAY_XGJ, INIT_MAX_ANGLE_OF_ERECTED } },
												
    { RIGHT_FOOT_STEP, { INIT_MAX_ANGLE_OF_ERECTED,  -INIT_MAX_ANGLE_OF_WALK_KB,  
                         INIT_MAX_ANGLE_OF_ERECTED,  INIT_MAX_ANGLE_OF_LAY_XGJ } },  																
};

//���岽��Ǩ��ͼ
//StateMachine_t WalkStateMachineTransMap[] = 
//{
//	{ SITDOWNED,        TO_ERECT,   ERECTED,          TO_KEEP, 1},      //����
//	{ ERECTED,          TO_SITDOWN, SITDOWNED,        TO_KEEP, 1},
//	{ ERECTED,          TO_WALK,    LEFT_FOOT_STEP,  TO_KEEP,  1},
//	{ LEFT_FOOT_STEP,   TO_WALK,    RIGHT_FOOT_STEP, TO_KEEP,  1},
//	{ RIGHT_FOOT_STEP,  TO_WALK,    LEFT_FOOT_STEP,  TO_KEEP,  1},
//	{ LEFT_FOOT_STEP,   TO_ERECT,   ERECTED,          TO_KEEP, 1},
//	{ RIGHT_FOOT_STEP , TO_ERECT,   ERECTED,          TO_KEEP, 1},
//	{ ERROR_WALK_PHASE, TO_ERECT,   ERECTED,          TO_KEEP, 1},
//	{ ERROR_WALK_PHASE, TO_SITDOWN, SITDOWNED,        TO_KEEP, 1},
//};

const uint16_t CanIdsOfMotorBackData[] =
{
    MOTOR_BACK_CAN_ID + LEFT_KGJ_NODE_ID  ,
    MOTOR_BACK_CAN_ID + RIGHT_KGJ_NODE_ID ,
    MOTOR_BACK_CAN_ID + LEFT_XGJ_NODE_ID  ,
    MOTOR_BACK_CAN_ID + RIGHT_XGJ_NODE_ID ,
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



/*��������*/

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

    //��ʼ��ȫ�ֱ���
    SetExternalCtrlCmd( TO_KEEP );
    for( i=0; i<ElementOfArray(NowMotorData.MotorAngles); i++ )
    { 
        NowMotorData.MotorAngles[i] = 0.0;
    }
    for( i=0; i<ElementOfArray(NowMotorData.MotorSpeeds); i++ )
    { 
        NowMotorData.MotorSpeeds[i] = 0.0;
    }
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6,GPIO_PIN_SET); //����ģ��͸��ģʽ
//		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15,GPIO_PIN_RESET);
//	SetInitDataFlash();
//	PrintfUserFlash();
}

/*�ú������ڻ�ȡ�����ʵʱ����*/
void GetAllMotorState(void)
{
	static uint8_t Rxtimes= 0 ;
    int MotorIndex;
    for (MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
    {				
        if ((CanMotorAngles[MotorIndex].Len == 8) && (CanIdsOfMotorBackData[MotorIndex] == CanMotorAngles[MotorIndex].Id))
        {
			Rxtimes++;
			if(Rxtimes >= 40)   //����ǰ40�ν��յ�����
			{
				Rxtimes = 100;
				NowMotorData.MotorAngles[MotorIndex] =
						LittleEndingShortToFloat((uint8_t *)CanMotorAngles[MotorIndex].Data, MOTOR_BACK_DATA_SCALE);  //����������ǰ�ĽǶ���Ϣ
				NowMotorData.MotorSpeeds[MotorIndex] =
						LittleEndingShortToFloat((uint8_t *)CanMotorAngles[MotorIndex].Data+2, MOTOR_BACK_DATA_SCALE);  //����������ǰ���ٶ���Ϣ
			}
		}
    }
}

/*�ú��������жϵ���Ƿ�ֹͣ*/
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

/*�ú������ڻ�ȡʵʱ����*/
void GetWalkPhase(void)
{
    WALK_PHASE_ENUM WalkPhase = SITDOWNED;
    int i = 0;
    bool hasNotEqual = false;
	if(NextWalkPhase_g == ERROR_WALK_PHASE)   //����
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
        //û�в���ȵģ���ȫ����ȣ����ڸò���
        if( !hasNotEqual )
        {
            NowWalkPhase_g =  WalkPhase;
			CmdFalg = CMD_NULL;
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
		return  ;
	}		
	switch((uint8_t)LastWalkPhase_g)
	{		
		/*��ǰ����Ϊ ������*/
		case LEFT_FOOT_STEP:
			if(NextWalkPhase_g ==  ERECTED)  //������ ֱ��
			{
				CmdFalg = CMD_TO_ERECT;
				//printf(" NOW : LEFT_FOOT_STEP , Next : CMD_TO_ERECT \n ");
			}
			if(NextWalkPhase_g == RIGHT_FOOT_STEP)  //������ ������
			{
				if(CmdFalg == CMD_NULL)
				{
					CmdFalg = CMD_TO_STEP_RIGHT_FOOT_1;
					//printf(" NOW : LEFT_FOOT_STEP , Next : CMD_TO_STEP_RIGHT_FOOT_1 \n ");
				}
				switch(CmdFalg)
				{
					case CMD_TO_STEP_RIGHT_FOOT_1:
						if(NowMotorData.MotorAngles[RIGHT_XGJ] > (STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[RIGHT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_RIGHT_FOOT_2;
							//printf(" NOW : LEFT_FOOT_STEP , Next : CMD_TO_STEP_RIGHT_FOOT_2 \n ");
						}
						break;
					case CMD_TO_STEP_RIGHT_FOOT_2:
						if(NowMotorData.MotorAngles[RIGHT_KGJ] < -(STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[RIGHT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_RIGHT_FOOT;
							//printf(" NOW : LEFT_FOOT_STEP , Next : CMD_TO_STEP_RIGHT_FOOT \n ");
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
		/*��ǰ����Ϊ ������*/
		case RIGHT_FOOT_STEP:
			if(NextWalkPhase_g ==  ERECTED)  //������ ֱ��
			{
				CmdFalg = CMD_TO_ERECT;
				//printf(" NOW : RIGHT_FOOT_STEP , Next : CMD_TO_ERECT \n ");
			}
			if(NextWalkPhase_g == LEFT_FOOT_STEP)  //������ ������
			{
				if(CmdFalg == CMD_NULL)
				{
					CmdFalg = CMD_TO_STEP_LEFT_FOOT_1;
					//printf(" NOW : RIGHT_FOOT_STEP , Next : CMD_TO_STEP_LEFT_FOOT_1 \n ");
				}
				switch(CmdFalg)
				{
					case CMD_TO_STEP_LEFT_FOOT_1:
						if( NowMotorData.MotorAngles[LEFT_XGJ] < -(STEP_FOOT_1_XGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[LEFT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT_2;
							//printf(" NOW : RIGHT_FOOT_STEP , Next : CMD_TO_STEP_LEFT_FOOT_2 \n ");
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT_2:
						if( NowMotorData.MotorAngles[LEFT_KGJ] > (STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) &&NowMotorData.MotorStatus[LEFT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT;
							//printf(" NOW : RIGHT_FOOT_STEP , Next : CMD_TO_STEP_LEFT_FOOT \n ");
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
			
			
		/*��ǰ����Ϊ ֱ��*/
		case ERECTED:
			if(NextWalkPhase_g ==  SITDOWNED)  //������ ����
			{
				//printf(" NOW : ERECTED , Next : CMD_TO_SITDOWN \n ");
				CmdFalg = CMD_TO_SITDOWN;
			}
			if(NextWalkPhase_g == LEFT_FOOT_STEP)  //������ ������
			{
				if(CmdFalg == CMD_NULL)	
				{
					CmdFalg = CMD_TO_STEP_LEFT_FOOT_1;
					//printf(" NOW : ERECTED , Next : CMD_TO_STEP_LEFT_FOOT_1 \n ");
				}					
				switch(CmdFalg)
				{
					case CMD_TO_STEP_LEFT_FOOT_1:
						if( NowMotorData.MotorAngles[LEFT_XGJ] < -(STEP_FOOT_1_XGJ_ANGLE - ANGLE_EPS) && NowMotorData.MotorStatus[LEFT_XGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT_2;
							//printf(" NOW : ERECTED , Next : CMD_TO_STEP_LEFT_FOOT_2 \n ");
						}
						break ;
					case CMD_TO_STEP_LEFT_FOOT_2:
						if( NowMotorData.MotorAngles[LEFT_KGJ] > (STEP_FOOT_1_KGJ_ANGLE - ANGLE_EPS) &&NowMotorData.MotorStatus[LEFT_KGJ] == MOTOR_STOPPING)
						{
							CmdFalg = CMD_TO_STEP_LEFT_FOOT;
							//printf(" NOW : ERECTED , Next : CMD_TO_STEP_LEFT_FOOT \n ");
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
			break;
		/*��ǰ����Ϊ ����*/
		case SITDOWNED:
			if(NextWalkPhase_g ==  ERECTED)  //������ ֱ��
			{
				if(CmdFalg == CMD_NULL)
				{
					CmdFalg = CMD_TO_ERECT_1;
					//printf(" NOW : SITDOWNED , Next : CMD_TO_ERECT_1 \n ");
				}
				switch(CmdFalg)
				{
					case CMD_TO_ERECT_1:
						
						if(  NowMotorData.MotorAngles[LEFT_KGJ] > (SITDOWN_TO_ERECT_1_KGJ_ANGLE - ANGLE_EPS) && 
							 NowMotorData.MotorAngles[RIGHT_KGJ]<-(SITDOWN_TO_ERECT_1_KGJ_ANGLE - ANGLE_EPS) && 
							 NowMotorData.MotorAngles[LEFT_XGJ] <-(SITDOWN_TO_ERECT_1_XGJ_ANGLE - ANGLE_EPS) && 
							 NowMotorData.MotorAngles[RIGHT_XGJ]> (SITDOWN_TO_ERECT_1_XGJ_ANGLE - ANGLE_EPS) &&  
							IsAllMotorStop()      )   //���������Ƿ�λ
						{
							CmdFalg = CMD_TO_ERECT;	
							//printf(" NOW : SITDOWNED , Next : CMD_TO_ERECT \n ");
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

		/*��ǰ����Ϊ �Ƿ�����*/
		case ERROR_WALK_PHASE:
			if(NextWalkPhase_g ==  ERECTED)  //������ ֱ��
			{
				CmdFalg = CMD_TO_ERECT;
			}
			if(NextWalkPhase_g == SITDOWNED)  //������ ����
			{
				CmdFalg = CMD_TO_SITDOWN;
			}
			break;
		
	    /*NULL*/
		default :
			break;
	}
	int MotorIndex = 0;
	static uint8_t OldCmd = 0xff;
	if(OldCmd != CmdFalg)
	{
		switch(CmdFalg)
		{
			case CMD_NULL:
				break;
			case CMD_TO_SITDOWN:
					ObjetMotorData.MotorAngles[LEFT_KGJ]   =  ERECT_TO_SITDOWN_KGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -ERECT_TO_SITDOWN_KGJ_ANGLE;
					ObjetMotorData.MotorAngles[LEFT_XGJ]   = -ERECT_TO_SITDOWN_XGJ_ANGLE;
					ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  ERECT_TO_SITDOWN_XGJ_ANGLE;
					if(LastWalkPhase_g == ERECTED)
					{
						ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERECT_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERECT_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  ERECT_TO_SITDOWN_XGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  ERECT_TO_SITDOWN_XGJ_TIMES;
					}
					else
					{
						ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERROR_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERROR_TO_SITDOWN_KGJ_TIMES;
						ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  ERROR_TO_SITDOWN_XGJ_TIMES;
						ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  ERROR_TO_SITDOWN_XGJ_TIMES;
					}
				break;
			case CMD_TO_ERECT_1:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  SITDOWN_TO_ERECT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -SITDOWN_TO_ERECT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_1_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_1_XGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  SITDOWN_TO_ERECT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  SITDOWN_TO_ERECT_1_KGJ_TIMES;
				ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  SITDOWN_TO_ERECT_1_XGJ_TIMES;
				ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  SITDOWN_TO_ERECT_1_XGJ_TIMES;
				break;
			case CMD_TO_ERECT:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  SITDOWN_TO_ERECT_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -SITDOWN_TO_ERECT_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[LEFT_XGJ]   = -SITDOWN_TO_ERECT_XGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_XGJ]  =  SITDOWN_TO_ERECT_XGJ_ANGLE;
				if(LastWalkPhase_g == SITDOWNED)
				{
					ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  SITDOWN_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  SITDOWN_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  SITDOWN_TO_ERECT_XGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  SITDOWN_TO_ERECT_XGJ_TIMES;
				}
				else //if(LastWalkPhase_g == LEFT_FOOT_STEP ||LastWalkPhase_g == RIGHT_FOOT_STEP)
				{
					ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  ERROR_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_KGJ]=  ERROR_TO_ERECT_KGJ_TIMES;
					ObjetMotorData.MotorRunTimes[LEFT_XGJ] =  ERROR_TO_ERECT_XGJ_TIMES;
					ObjetMotorData.MotorRunTimes[RIGHT_XGJ]=  ERROR_TO_ERECT_XGJ_TIMES;
				}
				break;
			case CMD_TO_STEP_LEFT_FOOT:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  STEP_FOOT_KGJ_ANGLE;
			
				ObjetMotorData.MotorRunTimes[LEFT_KGJ] =  STEP_FOOT_KGJ_TIMES;

				break;
			case CMD_TO_STEP_LEFT_FOOT_1:
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  STEP_FOOT_1_KGJ_ANGLE;
				ObjetMotorData.MotorAngles[RIGHT_KGJ]  = -SITDOWN_TO_ERECT_KGJ_ANGLE;
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
				ObjetMotorData.MotorAngles[LEFT_KGJ]   =  SITDOWN_TO_ERECT_KGJ_ANGLE;
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
			default:
				break;
		}

		for( MotorIndex=LEFT_KGJ; MotorIndex<=RIGHT_XGJ; MotorIndex++ )
		{
			 printf("%s current position angle is %f, object position angle is %f, assigned rotate time is %fs, rotate speed is %f\n",
					MotorPosStrs[MotorIndex], 
					NowMotorData.MotorAngles[MotorIndex], 
					ObjetMotorData.MotorAngles[MotorIndex], 
					ObjetMotorData.MotorRunTimes[MotorIndex] , 
					ObjetMotorData.MotorSpeeds[MotorIndex]);
		}
		OldCmd = CmdFalg;
	}
}

//���ⲿ����ָ��ת��Ϊ�ڲ�����ָ��
void TransExternalCmdToInternalCmd(void)
{
    if( NowWalkPhase_g == MOVING  )  
        return;

//    if( ExternalCtrlCmd_g.Cmd == TO_KEEP )
//        return;

    if( ExternalCtrlCmd_g.Cmd == TO_WALK )
    {
        //ֱ������������
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
    //���ȶ�̬���ⲿָ��ת��Ϊ�ڲ�ָ��󼴸�ԭ
    SetExternalCtrlCmd( TO_KEEP );

    return;
}

void PackMotorCtrlMsg( uint8_t *MotorCtrlMsg, MOTOR_POS_ENUM MotorIndex )
{
    FloatToLittleEndingShort( MotorCtrlMsg, ObjetMotorData.MotorAngles[MotorIndex], MOTOR_BACK_DATA_SCALE );
    FloatToLittleEndingShort( MotorCtrlMsg+2, ObjetMotorData.MotorSpeeds[MotorIndex], MOTOR_BACK_DATA_SCALE );
    FloatToLittleEndingShort( MotorCtrlMsg+4, 0.0, MOTOR_BACK_DATA_SCALE  ); //�Ǽ��ٶȣ�Ĭ��Ϊ0���ɵ��������о���
    FloatToLittleEndingShort( MotorCtrlMsg+6, ObjetMotorData.MotorRunTimes[MotorIndex] , MOTOR_BACK_DATA_SCALE); //���ʱ�䣬Ĭ��Ϊ0
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
#define REMOTE_CMD_TYPE 0xC1   //ָ������
#define WRITE_DATA_FLASH_TYPE 0xC2
#define WRITE_ID_FLASH_TYPE 0xC3
#define RIGHT_TO_LEFT_TYPE 0xC4
#define READ_FLASH_TYPE 0xC5
#define LEFT 0x11
#define RIGHT 0x22
#define LEFT_AND_RIGHT 0x33

/*�ú������ڽ���������ط����͵�����*/
void UnpackRemoteCmd( uint8_t *buf, uint8_t len )
{
	uint32_t  Cmd_Type = 0; 
	if( (buf[0]==REMOTE_CMD_HEAD) && (buf[len-1]==REMOTE_CMD_TAIL) )
	{
		Cmd_Type = buf[2];
		switch(Cmd_Type)
		{
/*******************����ָ��********************/
			case REMOTE_CMD_TYPE: //C1
				switch( buf[POS_OF_LEFT_OR_RIGHT] )
				{
					case LEFT:
						break;
					case RIGHT:
						if(buf[POS_OF_PRESS_TYPE] == 0x0D)  //�ұߵ㰴
						{
							ExternalCtrlCmd_g.Cmd  = TO_WALK; 
							//printf("�ұߵ㰴\n");
						}
						break;
					case LEFT_AND_RIGHT:
						if(buf[POS_OF_PRESS_TYPE] == 0x0D)  //���ߵ㰴
						{
							ExternalCtrlCmd_g.Cmd  = TO_ERECT; 
							//printf("���ߵ㰴\n");
						}
						if(buf[POS_OF_PRESS_TYPE] == 0x0C)  //���߳���
						{
							ExternalCtrlCmd_g.Cmd  = TO_SITDOWN; 
							//printf("���߳���\n");
						}
						break;
					default:
						ExternalCtrlCmd_g.Cmd = TO_WALK;
						break;
				}
			break;
/*********д����Flash����*********/
//			case WRITE_DATA_FLASH_TYPE:  //C2
//				printf("***Writing Data to Flash,Please Wait.....!***\n");
//				for(int i = 0; i<ElementOfArray(ReadFlashBuffer); i++)
//				{
//					WriteFlashBuffer[i] = buf[i+3];        							
//					Dec_User_Data[i] = HexToDec(&WriteFlashBuffer[i]);  //���������16������תΪ10���Ʒ���Ƚ�
//				}
//				SetDataJudge();
//				WriteFlashData(FLASH_DATA_START_ADDR,FLASH_DATA_END_ADDR,WriteFlashBuffer,WRITE_FLASH_DATA_SIZE);  //�ȴ�д�����
//				printf("***Writing OK! Sum Check is 0x%x\n",WriteFlashBuffer[7] );
//				SetInitDataFlash();
//				break;
//			
/*********д�豸���Flash����*********/
			case WRITE_ID_FLASH_TYPE:  //C3
				printf("***Writing ID to Flash,Please Wait.....!***\n");
				for(int i = 0; i<WRITE_FLASH_ID_SIZE; i++)
				{
					WriteFlashBuffer[i] = buf[i+3];        							
				}
				WriteFlashData(FLASH_ID_START_ADDR,FLASH_ID_END_ADDR,WriteFlashBuffer,WRITE_FLASH_ID_SIZE);  //�ȴ�д�����
				printf("***Writing OK! Sum Check is 0x%x\n",WriteFlashBuffer[7] );
				break;			
/*********��Flash����*********/				
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

void UpLoadAppDataPro(void)
{
	//���ȵ������
//	printf("\nLeft Motor Data:\n");
	UploadMotorDataLeft[0] = 0x68;
	UploadMotorDataLeft[1] = 0x0A;
	UploadMotorDataLeft[2] = 0x28;//NowMotorData.MotorAngles[LEFT_KGJ];
	UploadMotorDataLeft[3] = 0x23;//NowMotorData.MotorAngles[LEFT_XGJ];
	UploadMotorDataLeft[4] = 0xd8;
	UploadMotorDataLeft[5] = 0xdc;
	UploadMotorDataLeft[6] = 0x00;  //����
	UploadMotorDataLeft[7] = 0x00;
	UploadMotorDataLeft[8] = 0xD0;  //�¶�
	UploadMotorDataLeft[9] = 0x07;
	UploadMotorDataLeft[10] = 0x00;  //������
	UploadMotorDataLeft[11] = 0xFF;
	UploadMotorDataLeft[12] = 0x16;
	HAL_UART_Transmit(&huart4,UploadMotorDataLeft,13,1000);


	//printf("\nRight Motor Data:\n");
	//���ȵ������
	UploadMotorDataRight[0] = 0x68;
	UploadMotorDataRight[1] = 0x0A;
	UploadMotorDataRight[2] = 0xd8; //KGJ�Ƕ�  -90��
	UploadMotorDataRight[3] = 0xdc;
	UploadMotorDataRight[4] = 0x28; //XGJ�Ƕ�  90��
	UploadMotorDataRight[5] = 0x23;
	UploadMotorDataRight[6] = 0x00; //����     0
	UploadMotorDataRight[7] = 0x00;
	UploadMotorDataRight[8] = 0xD0; //�¶�     20��
	UploadMotorDataRight[9] = 0x07;
	UploadMotorDataRight[10] = 0x00;
	UploadMotorDataRight[11] = 0xFF; 
	UploadMotorDataRight[12] = 0x16;
	HAL_UART_Transmit(&huart4,UploadMotorDataRight,13,1000);

	//�������
//	printf("\nCell Data:\n");
	UploadCellData[0] = 0x68;
	UploadCellData[1] = 0x0C;
	UploadCellData[2] = 0x10;  //����
	UploadCellData[3] = 0x27;
	UploadCellData[4] = 0x10;  //��ѹ
	UploadCellData[5] = 0x0E;
	UploadCellData[6] = 0xF4;  //����
	UploadCellData[7] = 0x01;
	UploadCellData[8] = 0xD0;  //�¶�
	UploadCellData[9] = 0x07;
	UploadCellData[10] = 0x00;
	UploadCellData[11] = 0xFF;
	UploadCellData[12] = 0x16;
	
	HAL_UART_Transmit(&huart4,UploadCellData,13,1000);

}

#define MAX_WARNING_ANGLE 120.0
#define MIN_WARNING_ANGLE 15.0
void SoftwareLimit(void)   //�Ƕ���λ�ж�
{
	/*����Źؽ�*/ 
	if((float)MAX_WARNING_ANGLE<NowMotorData.MotorAngles[LEFT_KGJ] || NowMotorData.MotorAngles[LEFT_KGJ]<-(float)MIN_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[LEFT_KGJ].AngleStatus = STATUS_ERROR;

	}
	else
	{
		JointStatus[LEFT_KGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*�ұ��Źؽ�*/
	if((float)MIN_WARNING_ANGLE<NowMotorData.MotorAngles[RIGHT_KGJ] || NowMotorData.MotorAngles[RIGHT_KGJ]<-(float)MAX_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[RIGHT_KGJ].AngleStatus = STATUS_ERROR;

	}
	else
	{
		JointStatus[RIGHT_KGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*���ϥ�ؽ�*/
	if((float)MIN_WARNING_ANGLE<NowMotorData.MotorAngles[LEFT_XGJ] || NowMotorData.MotorAngles[LEFT_XGJ]<-(float)MAX_WARNING_ANGLE)
	{
		NextWalkPhase_g = ERROR_WALK_PHASE;
		JointStatus[LEFT_XGJ].AngleStatus = STATUS_ERROR;
	}
	else
	{
		JointStatus[LEFT_XGJ].AngleStatus = STATUS_NORMAL;
	}
	
	/*�ұ�ϥ�ؽ�*/
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
STATUS_ENUM CheckStatus(void) //�Լ�״̬
{

	STATUS_ENUM SystemStatus = STATUS_NORMAL;
	int MotorIndex ;
	SoftwareLimit();
	CanRxErrorCheck();
  for(MotorIndex = LEFT_KGJ; MotorIndex <= RIGHT_XGJ; MotorIndex++)
	{
		// �жϽǶ�
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

		//�ж�Can
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
		else //ϵͳ������
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

/*��ȡ��ص���*/
#define VOLTAGE_MIN 28    
#define VOLTAGE_MAX 42    
#define VOLTAGE_SCALE  35.8  //Ӳ����ѹ��
#define VOLTAGE_WARN 30.8   //((VOLTAGE_MAX-VOLTAGE_MIN)*0.2 + VOLTAGE_MIN) ������ѹ20%  30.8v
#define VOLTAGE_EPS 0.3
volatile uint16_t ADC1Value = 0; //

void GetBatteryVoltage(void)
{
	static float Voltage_Reality = 0;  //���ʵ�ʵ�ѹ
	static float TempVoltageMin = 0;   //�ɼ�������С��ѹ
	float temp_V = 0;
	if(ADC1Value!=0)
	{
		Voltage_Reality = ADC1Value*VOLTAGE_SCALE*3.3f/4096;
		
		if(TempVoltageMin > Voltage_Reality||TempVoltageMin < VOLTAGE_MIN) //�ɼ����ĵ�ѹ����ǰС�ĵ�ѹ����Ϊ����Ч��ѹ��
		{
			temp_V = TempVoltageMin;
			TempVoltageMin = Voltage_Reality;		
 			if( !IsEqual( Voltage_Reality, temp_V, (float)VOLTAGE_EPS ))   //�������ֵ����VOLTAGE_EPS��������˴β���
			{
				TempVoltageMin = temp_V;
				if(temp_V  == 0) //��һ�ν�������С��ѹ��ֵ
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

void SetExternalCtrlCmd( CTRL_CMD_ENUM Cmd )
{
    ExternalCtrlCmd_g.Cmd = Cmd;
    //ExternalCtrlCmd_g.escapeTime = 0;
}
