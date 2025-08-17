#ifndef API_FLASH_H_
#define API_FLASH_H_

#include <stdint.h>

typedef enum
{
    FLASH_SECTOR_A = 0x00080000ul, // 0x_8_0000 - 0x_8_1FFF,  8K * 16bit
    FLASH_SECTOR_B = 0x00082000ul, // 0x_8_2000 - 0x_8_3FFF,  8K * 16bit
    FLASH_SECTOR_C = 0x00084000ul, // 0x_8_4000 - 0x_8_5FFF,  8K * 16bit
    FLASH_SECTOR_D = 0x00086000ul, // 0x_8_6000 - 0x_8_7FFF,  8K * 16bit
    FLASH_SECTOR_E = 0x00088000ul, // 0x_8_8000 - 0x_8_FFFF, 32K * 16bit
    FLASH_SECTOR_F = 0x00090000ul, // 0x_9_0000 - 0x_9_7FFF, 32K * 16bit
    FLASH_SECTOR_G = 0x00098000ul, // 0x_9_8000 - 0x_9_FFFF, 32K * 16bit
    FLASH_SECTOR_H = 0x000A0000ul, // 0x_A_0000 - 0x_A_7FFF, 32K * 16bit
    FLASH_SECTOR_I = 0x000A8000ul, // 0x_A_8000 - 0x_A_FFFF, 32K * 16bit
    FLASH_SECTOR_J = 0x000B0000ul, // 0x_B_0000 - 0x_B_7FFF, 32K * 16bit
    FLASH_SECTOR_K = 0x000B2000ul, // 0x_B_2000 - 0x_B_9FFF,  8K * 16bit
    FLASH_SECTOR_L = 0x000B4000ul, // 0x_B_4000 - 0x_B_BFFF,  8K * 16bit
    FLASH_SECTOR_M = 0x000B6000ul, // 0x_B_6000 - 0x_B_DFFF,  8K * 16bit
    FLASH_SECTOR_N = 0x000B8000ul, // 0x_B_8000 - 0x_B_FFFF,  8K * 16bit
}eApiFlashSector;

int ApiFlashInit(void);
int ApiFlashSectorErase(eApiFlashSector sector);
void ApiFlashRead(uint16_t* readAdr, uint16_t*datBuf, uint32_t len);
int ApiFlashWrite(uint16_t* writeAdr, uint16_t*datBuf, uint32_t len);

#endif
