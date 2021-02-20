#include "flash.h"
#include "flash.h"   
#include "stdio.h"
#include "user.h"

static uint32_t GetSector(uint32_t Address);

/**
  * @brief  InternalFlash_Test,���ڲ�FLASH���ж�д����
  * @param  None
  * @retval None
  */
uint8_t WriteFlashData(uint32_t start_addr,uint32_t end_addr,uint32_t *pdata,uint8_t len)
{
	/*Ҫ��������ʼ����(����)����������(������)����8-12����ʾ����8��9��10��11����*/
	uint32_t FirstSector = 0;
	uint32_t NbOfSectors = 0;
	
	uint32_t SECTORError = 0;
	
	uint32_t Address = 0;
	uint32_t Sum_Cheak_Write = 0; // д��ʱ��У��
	
// 	volatile uint32_t Data32 = 0;
// 	volatile uint32_t MemoryProgramStatus = 0;
	static FLASH_EraseInitTypeDef EraseInitStruct;
	
	/* FLASH ���� ********************************/
	/* ʹ�ܷ���FLASH���ƼĴ��� */
	HAL_FLASH_Unlock();

	FirstSector = GetSector(start_addr);
	NbOfSectors = GetSector(end_addr)- FirstSector + 1;
	
	/* �����û����� (�û�����ָ������û��ʹ�õĿռ䣬�����Զ���)**/
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;/* �ԡ��֡��Ĵ�С���в��� */ 
	EraseInitStruct.Sector        = FirstSector;
	EraseInitStruct.NbSectors     = NbOfSectors;
	/* ��ʼ�������� */
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK)
	{
		/*�����������أ�ʵ��Ӧ���пɼ��봦�� */
		return 1;
	}
	/* �ԡ��֡��Ĵ�СΪ��λд������ ********************************/
	Address = start_addr;
	
	for(int i = 0; i<len-1; i++)    //����У��ֵ
	{
		Sum_Cheak_Write = Sum_Cheak_Write + *(pdata+i);
	}
	*(pdata+7) = Sum_Cheak_Write;
	for(int i = 0; i<len; i++)  //д��Flash
	{
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, *(pdata+i)) == HAL_OK)
		{
		  Address = Address + 4;
		}
	  printf("addr is:0x%x, data is:0x%02x\n", start_addr + i*4, *(pdata+i));
	}
	/* ��FLASH��������ֹ���ݱ��۸�*/
	HAL_FLASH_Lock(); 
	return 0;

}

/**********FLASH��ȡ��ӡ����***********/
//����ֵΪ0��ʾ��ȡ�ɹ�
uint8_t ReadFlashData(uint32_t start_addr)
{
	uint32_t Sum_Cheak_Read = 0; // ��ȡʱ��У��

	for(int i = 0; i<ElementOfArray(ReadFlashBuffer); i++)
	{
		ReadFlashBuffer[i] = *(__IO uint32_t*)(start_addr + i*4 );
    if(i < ElementOfArray(ReadFlashBuffer)-1)	
		{
			Sum_Cheak_Read = Sum_Cheak_Read + *(__IO uint32_t*)(start_addr + i*4);
    }
	}
	if(Sum_Cheak_Read != *(__IO uint32_t*)(start_addr + 7*4))
	{
		//printf("Verification failed\n");
		return 1;     //У��ʧ��
	}
  return 0;   //У��ɹ�
		 
}
/**
  * @brief  ��������ĵ�ַ���������ڵ�sector
  *					���磺
						uwStartSector = GetSector(FLASH_USER_START_ADDR);
						uwEndSector = GetSector(FLASH_USER_END_ADDR);	
  * @param  Address����ַ
  * @retval ��ַ���ڵ�sector
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_SECTOR_7;  
  }
  return sector;
}

/*��ӡFlash�е�����*/
void PrintfUserFlash(void)
{
	ReadFlashData(FLASH_ID_START_ADDR);
	printf("The device code of this machine is :");
	for(int i = 0; i < WRITE_FLASH_ID_SIZE-4; i++)
	{
		printf("0x%02x, ",ReadFlashBuffer[i]);
	}
	printf("\n");
//	printf("The user's data is :");
//	ReadFlashData(FLASH_DATA_START_ADDR);
//	for(int i = 0;i < ElementOfArray(ReadFlashBuffer)-1; i++)  
//	{
//	  printf("0x%02x, ",ReadFlashBuffer[i]);
//	}
//	printf("\n");
}


