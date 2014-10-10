/***************************** (C) COPYRIGHT  **********************************
* File Name          : SD_Driver.c
* Author             : zidong404(zxb)
* Version            : V1.2
* Date               : 6/12/2010
* Description        : This file contains the SD card and MMC card driver 
*                      functions.
*					 �����ļ�������SD����MMC����������,֧��SD V1��V2(SDHC������ͨ
*						�ֶ���Ѱַ��)�汾���Լ�MMC�������������ELM FATFSʹ�á�
********************************************************************************/
/*------------------------------------------------------------------------------/
/  Copyright (C) 2010, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/------------------------------------------------------------------------------*/

#include "sd_driver.h"
#include "OnChip_Driver_Inc.h"

/*--------------------------------------------------------------------------
****************************************************************************
*   						ģ��˽�к������궨��ͱ���,���ⲻ�ɼ�			*
****************************************************************************							
----------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* 						һ��ƽ̨��غ�����궨��					    	*/
/*--------------------------------------------------------------------------*/
#define SD_SPI			USE_SPI1

/* SPI Speed */ 
#define SPI_SPEED_LOW   	0
#define SPI_SPEED_HIGH  	1

#define xmit_spi(dat) 		SPI_WriteByte(SD_SPI,(dat))
#define rcvr_spi()			SPI_ReadByte(SD_SPI, 0xff)
#define rcvr_spi_m(p)		do{*(p) = SPI_ReadByte(SD_SPI, 0xff);}while(0)

/* SD�� ��MISO,MOSI,SCK���CSƬѡ,PWR���ƺ�INSERT�����˿ں궨�� */
#define SD_CS_ENABLE()		SPI_ClrCS(SD_SPI) 	  	// ѡ��SD��
#define SD_CS_DISABLE()     SPI_SetCS(SD_SPI) 		// ��ѡ��SD��

/********************************************************************			
* ���¿��ϵ磬��д������⣬����������Ӳ��δ�ṩ�򱣳ֲ��䣬		*
* �����滻Ϊ��ȷ�Ķ˿ڲ������� 										*
********************************************************************/

#define SD_PWR_ON()         	do{;}while(0)	// SD���ϵ�,����Ӳ��IO�˿�
#define SD_PWR_OFF()        	do{;}while(0)	// SD���ϵ磬����Ӳ��IO�˿�	
		
#define SD_IS_INSERTED()		(0)				// ��⿨�Ƿ���룺
												// ����Ӳ��IO�˿�0: ������  1: ���γ�
												
#define SD_IS_WR_PROTECTED()	(0)				// ��⿨�Ƿ�д������
												// ����Ӳ��IO�˿�0��δд����1: д����
/*******************************************************************************
* Function Name	: SD_SpiHardwareInit											
* Description   : SD_MMC ʹ��Ӳ��SPI��ʼ��(Mode 0),SPIƬѡ���ų�ʼ��						
* Input         : None															
* 		        : None															
* Return        : None		
*				��None													
*******************************************************************************/
void SD_SpiHardwareInit(void)
{
	SPI_GpioInit(SD_SPI);
	SPI_SetSpiSoftCS(SD_SPI);
	////////////////////////////////////////////
	//������д��PWR,WP,INSERT�Ķ˿ڳ�ʼ������///
	////////////////////////////////////////////
	SPI_MsterConfig(SD_SPI, 0, 8);	//ģʽ0,8λ����ģʽ
	SPI_SetSpeed(SD_SPI, 7);
}

/*******************************************************************************
* Function Name	: SD_SetSpiSpeed											
* Description   : ����SPI�ٶ�						
* Input         : speed_set		SPI_SPEED_LOW -> ����;SPI_SPEED_HIGH -> ����														
* 		        : None															
* Return        : None		
*				��None													
*******************************************************************************/
static void SD_SetSpiSpeed(BYTE speed_set)
{
    if(speed_set == SPI_SPEED_LOW)
    {
		SPI_SetSpeed(SD_SPI, 7);	//256��Ƶ
    }
    else
    {
		SPI_SetSpeed(SD_SPI, 1);	//4��Ƶ
    }
    return ;
}
/*--------------------------------------------------------------------------*/
/* 						����ƽ̨�޹غ�����궨��					    	*/
/*--------------------------------------------------------------------------*/

