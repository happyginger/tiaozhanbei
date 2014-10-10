/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : memory.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Memory management layer
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
/////////////////////////////////
//memory����һ��Ϊmass_mal
/////////////////////////////////
/* Includes ------------------------------------------------------------------*/

#include "memory.h"
#include "usb_scsi.h"
#include "usb_bot.h"
#include "usb_regs.h"
#include "usb_mem.h"
#include "usb_conf.h"
#include "hw_config.h"
#include "mass_mal.h"
#include "usb_lib.h"

//#include "stm32_eval.h"

#include "sd_driver.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t Block_Read_count = 0;
__IO uint32_t Block_offset;
__IO uint32_t Counter = 0;
uint32_t  Idx;
uint32_t Data_Buffer[BULK_MAX_PACKET_SIZE *2]; /* 512 bytes*/
uint8_t TransferState = TXFR_IDLE;
/* Extern variables ----------------------------------------------------------*/
extern uint8_t Bulk_Data_Buff[BULK_MAX_PACKET_SIZE];  /* data buffer*/
extern uint16_t Data_Len;
extern uint8_t Bot_State;
extern Bulk_Only_CBW CBW;
extern Bulk_Only_CSW CSW;
extern uint32_t Mass_Memory_Size[2];
extern uint32_t Mass_Block_Size[2];

