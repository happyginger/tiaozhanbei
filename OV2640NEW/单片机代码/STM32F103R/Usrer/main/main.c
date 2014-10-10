/**
  ******************************************************************************
  * @file    GPIO/IOToggle/main.c 
  * @author  MCD Application Team
  * @version V3.3.0
  * @date    04/16/2010
  * @brief   Main program body.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include <stdio.h>

#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"

#include "MyBspInc.h"
#include "FATFS_008.h"

#include "OnChip_Driver_Inc.h"
#include "../BMP/BMP.h"



#include <stm32f10x.h>

#define sim900_rcc                    RCC_APB2Periph_GPIOB
#define sim900_gpio                   GPIOB
#define sim900_pin                    (GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_6)

#define led_rcc                    RCC_APB2Periph_GPIOA
#define led_gpio                   GPIOA
#define led_pin                    (GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7)


void init_gpio_func(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(sim900_rcc,ENABLE);
	RCC_APB2PeriphClockCmd(led_rcc,ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin   = sim900_pin;
    GPIO_Init(sim900_gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = led_pin;
	GPIO_Init(led_gpio, &GPIO_InitStructure);

}

void open_ov2640_3v3(void)
{
	GPIO_SetBits(sim900_gpio, GPIO_Pin_6);
	GPIO_ResetBits(sim900_gpio, GPIO_Pin_6);
	//
}




extern unsigned char VsyncCnt;			//��ͬ����־	
//////////////////////////////
FATFS fs;         			// Work area (file system object) for logical drives
FRESULT res;         		// FatFs function common result code
//////////////////////////////
/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
void Delay(__IO uint32_t nCount);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
//ע���˰�����ʼ�����Ͱ����жϳ�ʼ��


int KeyTest(void)
{
	u8 key_down;
		///////////////����PA0��ʼ��////////////////
	GpioInit(GPIOA, GPIO_Pin_0, GPIO_Mode_IPU, 0);
	////////////////////////////////////////////
	while(1)
	{
		if(!(GPIOA->IDR &0x01))
		{
			key_down = 1;
		}
	}	
	return 1;
}
extern u32 JpegDataCnt;
extern u8 JpegBuffer[10240];
extern u8 VsyncActive;
OV2640_IDTypeDef OV2640_Camera_ID;

extern u8 Key_Pressed;
u32 max_id = 0;
u32 min_id = 0xFFFFFFFF;
u32 max_sum = 0;

u8 UsartBuf[3];		//���ڽ��ջ���

/////////////////////////////////////////
//��˾���ƣ�����������Ƭ�洢�ļ��к���Ƭ��ǰ׺
#define COM_NAME_0		'S'
#define COM_NAME_1		'T'
#define COM_NAME_2		'M'
#define COM_FINAL_NAME	"0:STM"		//����ַ��������ϵ����ַ��ĺϼ�,����"0:"Ϊ�̶��ַ�����ʾ�ڸ�Ŀ¼��
//��˾���Ƶ��ַ�������
#define COM_NAME_LEN	3

extern u16* P_ImageBuf;

u8 PhotoName[] = {COM_NAME_0, COM_NAME_1, COM_NAME_2,'/',
				  COM_NAME_0, COM_NAME_1, COM_NAME_2,'_',
				 '1', '2', '3','4', '5',
				  '.','j','p','g','\0'};
static s32 CharToNumber(u8* pbuf, u8 buf_len)
{
	u32 ten[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};//���6�η�,7λ��
	u8 i = 0;
	s32 temp = 0;
	for(i = 0; i < buf_len; i ++)
	{
		temp += (pbuf[i] - '0')*ten[buf_len - 1-i];
	}
	return temp;
}				  
////////////////////////////
FRESULT scan_files (char* path, u32* pmax_id, u32* pmin_id, u32* psum)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;
	u16 i = 0;
	u16 temp_id = 0;
#if _USE_LFN
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif


    res = f_opendir(&dir, path);
    if (res == FR_OK) 
	{
        for (;;) 
		{
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) 
			{
				break;					//�������
			}
           
#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif

			
			if(fn[0] == COM_NAME_0 && fn[1] == COM_NAME_1 && fn[2] == COM_NAME_2 && fn[3] == '_')
			{
			#if USART_FILE_SCAN_EN		> 0	//����ú궨��ֵΪ1��ͨ����������ļ�������
				printf("%s\r\n", fn);
			#endif
				fn +=4;				//����ʵ�����ݴ�
				temp_id = (u16)CharToNumber(fn, 5);
				(*pmax_id) = ((*pmax_id) > temp_id)? (*pmax_id) : temp_id;//�ҵ������ļ�ID
				* pmin_id = ((*pmin_id) < temp_id)? (*pmin_id) : temp_id;
				(*psum) ++;			//���ļ�����1
			}
		}
    }
    return res;
}
const unsigned char NumberTable[] = {'0', '1', '2', '3','4', '5', '6', '7','8', '9', 
								'A', 'B', 'C', 'D', 'E', 'F'};
//�ú���ʵ�����ֵ��ַ���ת�����ַ���ĩβΪ�ӽ����ַ���\0��
void NumberToString(u32 data, u8* pbuf, u8 buf_len)
{
	u8 digit_sum =0;
	u8 digit = 0;
	//static unsigned char NumberBuffer[] = "0123456789";
	for(digit_sum = 0; digit_sum < buf_len; digit_sum ++)
	{
		digit = data%10;					//��ȡ���λ						
		data /= 10;							//ʮ��������						
		pbuf[buf_len - 1 - digit_sum]=NumberTable[digit];
	}
}
void SD_SavePhoto(u32 bmp_width, 
				  u32 bmp_height,
				  FIL* p_bmp,
				  const char *p_path,
			      u8 bmp_type)
{
}
void SD_SaveJpegPhoto(const u8* p_path, u8* jpg_buf, u32 jpg_size)
{
	FIL f_jpg;
	FRESULT res;
	u32 bw = 0;
	
	//�½�һ��jpg�ļ����ļ���Ϊp_path
	res = f_open(&f_jpg, p_path, FA_CREATE_ALWAYS | FA_WRITE);
	if(res)
	{
		res = f_close(&f_jpg);
		return;
	}
	
	res = f_write(&f_jpg, jpg_buf, jpg_size, &bw);
	f_close(&f_jpg);
}
void MenuSavePhoto(u32* p_max_id, u32* p_min_id, u32* p_max_sum)
{				
	(*p_max_id) ++;
	NumberToString((*p_max_id), &PhotoName[(COM_NAME_LEN<<1)+ 2], 5);
	
	SD_SaveJpegPhoto(PhotoName, JpegBuffer, JpegDataCnt);
	(*p_max_sum)++;	
}
void UsartSendData(const void* pbuf, u32 buf_size)
{
	u8* pidx = (u8*)pbuf;
	u32 i;
	for(i = 0; i < buf_size; i ++)
	{
		USART_Transmit(*pidx ++);
	}
}
void MenuUsartSendPhoto(u32 start_id, u32 max_id)
{
	FIL f_jpg;
	FRESULT res;
	u32 br = 0;
	while(1)
	{
		NumberToString(start_id, &PhotoName[(COM_NAME_LEN<<1)+ 2], 5);
		res = f_open(&f_jpg, PhotoName, FA_OPEN_EXISTING | FA_READ);
		//�ļ�ID������һ��
		if(start_id <= max_id)	
		{	//Ѱ����һ��ID���ļ�
			start_id ++;
		}
		else
		{	//�����Ѿ������������ļ���
			return;
		}
		
		if(res)
		{	//���ļ�ʧ�ܣ�������Ѱ��һ��
			continue;
		}
		//��ȡ�ļ�����
		res = f_read(&f_jpg, JpegBuffer, f_jpg.fsize, &br);
		f_close(&f_jpg);
		//ͨ�����ڽ����ݷ���
		UsartSendData(JpegBuffer, br);		//br����ʵ�ʶ�ȡ������
	}
}
u32 JPEG_Cnt  = 0;
int main(void)
{
	u16 i = 0;
	u16 j = 0;
	u16 temp = 0;
	u16 data = 0;
	
	u8 key_down = 0;
	u16 key_cnt = 0;
	//NVIC_Config(NVIC_PriorityGroup_0);
	//GPIO_DeInit(GPIOB);
	//GPIO_DeInit(GPIOC);
	///////////////Delay��ʼ��//////////////////		
	Delay_Init(72);
	////////////////////////////////////////////
	
	///////////////USART1��ʼ��/////////////////
	USART1_Init(115200);

	//init_gpio_func();
	//open_ov2640_3v3();
   	USART_SendString("uart init ok!\r\n");

#if 0
	UsartSendData("1\r\n",3);
	UsartSendData("2\r\n",3);
	////////////////////////////////////////////
	
	//ָʾ�Ƴ�ʼ��
	GpioInit(GPIOA, GPIO_Pin_1, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	GpioInit(GPIOA, GPIO_Pin_0, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	//������ʼ��
	GpioInit(GPIOA, GPIO_Pin_3, GPIO_Mode_IPU, 0);

	UsartSendData("3\r\n",3);
	///////////FTAFS��������ʼ��///////////////
	res = f_mount(0, &fs);
	res = f_mkdir(COM_FINAL_NAME);				//������˾���Ƶ��ļ����Դ洢ͼƬ
	scan_files("0:STM", &max_id, &min_id, &max_sum);

	if(max_sum == 0)
	{
		min_id = 1;
		max_id = 0;
	}
	GPIOA->ODR ^= (1 << 0);						//LED��˸һ��
	////////////////////////////////////////////
	
	////////////////OV2640��ʼ��////////////////
	//��λ
	/*
	GpioInit(GPIOC, GPIO_Pin_12, GPIO_Mode_Out_PP, GPIO_Speed_50MHz);
	SET_PORT_L(GPIOC, PC12);
	Delay_nMS(50);
	SET_PORT_H(GPIOC, PC12);
	Delay_nMS(50);
	*/
