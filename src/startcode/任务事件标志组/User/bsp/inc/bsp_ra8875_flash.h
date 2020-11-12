/*
*********************************************************************************************************
*
*	??????? :  RA8875зр????????Flash???????
*	??????? : bsp_ra8875_flash.h
*	??    ?? : V1.1
*	?    ?? : ????
*
*	Copyright (C), 2014-2015, ??????www.OS-Q.comm
*
*********************************************************************************************************
*/


#ifndef _BSP_RA8875_FLASH_H
#define _BSP_RA8875_FLASH_H

/* ???с─??Flash ID */
enum
{
	SST25VF016B = 0xBF2541,
	MX25L1606E  = 0xC22015,
	W25Q64BV    = 0xEF4017,
	W25Q128		= 0xEF4018
};

typedef struct
{
	uint32_t ChipID;		/* зр?ID */
	uint32_t TotalSize;		/* ?????? */
	uint16_t PageSize;		/* ????з│ */
}W25_T;

enum
{
	FONT_CHIP = 0,			/* ???зр? */
	BMP_CHIP  = 1			/* ???зр? */
};

/*
	????1? W25Q128???????
*/
#define PIC_OFFSET	(2*1024*1024)	/* ?????????????? */

void bsp_InitRA8875Flash(void);

void w25_CtrlByMCU(void);
void w25_CtrlByRA8875(void);
void w25_SelectChip(uint8_t _idex);

uint32_t w25_ReadID(void);
void w25_EraseChip(void);
void w25_EraseSector(uint32_t _uiSectorAddr);
void s25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
void w25_WritePage(uint8_t * _pBuf, uint32_t _uiWriteAddr, uint16_t _usSize);
void w25_ReadBuffer(uint8_t * _pBuf, uint32_t _uiReadAddr, uint32_t _uiSize);

extern W25_T g_tW25;

#endif

/***************************** ??????www.OS-Q.comm (END OF FILE) *********************************/