/* static local variables */
static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
static volatile UINT Timer1, Timer2;		/* 1ms decrement timer */
static UINT CardType;


/* MMC/SDC card type definitions (CardType) */
#define CT_MMC				0x01
#define CT_SD1				0x02
#define CT_SD2				0x04
#define CT_SDC				(CT_SD1|CT_SD2)
#define CT_BLOCK			0x08

/*Definitions for MMC/SDC command */
#define CMD0   (0)			/* GO_IDLE_STATE */
#define CMD1   (1)			/* SEND_OP_COND */
#define ACMD41 (41|0x80)	/* SEND_OP_COND (SDC) */
#define CMD8   (8)			/* SEND_IF_COND */
#define CMD9   (9)			/* SEND_CSD */
#define CMD10  (10)			/* SEND_CID */
#define CMD12  (12)			/* STOP_TRANSMISSION */
#define ACMD13 (13|0x80)	/* SD_STATUS (SDC) */
#define CMD16  (16)			/* SET_BLOCKLEN */
#define CMD17  (17)			/* READ_SINGLE_BLOCK */
#define CMD18  (18)			/* READ_MULTIPLE_BLOCK */
#define CMD23  (23)			/* SET_BLOCK_COUNT */
#define ACMD23 (23|0x80)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24  (24)			/* WRITE_BLOCK */
#define CMD25  (25)			/* WRITE_MULTIPLE_BLOCK */
#define CMD41  (41)			/* SEND_OP_COND (ACMD) */
#define CMD55  (55)			/* APP_CMD */
#define CMD58  (58)			/* READ_OCR */

/* CSƬѡ defines*/
#define CS_LOW()  			SD_CS_ENABLE()
#define CS_HIGH() 			SD_CS_DISABLE()

/* SPI speed defines */
#define	FCLK_SLOW()			SD_SetSpiSpeed(SPI_SPEED_LOW)/* Set slow clock (100k-400k) */
#define	FCLK_FAST()			SD_SetSpiSpeed(SPI_SPEED_HIGH)/* Set fast clock (depends on the CSD) */

/*******************************************************************************
* Function Name	: wait_ready											
* Description   : �ȴ�������,�ȴ�500ms��ʱ�˳�					
* Input         : None															
* 		        : None														
* Return        : 0xFF				->	������
*				��other				->	�ȴ���ʱ													
*******************************************************************************/
static BYTE wait_ready (void)
{
	BYTE res;

	Timer2 = 500;	/* Wait for ready in timeout of 500ms */
	rcvr_spi();
	do
	{
		res = rcvr_spi();
	}
	while ((res != 0xFF) && Timer2);

	return res;
}



/*******************************************************************************
* Function Name	: deselect											
* Description   : Deselect the card and release SPI bus		�ͷſ� 					
* Input         : None															
* 		        : None														
* Return        : None	
*				��None													
*******************************************************************************/
static void deselect (void)
{
	CS_HIGH();
	rcvr_spi();
}

/*******************************************************************************
* Function Name	: select											
* Description   : Select the card and wait ready 	ѡ�п�,���ȴ�������				
* Input         : None															
* 		        : None														
* Return        : 1					->	Successful 	�ɹ�	
*				��0					->	Timeout		�ȴ���ʱ													
*******************************************************************************/
static BYTE select (void)
{
	CS_LOW();
	if (wait_ready() != 0xFF) 
	{
		deselect();
		return 0;
	}
	return 1;
}

/*******************************************************************************
* Function Name	: power_on											
* Description   : Power Control		���ϵ�,����ͬƽ̨�޸ĵ��õĺ궨��
*	Note: 	When the target system does not support socket power control, there
*		is nothing to do in these functions and chk_power always returns 1.		
* Input         : None															
* 		        : None														
* Return        : None
*				��None													
*******************************************************************************/
static void power_on (void)
{
	SD_PWR_ON();
}