#endif
	OV2640_HW_Init();					//IIC��ʼ��
	memset(&OV2640_Camera_ID,0x0,sizeof(OV2640_IDTypeDef));
	USART_SendString("uart IIC ok!\r\n");

	OV2640_ReadID(&OV2640_Camera_ID);	//��ȡOV2640ID������Ӳ��������Ϊ:0x7F,0xA2,0x26,0x42

	OV2640_JPEGConfig(JPEG_320x240);	//����OV2640���320*240���ص�JPGͼƬ
	//�����Զ��ع�Ͱ�ƽ��
	OV2640_BrightnessConfig(0x20);
	OV2640_AutoExposure(2);



	OV2640_CaptureGpioInit();				//���ݲɼ����ų�ʼ��
	EXTI->IMR &= ~EXTI_Line8;				//�رճ�ͬ���ж�
	EXTI->EMR &= ~EXTI_Line8;	
	
	EXTI->IMR &= ~EXTI_Line15;				//�ر�����ͬ���ж�
	EXTI->EMR &= ~EXTI_Line15;	
	
	Delay_nMS(10);							//�ȴ�ͼ������ȶ�
	EXTI->IMR |= EXTI_Line8;				//ʹ�ܳ�ͬ���жϣ�׼���´βɼ�
	EXTI->EMR |= EXTI_Line8;


	
	
	while(1)
	{
		if(VsyncActive == 2)
		{
			//��֡��ʽΪ��0xAA CMD 0x55
			if(/*UsartBuf[0] == 0xAA && UsartBuf[1] == 0x02*/1)

				
			{	//�������һֱ��
				for(i = 0; i < JpegDataCnt; i ++)
				{
					USART_Transmit(JpegBuffer[i]);
				}
#if 0
				if (JpegDataCnt>0)
					USART_Transmit(JpegDataCnt);
#endif

			}
#if 0			
			if((GPIOA->IDR &  (1 << 3)) && key_down)
			{
				key_down = 0;
				if(JpegDataCnt > 1024)
				{	//��ֹ�󴥷����ɼ����㹻�����ݲŴ洢��Ƭ
					SET_PORT_H(GPIOA, PA1);
					MenuSavePhoto(&max_id, &min_id, &max_sum);
					SET_PORT_L(GPIOA, PA1);
					JPEG_Cnt ++;
				}
				
			}
#endif
		
			JpegDataCnt = 0;						//JPEG����������
			
			EXTI->IMR |= EXTI_Line8;				//ʹ�ܳ�ͬ���жϣ�׼���´βɼ�
			EXTI->EMR |= EXTI_Line8;
			
			//VsyncCnt = 0;				//��ʼ��һ֡���ݲɼ�				
		}
#if 0
		else
		{

			if(!(GPIOA->IDR &  (1 << 3)))		//��ⰴ��
			{
				key_down = 1;
			}
			
			if(USART_GetChar(UsartBuf, 3))
			{	//��֡��ʽΪ��0xAA CMD 0x55
				if(UsartBuf[0] == 0xAA && UsartBuf[1] == 0x01)
				{	//���յ���ȷ�Ĵ�������
					MenuUsartSendPhoto(min_id, max_id);
				}
			}

		}
#endif
	}
}

/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
