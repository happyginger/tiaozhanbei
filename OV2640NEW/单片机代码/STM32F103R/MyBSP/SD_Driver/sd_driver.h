/***************************** (C) COPYRIGHT  **********************************
* File Name          : SD_Driver.h
* Author             : zidong404(zxb)
* Version            : V1.2
* Date               : 6/12/2010
* Description        : This file contains all the interface functions prototypes 
*                      for the SD_Driver.c.
*					 �����ļ�������SD����MMC��������ӿں�������,����
*					 ��SD_TimeoutHandler()����Ϊ�û���ʱ���ж��е��ã���������Ϊ
*					 ��ELM FATFS�ṩ����SD��MMC�Ľӿ�
********************************************************************************/
#ifndef _SD_DRIVER_H_
#define _SD_DRIVER_H_

#include "stm32f10x.h"

#define AVR_8BIT_MCU_EN		0

#if AVR_8BIT_MCU_EN		> 0
//SD��SS��������					
#define MMC_SD_CS_DDR  DDRD
#define MMC_SD_CS_PORT PORTD
#define MMC_SD_CS_BIT  PD5

//SD��SPI_��������					
#define MMC_SD_SCK_DDR  SPI_DDR
#define MMC_SD_SCK_PORT SPI_PORT
#define MMC_SD_SCK_BIT  SPI_SCK

#define MMC_SD_MOSI_DDR  SPI_DDR
#define MMC_SD_MOSI_PORT SPI_PORT
#define MMC_SD_MOSI_BIT  SPI_MOSI

#define MMC_SD_MISO_DDR  SPI_DDR
#define MMC_SD_MISO_PORT SPI_PORT
#define MMC_SD_MISO_BIT  SPI_MISO

#endif

//////////////////////////////// Public Functions////////////////////////////////////
/* ����diskio.h */
#include "diskio.h"

/* ������������ΪELM FATFS�Ľ����ӿں��� */
DSTATUS SD_Status(void);

DRESULT SD_Write(
	const BYTE *buff,		/* Pointer to the data to be written */
	DWORD sector,			/* Start sector number (LBA) */
	BYTE count				/* Sector count (1..255) */
);
DRESULT SD_Read(
	BYTE *buff,				/* Pointer to the data buffer to store read data */
	DWORD sector,			/* Start sector number (LBA) */
	BYTE count				/* Sector count (1..255) */
);

DSTATUS SD_Init(void);

DRESULT SD_Ioctl(
	BYTE ctrl,				/* Control code */
	void *buff				/* Buffer to send/receive data block */
);

/********************************************************************
* ������������Ϊ��ʱ����ӿں������û���Ҫ��1KHZ��ʱ�ж��е��� 		*
* ����ú���û�е��ã��ڵ���SD_Driver�ṩ�Ľӿں���ʱ�翨����Ӳ��	*
* ���ϣ������ܵ���������ֲ���Ԥ�ϵĽ���Ӷ��������ܷ���			*
********************************************************************/
void SD_TimeoutHandler(void);

/* ������������Ϊ���ݾɰ汾SD������������,ELM FATFS������ */ 
unsigned long SD_GetCapacity(void); 													//��ȡ����
DRESULT SD_ReadSingleBlock(unsigned long sector, unsigned char *p_buff);             	//��һ��sector
DRESULT SD_WriteSingleBlock(unsigned long sector, unsigned char *p_buff);      			//дһ��sector
DRESULT SD_ReadMultiBlock(unsigned long sector, unsigned char *p_buff, unsigned char count);    //�����sector
DRESULT SD_WriteMultiBlock(unsigned long sector, const unsigned char *p_buff, unsigned char count);//д���sector

//////////////////
void SD_WriteSectorEnd(BYTE count);
DWORD SD_WriteSectorStart(DWORD sector, BYTE count);
void SD_WriteSector(const BYTE *buff, BYTE count);
//////////////////
//////////////////
DWORD SD_ReadSectorStart(DWORD sector, BYTE count);
void SD_ReadSector(BYTE *buff);
void SD_ReadSectorEnd(BYTE count);
/////////////////
#endif