/*******************************************************************************
* Function Name	: power_off											
* Description   : Power Control		�ڿ��ϵ�֮ǰҪ�ȵȴ�������(wait redy)		
* Input         : None															
* 		        : None														
* Return        : None
*				��NOne													
*******************************************************************************/
static void power_off (void)
{
	select();			/* Wait for card ready */
	deselect();
	
	Stat |= STA_NOINIT;	/* Set STA_NOINIT */
}

/*******************************************************************************
* Function Name	: rcvr_datablock											
* Description   : Receive a data packet from SD_MMC	�ӿ��϶�ȡ���ݿ�(CMD֮�����)		
* Input         : BYTE *buff		->	Data buffer to store received data 															
* 		        : UINT btr			->	Byte count (must be multiple of 4) 														
* Return        : 1					->	OK
*				��0					->	Failed (Time out)													
*******************************************************************************/
static int rcvr_datablock(BYTE *buff, UINT btr)
{
	BYTE token;


	Timer1 = 100;
	do 
	{							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} 
	while ((token == 0xFF) && Timer1);

	if(token != 0xFE) 
	{							/* ������ʼ����	*/
		return 0;				/* If not valid data token, retutn with error */
	}
	do 
	{							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} 
	while (btr -= 4);
	rcvr_spi();					/* Dummy CRC */
	rcvr_spi();

	return 1;					/* Return with success */
}



/*******************************************************************************
* Function Name	: xmit_datablock											
* Description   : Send a data packet to MMC  		
* Input         : const BYTE *buff	->	512 byte data block to be transmitted 															
* 		        : BYTE token		->	Data token(0xFE/0xFC: ��ʼ����;0xFD: ��������)													
* Return        : 1					->	OK
*				��0					->	Failed(Timeout or data not accepted)											
*******************************************************************************/
static BYTE xmit_datablock (const BYTE *buff, BYTE token)
{
	BYTE resp;
	UINT bc = 512;


	if (wait_ready() != 0xFF) 
	{
		return 0;
	}

	xmit_spi(token);		/* Xmit a token */
	if (token != 0xFD) 
	{	/* Not StopTran token */
		do 
		{						/* Xmit the 512 byte data block to the MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} 
		while (bc -= 2);
		xmit_spi(0xFF);				/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();			/* Receive a data response */
		if ((resp & 0x1F) != 0x05)	/* If not accepted, return with error */
		{
			return 0;
		}	
	}

	return 1;
}

/*******************************************************************************
* Function Name	: xmit_datablock											
* Description   : Send a data packet to MMC  		
* Input         : BYTE cmd			->	Command byte 															
* 		        : DWORD arg			-> 	Argument 													
* Return        : R1				->	Return with the response value
*				��0xFF				->	Timeout	
*				: other				->	����������										
*******************************************************************************/
static BYTE send_cmd(BYTE cmd, DWORD arg)
{
	BYTE n, res;

	if (cmd & 0x80)
	{	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) 
		{
			return res;
		}
	}

	/* Select the card and wait for ready */
	deselect();
	if (!select()) 
	{
		return 0xFF;
	}
	/* Send command packet */
	xmit_spi(0x40 | cmd);			/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));	/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));	/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));		/* Argument[15..8] */
	xmit_spi((BYTE)arg);			/* Argument[7..0] */
	
	n = 0x01;						/* Dummy CRC + Stop */
	if (cmd == CMD0) 
	{
		n = 0x95;					/* Valid CRC for CMD0(0) */
	}
	if (cmd == CMD8)
	{
		n = 0x87;					/* Valid CRC for CMD8(0x1AA) */
	}
	xmit_spi(n);					/* CRC */

	/* Receive command response */
	if (cmd == CMD12) 
	{								/* �����CMD12(��������) ��������ֽ� */
		rcvr_spi();					/* Skip a stuff byte when stop reading */
	}
	
	n = 10;							/* Wait for a valid response in timeout of 10 attempts */
	do
	{								/* ����10�� */
		res = rcvr_spi();
	}
	while ((res & 0x80) && --n);

	return res;						/* Return with the response value */
}




/*--------------------------------------------------------------------------
****************************************************************************
   							ģ�����ӿں���
****************************************************************************							
----------------------------------------------------------------------------*/

