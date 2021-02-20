#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f4xx.h"

#define FLASH_ID_START_ADDR   	((uint32_t)0x08040000)
#define FLASH_ID_END_ADDR     	((uint32_t)0x08040010)
#define FLASH_DATA_START_ADDR  	((uint32_t)0x08060000)   
#define FLASH_DATA_END_ADDR     ((uint32_t)0x08060100)  

/*写入Flash的长度*/

/* Base address of the Flash sectors */ 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */

uint8_t WriteFlashData(uint32_t start_addr,uint32_t end_addr,uint32_t *pdata,uint8_t len);

uint8_t ReadFlashData(uint32_t start_addr);
void PrintfUserFlash(void);
#endif