////////////////////////////////////////
#define SECTOR_SIZE		512
#define SECTOR_CNT		8
#define BUF_SIZE		SECTOR_SIZE*SECTOR_CNT
static u8 SD_Buffer[BUF_SIZE];
///////////////////////////////////////

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Read_Memory1(uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{
  static uint32_t Offset, Length;

  if (TransferState == TXFR_IDLE )
  {
    Offset = Memory_Offset * Mass_Block_Size[lun];	//������ַת��Ϊ�ֽڵ�ַ(SDHC ��Ҫ�޸�)
    Length = Transfer_Length * Mass_Block_Size[lun];//�鳤��ת��Ϊ�ֽڳ���
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {
    if (!Block_Read_count)			//Block_Read_count = 0
    {								//��ȡһ����(��SD��˵���СΪһ�������Ĵ�С)	
      MAL_Read(lun ,
               Offset ,
               Data_Buffer,
               Mass_Block_Size[lun]);
	//�������BULK_MAX_PACKET_SIZE(64)���ֽ�����
      USB_SIL_Write(EP1_IN, (uint8_t *)Data_Buffer, BULK_MAX_PACKET_SIZE);
	//512-64
      Block_Read_count = Mass_Block_Size[lun] - BULK_MAX_PACKET_SIZE;
      Block_offset = BULK_MAX_PACKET_SIZE;	//����ƫ������64
    }
    else
    {//��������ʣ���(512-64)�ֽ�����
      USB_SIL_Write(EP1_IN, (uint8_t *)Data_Buffer + Block_offset, BULK_MAX_PACKET_SIZE);

      Block_Read_count -= BULK_MAX_PACKET_SIZE;
      Block_offset += BULK_MAX_PACKET_SIZE;
    }
	//������������˵��С
    SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);
#ifndef USE_STM3210C_EVAL
    SetEPTxStatus(ENDP1, EP_TX_VALID);		//����ʹ�ܣ���ʼ��������
#endif    
    Offset += BULK_MAX_PACKET_SIZE;			//�ֽڵ�ַƫ������64
    Length -= BULK_MAX_PACKET_SIZE;			//�����ͳ��ȼ�ȥ64

    CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;	//�޸�CSW��ʣ���ֽ���
    Led_RW_ON();							//ָʾ����
  }
  if (Length == 0)
  {											//������ݷ������
    Block_Read_count = 0;					//Block_Read_count ��0
    Block_offset = 0;						//����ƫ����0
    Offset = 0;								//�ֽڵ�ַƫ����0
    Bot_State = BOT_DATA_IN_LAST;			//BOT״̬ΪBOT_DATA_IN_LAST,�����������������ж����CSW
    TransferState = TXFR_IDLE;				//����״̬����
    Led_RW_OFF();
  }
}
void Read_Memory2(uint8_t lun, uint32_t sector, uint32_t sector_cnt)
{
  	static uint32_t Offset, Length;
	
	static u16 DataLen = 0;					//�Ӵ洢��ʵ�ʶ��������ݴ�С
	static u8 Cnt = 0;						//�洢��������������С��������
	static u8 CntLast = 0;					//�洢��������������С������
	static u8 MemorReadFlag = 0;			//���洢����־
	static u8 TransOver = 0;				//������ɱ�־
	static u8* pTemp;						
	
  	if (TransferState == TXFR_IDLE )
  	{
	    Offset = sector;					//������ַת��Ϊ�ֽڵ�ַ(SDHC ��Ҫ�޸�)
	    
		TransferState = TXFR_ONGOING;
		
		Cnt = sector_cnt/SECTOR_CNT;		//��ȡ����������
		CntLast = sector_cnt%SECTOR_CNT;	//��ȡ����������
		
		MemorReadFlag = 1;					//��1,��һ��Ҫ���洢��
		TransOver = 0;
	
  	}
  	if (TransferState == TXFR_ONGOING )
  	{
		if(MemorReadFlag)
		{								//���洢��
			if(Cnt)						
			{							//�ȶ���������
				SD_Read(SD_Buffer, Offset, SECTOR_CNT);
				Offset += SECTOR_CNT;	//����ƫ������
				Cnt -= 1;				//�޸��������ֵĴ�С
				DataLen = BUF_SIZE;		//��ȡ��������
				
			}
			else if(CntLast)			
			{							//����������������
				SD_Read(SD_Buffer, Offset, CntLast);
				Offset += CntLast;
				DataLen = CntLast*512;
				CntLast = 0;			//����
			}
			
			MemorReadFlag = 0;			//�޸ı�־,������ͨ��USB��������
			pTemp = SD_Buffer;
			Length = 0;
			if((Cnt == 0) && (CntLast == 0))
			{							//����������������ֶ�Ϊ0,˵�������Ѿ�����
				TransOver = 1;			//��־��1
			}
		}
		if(!MemorReadFlag)
		{									//�������BULK_MAX_PACKET_SIZE(64)���ֽ�����
	      	USB_SIL_Write(EP1_IN, (u8*)pTemp, BULK_MAX_PACKET_SIZE);
			pTemp += BULK_MAX_PACKET_SIZE;	//�޸�ָ��,׼����һ�δ���
			
			SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);	//������������˵��С
		#ifndef USE_STM3210C_EVAL
			SetEPTxStatus(ENDP1, EP_TX_VALID);			//����ʹ��,��ʼ��������
		#endif    
			CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;	//�޸�CSW��ʣ����
			
			Length += BULK_MAX_PACKET_SIZE;				//�޸��Ѿ�ͨ��USB���͵�������
			if(Length == DataLen)
			{								//���USB���͵����������ڴӴ洢������������
				MemorReadFlag = 1;			//��־��1,׼����һ�δӴ洢����ȡ����
				if(TransOver == 1)			
				{							//����Ѿ�����������������
					TransOver = 2;			//�������
				}
			}
		}
		Led_RW_ON();
	}

	if(TransOver == 2)
	{										//�������
		Bot_State = BOT_DATA_IN_LAST;		//BOT״̬ΪBOT_DATA_IN_LAST,�����������������ж����CSW
		TransferState = TXFR_IDLE;			//����״̬����
		Led_RW_OFF();
		TransOver = 0;
	}
}
/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Write_Memory1(uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{

  static uint32_t W_Offset, W_Length;

  uint32_t temp =  Counter + 64;						//64�ֽ�

  if (TransferState == TXFR_IDLE )
  {											
    W_Offset = Memory_Offset * Mass_Block_Size[lun];	//ת��Ϊ�ֽڵ�ַ(SDHC��Ҫ�޸�)
    W_Length = Transfer_Length * Mass_Block_Size[lun];	//ת��Ϊ�ֽڳ���
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {

    for (Idx = 0 ; Counter < temp; Counter++)
    {													//���û������˵㻺������ȡ64�ֽ�
      *((uint8_t *)Data_Buffer + Counter) = Bulk_Data_Buff[Idx++];
    }

    W_Offset += Data_Len;			//Data_LenΪBulk_Data_Buff�е���Ч���ݳ���,���64�ֽ�
    W_Length -= Data_Len;			//��д��ĳ��ȼ�ȥData_Len

    if (!(W_Length % Mass_Block_Size[lun]))
    {								//�����д�볤���ܱ�512����,��д��512�ֽڵ�SD��
      Counter = 0;					//Counter��0(Counter��ֵ��0���ӵ�512����64�ֶ�)
      MAL_Write(lun ,
                W_Offset - Mass_Block_Size[lun],	//�������Ϊ�ֽ�ƫ��,Ӧ�ø���Ϊ����ƫ��
                Data_Buffer,
                Mass_Block_Size[lun]);
    }
	

    CSW.dDataResidue -= Data_Len;	//�޸�CSW��ʣ���ֽ���
  #ifndef STM32F10X_CL				//ʹ����һ�ν���
    SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/   
  #endif /* STM32F10X_CL */

    Led_RW_ON();
  }

  if ((W_Length == 0) || (Bot_State == BOT_CSW_Send))//
  {									//��������Ѿ�������ϲ���BOTΪBOT_CSW_Send����CSW
    //////////////////////////////////
	if(Bot_State == BOT_CSW_Send)	//����ʹ��,����BOT��״̬ΪBOT_CSW_Send�������
	{
		//STM_EVAL_LEDOff(LED2);
		LED_Off(LED2);
	}
	/////////////////////////////////
	Counter = 0;
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
	
  }
}

//�жϵ���
void Write_Memory2(uint8_t lun, uint32_t sector, uint32_t sector_cnt)
{
	u8 i = 0;
  	static uint32_t W_Offset, W_Length;
	static u32 DataCnt = 0;

  	if(TransferState == TXFR_IDLE )
  	{											
		W_Offset = sector;
		W_Length = sector_cnt*Mass_Block_Size[lun];	//��ȡ�ֽڳ���
		TransferState = TXFR_ONGOING;
  	}
  	if(TransferState == TXFR_ONGOING )
  	{
		for(i = 0; i < 64; i ++)
		{											//��ȡһ���������������
			SD_Buffer[DataCnt + i] =  Bulk_Data_Buff[i];	
		}
		DataCnt += Data_Len;						//��¼һ��ʵ�ʴ���SD_Buffer��������
		W_Length -= Data_Len;						//��ȥһ���������䳤��
		
		if(!(W_Length % BUF_SIZE))					//�ж��Ƿ񿽱���BUF_SIZE��С������
		{
			DataCnt /= SECTOR_SIZE;					//ת��Ϊ������
			SD_Write(SD_Buffer, W_Offset, DataCnt);	//��SD��д������
			W_Offset += DataCnt;
			DataCnt = 0;
		}
		
		CSW.dDataResidue -= Data_Len;				//�޸�CSW��ʣ���ֽ���
		
		#ifndef STM32F10X_CL				
		SetEPRxStatus(ENDP2, EP_RX_VALID);			//ʹ����һ�ν���
		#endif /* STM32F10X_CL */
		
		Led_RW_ON();
  	}

  	if((W_Length == 0) || (Bot_State == BOT_CSW_Send))//
  	{									//��������Ѿ�������ϲ���BOTΪBOT_CSW_Send����CSW
    	//////////////////////////////////
		if(Bot_State == BOT_CSW_Send)	//����ʹ��,����BOT��״̬ΪBOT_CSW_Send�������
		{
			//STM_EVAL_LEDOff(LED2);
			LED_Off(LED2);
		}
		/////////////////////////////////
    	Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    	TransferState = TXFR_IDLE;
    	Led_RW_OFF();
  	}
}

//��memory,����(��)��СΪ512�ֽ�
void Read_Memory(uint8_t lun, uint32_t sector, uint32_t sector_cnt)
{
  	static uint32_t DataCnt;
	
	static u32 SectorCnt = 0;						
	static u32 SectorCntTemp = 0;		
	
	static u8 MemorReadFlag = 0;			//���洢����־
	static u8 TransOver = 0;				//������ɱ�־
	static u8* pTemp;						
	
  	if (TransferState == TXFR_IDLE )
  	{ 
		TransferState = TXFR_ONGOING;

		MemorReadFlag = 1;					//��1,��һ��Ҫ���洢��
		TransOver = 0;
		
		SD_ReadSectorStart(sector, sector_cnt);	//���Ͷ�����,׼��������
		
		SectorCnt = sector_cnt;
		SectorCntTemp = sector_cnt;
  	}
  	if (TransferState == TXFR_ONGOING )
  	{
		if(MemorReadFlag)
		{								//���洢��
			SD_ReadSector(SD_Buffer);	//�ڶ���:��memory�е�����
			
			pTemp = SD_Buffer;
			DataCnt = 0;				//���������0
			MemorReadFlag = 0;			//�޸ı�־,������ͨ��USB��������
			
			SectorCnt --;
			if(SectorCnt == 0)
			{
				TransOver = 1;			//��־��1
			}
		}
		if(!MemorReadFlag)
		{									//�������BULK_MAX_PACKET_SIZE(64)���ֽ�����
	      	USB_SIL_Write(EP1_IN, (u8*)pTemp, BULK_MAX_PACKET_SIZE);
			pTemp += BULK_MAX_PACKET_SIZE;	//�޸�ָ��,׼����һ�δ���
			
			SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);	//������������˵��С
		#ifndef USE_STM3210C_EVAL
			SetEPTxStatus(ENDP1, EP_TX_VALID);			//����ʹ��,��ʼ��������
		#endif    
			CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;	//�޸�CSW��ʣ����
			
			DataCnt += BULK_MAX_PACKET_SIZE;			//�޸��Ѿ�ͨ��USB���͵�������
			if(DataCnt == 512)
			{							//���USB���͵����������ڴӴ洢������������
				MemorReadFlag = 1;		//��־��1,׼����һ�δӴ洢����ȡ����
				if(TransOver == 1)			
				{						//����Ѿ�����������������
					TransOver = 2;		//�������
				}
			}
		}
		Led_RW_ON();
	}

	if(TransOver == 2)
	{									//�������
		SD_ReadSectorEnd(SectorCntTemp);//������:�����������Ľ�������,�ر�Ƭѡ
		
		Bot_State = BOT_DATA_IN_LAST;	//BOT״̬ΪBOT_DATA_IN_LAST,�����������������ж����CSW
		TransferState = TXFR_IDLE;		//����״̬����
		Led_RW_OFF();
		TransOver = 0;
	}
}
//�жϵ���
void Write_Memory(uint8_t lun, uint32_t sector, uint32_t sector_cnt)
{
  	static uint32_t W_Length;
	static u16 DataCnt = 0;
	static u8 Count = 0;
	u8 i = 0;
  	if(TransferState == TXFR_IDLE )
  	{											
		W_Length = sector_cnt*Mass_Block_Size[lun];	//��ȡ�ֽڳ���
		TransferState = TXFR_ONGOING;
		Count = sector_cnt;
		SD_WriteSectorStart(sector, sector_cnt);//��һ��:��������,����д����,��ʼд
  	}
  	if(TransferState == TXFR_ONGOING )
  	{
		for(i = 0; i < 64; i ++)
		{											//��ȡһ���������������
			SD_Buffer[DataCnt + i] =  Bulk_Data_Buff[i];	
		}
		DataCnt += Data_Len;						//��¼һ��ʵ�ʴ���SD_Buffer��������
		W_Length -= Data_Len;						//��ȥһ���������䳤��
		
		if(!(W_Length & ((1 << 9) - 1)))			//�ж��Ƿ񿽱���512�ֽڴ�С������
		{											//ÿ��д512�ֽ�,�ڼ䲻�ر�Ƭѡ
			DataCnt = 0;
			SD_WriteSector(SD_Buffer, Count);	//�ڶ���:д������!!!
		}
		
		CSW.dDataResidue -= Data_Len;				//�޸�CSW��ʣ���ֽ���
		
		#ifndef STM32F10X_CL				
		SetEPRxStatus(ENDP2, EP_RX_VALID);			//ʹ����һ�ν���
		#endif /* STM32F10X_CL */
		
		Led_RW_ON();
  	}

  	if((W_Length == 0) || (Bot_State == BOT_CSW_Send))
  	{												//��������Ѿ�������ϲ���BOTΪBOT_CSW_Send����CSW
		SD_WriteSectorEnd(Count);				//������:����д��������ر�Ƭѡ!!!
		
    	Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    	TransferState = TXFR_IDLE;
    	Led_RW_OFF();
  	}
}
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