/*******************************************************************************
* Function Name	: SD_Status											
* Description   : Get SD_MMC Status 		
* Input         : None															
* 		        : None													
* Return        : Stat			->	STA_NOINIT..STA_NODISK..STA_PROTECT
*				��None										
*******************************************************************************/
DSTATUS SD_Status(void)
{
	return Stat;
}

/*******************************************************************************
* Function Name	: SD_Init											
* Description   : Initialize Disk Drive    		
* Input         : None															
* 		        : None													
* Return        : Stat			->	STA_NOINIT..STA_NODISK..STA_PROTECT
*				��None										
*******************************************************************************/
DSTATUS SD_Init(void)
{
	BYTE n, cmd, ty, ocr[4];
	
	SD_SpiHardwareInit();					/* Init Hardware SPI */
	
	if (Stat & STA_NODISK) 
	{
		return Stat;						/* No card in the socket */
	}
	
	power_on();								/* Force socket power on */
	FCLK_SLOW();							/* Low speed */

	for (n = 20; n; n--) 
	{
		rcvr_spi();							/* 80 dummy clocks */
	}

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) 
	{			/* Enter Idle state */
		Timer1 = 1000;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) 
		{	/* SDv2? */
			for (n = 0; n < 4; n++) 
			{
				ocr[n] = rcvr_spi();			/* Get trailing return value of R7 resp */
			}
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) 
			{				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 0x40000000));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) 
				{			/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2|CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} 
		else 
		{							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	
			{
				ty = CT_SD1; 
				cmd = ACMD41;	/* SDv1 */
			} 
			else 
			{
				ty = CT_MMC; 
				cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));		/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set read/write block length to 512 */
			{
				ty = 0;
			}
		}
	}
	CardType = ty;
	deselect();

	if (ty) 
	{							/* Initialization succeded */
		Stat &= ~STA_NOINIT;	/* Clear STA_NOINIT */
		FCLK_FAST();
	} 
	else 
	{							/* Initialization failed */
		power_off();
	}

	return Stat;
}


/*******************************************************************************
* Function Name	: SD_Init											
* Description   : Read Sector(s)		->	��ȡ����(�������������)   		
* Input         : BYTE *buff			->  Pointer to the data buffer to store read data 															
* 		        : DWORD sector 			->	Start sector number (LBA) ��ʼ������
*				��BYTE count			->	Sector count (1..255) 												
* Return        : RES_NOTRDY			->	Card was not initialized
*				: RES_ERROR		
*				��RES_OK										
*******************************************************************************/
DRESULT SD_Read(BYTE *buff,	DWORD sector,BYTE count)
{
	
	if (Stat & STA_NOINIT) 
	{
		return RES_NOTRDY;
	}

	if (!(CardType & CT_BLOCK))
	{
		sector <<= 9;	/* Convert to byte address if needed: sector *= 512*/
	} 

	if (count == 1) 
	{					/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
		{
			count = 0;						/* read OK */
		}
	}
	else
	{										/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) 
		{									/* READ_MULTIPLE_BLOCK */
			do 
			{
				if (!rcvr_datablock(buff, 512)) 
				{
					break;					/* read not completed */
				}
				buff += 512;
			} 
			while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

/*******************************************************************************
* Function Name	: SD_Write											
* Description   : Write Sector(s)		->	д����(�������������)   		
* Input         : const BYTE *buff,		-> 	Pointer to the data to be written															
* 		        : DWORD sector 			->	Start sector number (LBA) ��ʼ������
*				��BYTE count			->	Sector count (1..255) 												
* Return        : RES_NOTRDY			->	Card was not initialized
*				: RES_WRPRT				->	write protected
*				: RES_ERROR		
*				��RES_OK										
*******************************************************************************/
DRESULT SD_Write(const BYTE *buff, DWORD sector, BYTE count)
{
	if(Stat & STA_NOINIT)
	{
		 return RES_NOTRDY;
	}
	if (Stat & STA_PROTECT) 
	{
		return RES_WRPRT;
	}

	if (!(CardType & CT_BLOCK))
	{					/* SDHC(SD_V2) was write in sector align */
		sector <<= 9;	/* Convert to byte address if needed: sector *= 512 */
	}

	if (count == 1) 
	{					/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
		{
			count = 0;
		}
	}
	else 
	{					/* Multiple block write */
		if (CardType & CT_SDC) 
		{				/* �����SD��,��ִ��Ԥ���� */
			send_cmd(ACMD23, count);
		}
		if (send_cmd(CMD25, sector) == 0) 
		{				/* WRITE_MULTIPLE_BLOCK */
			do 
			{
				if (!xmit_datablock(buff, 0xFC)) 
				{
					break;
				}
				buff += 512;
			} 
			while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
			{
				count = 1;
			}
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}


/*******************************************************************************
* Function Name	: SD_Ioctl											
* Description   : Miscellaneous Functions 	��Ϲ���(���ݲ�ͬ��ctrl��ȡ��ͬ����Ϣ) 		
* Input         : BYTE ctrl,			->	Control code 													
* 		        : void *buff			->	Buffer to send/receive data block 										
* Return        : RES_NOTRDY			->	Card was not initialized
*				��RES_OK										
*******************************************************************************/
DRESULT SD_Ioctl(
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive data block */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	DWORD csize;
	
	if (Stat & STA_NOINIT)
	{
		return RES_NOTRDY;
	} 

	res = RES_ERROR;
	switch (ctrl) 
	{
		case CTRL_SYNC :	/* Flush dirty buffer if present */
			if (select()) 
			{
				deselect();
				res = RES_OK;
			}
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (WORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) 
			{
				if ((csd[0]&0xC0)==0x40) 		//SD_V2:0100_0000b 	SD_V1:0000_0000b
				{	/* SDv2? */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} 
				else 
				{					/* SDv1 or MMCv2 */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = ((csd[8]&0xC0) >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 0x03) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get sectors on the disk (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sectors (DWORD) */
			if (CardType & CT_SD2) 
			{	/* SDv2? */
				if (send_cmd(ACMD13, 0) == 0) 
				{		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) 
					{				/* Read partial block */
						for (n = 64 - 16; n; n--) 
						{
							rcvr_spi();	/* Purge trailing data */
						}
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} 
			else 
			{					/* SDv1 or MMCv3 */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) 
				{	/* Read CSD */
					if (CardType & CT_SD1) 
					{	/* SDv1 */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} 
					else 
					{					/* MMCv3 */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case SD_MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case SD_MMC_GET_CSD :	/* Receive CSD as a data block (16 bytes) */
			if ((send_cmd(CMD9, 0) == 0)	/* READ_CSD */
				&& rcvr_datablock(buff, 16))
			{
				res = RES_OK;
			}
				
			break;

		case SD_MMC_GET_CID :	/* Receive CID as a data block (16 bytes) */
			if ((send_cmd(CMD10, 0) == 0)	/* READ_CID */
				&& rcvr_datablock(buff, 16))
			{
				res = RES_OK;
			}
			break;

		case SD_MMC_GET_OCR :	/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0)
			 {	/* READ_OCR */
				for (n = 0; n < 4; n++)
				{
					*((BYTE*)buff+n) = rcvr_spi();
				}
				res = RES_OK;
			}
			break;

		case SD_MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) 
			{	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(buff, 64))
				{
					res = RES_OK;
				}
			}
			break;

		default:
			res = RES_PARERR;
	}

	deselect();

	return res;
}


/*******************************************************************************
* Function Name	: SD_TimeoutHandle											
* Description   : Device Timer Interrupt Procedure  (Platform dependent)
* Note			: must be called in period of 1ms	��ʱ��1ms�����һ��		
* Input         : None										
* 		        : None									
* Return        : None
*				��None										
*******************************************************************************/
void SD_TimeoutHandler(void)
{
	BYTE s = 0;
	UINT cnt;

	cnt = Timer1;				/* 1000Hz decrement timer */
	if(cnt) 
	{			/* Timer1 ����������ʼ����ʱ�ͽ������ݿ鳬ʱ�� */
		Timer1 = --cnt;					
	}
	cnt = Timer2;						
	if (cnt) 
	{			/* Timer2 �����ڵȴ������г�ʱ�� */
		Timer2 = --cnt;	
	}
	
	s = Stat;
	if(SD_IS_WR_PROTECTED())	
	{
		s |= STA_PROTECT;		/* WP is H (write protected) */
	}
	else	
	{
		s &= ~STA_PROTECT;		/* WP is L (write enabled) */
	}					
		
	if(SD_IS_INSERTED())	
	{							/* INS = H (Socket empty) */
		s |= (STA_NODISK | STA_NOINIT);
	}
	else	
	{							/* INS = L (Card inserted) */
		s &= ~STA_NODISK;		
	}						
	
	Stat = s;
}

/*--------------------------------------------------------------------------*/
/* 					���º������ݾɰ汾SD_Driver.c�ĺ���					   	*/
/*--------------------------------------------------------------------------*/
unsigned long SD_GetCapacity(void)
{
	unsigned long sd_size = 0;
	if(SD_Ioctl(GET_SECTOR_COUNT, &sd_size) == RES_OK)
	{
		return sd_size;
	}
	return 0;
}

DRESULT SD_ReadSingleBlock(unsigned long sector, unsigned char *p_buff)
{
	return SD_Read(p_buff, sector, 1);
}

DRESULT SD_WriteSingleBlock(unsigned long sector, unsigned char *p_buff)
{
	return SD_Write(p_buff, sector,  1);
}
DRESULT SD_ReadMultiBlock(unsigned long sector, unsigned char *p_buff, unsigned char count)
{
	return SD_Read(p_buff, sector, count);
}
DRESULT SD_WriteMultiBlock(unsigned long sector, const unsigned char *p_buff, unsigned char count)
{
	return SD_Write(p_buff, sector, count);
}
/*--------------------------------------------------------------------------*/
/* 					���º���Ϊ���U���ٶȶ�����˶�д����			 	 	*/
/*--------------------------------------------------------------------------*/
///////////////////////////////////////////////////////
//write//
DWORD SD_WriteSectorStart(DWORD sector, BYTE count)
{
	if(Stat & STA_NOINIT)
	{
		 return RES_NOTRDY;
	}
	if (Stat & STA_PROTECT) 
	{
		return RES_WRPRT;
	}

	if (!(CardType & CT_BLOCK))
	{					
		sector <<= 9;	
	}
	
	if (count == 1) 
	{					/* Single block write */
		if (send_cmd(CMD24, sector) == 0)
		{
			count = 0;
		}
	}
	else 
	{					/* Multiple block write */
		if (CardType & CT_SDC) 
		{				/* �����SD��,��ִ��Ԥ���� */
			send_cmd(ACMD23, count);
		}
		if (send_cmd(CMD25, sector) == 0) 
		{
			count = 0;
		}
	}
	return count;
}
void SD_WriteSector(const BYTE *buff, BYTE count)
{
	if(count == 1)
	{
		xmit_datablock(buff, 0xFE);
	}
	else
	{
		xmit_datablock(buff, 0xFC);	
	}
}
void SD_WriteSectorEnd(BYTE count)
{
	if(count == 1)
	{
		deselect();
		return;
	}
	
	if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
	{
		deselect();
		return;
	}
	deselect();
}
////////////////////////////////////////////////////////////
//read//
DWORD SD_ReadSectorStart(DWORD sector, BYTE count)
{
	if (Stat & STA_NOINIT) 
	{
		return RES_NOTRDY;
	}

	if (!(CardType & CT_BLOCK))
	{
		sector <<= 9;	/* Convert to byte address if needed: sector *= 512*/
	} 

	if (count == 1) 
	{					/* Single block read */
		if (send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
		{
			count = 0;						/* read OK */
		}
	}
	else
	{										/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) 
		{									/* READ_MULTIPLE_BLOCK */
			count = 0;
		}
	}
	return count;
}

void SD_ReadSector(BYTE *buff)
{
	rcvr_datablock(buff, 512);
}

void SD_ReadSectorEnd(BYTE count)
{
	if(count == 1)
	{
		deselect();
		return;
	}
	
	send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
	deselect();
}
////////////////////////////////////////////////////////////
