#include "SPI.h"
#include "MyBspInc.h"
static SPI_TypeDef* SPI_TypeBuffer[2] = {SPI1, SPI2};
static GPIO_TypeDef* SPI_CS_PORT[2] = {SPI1_CS_GPIO_PORT, SPI2_CS_GPIO_PORT};
const u16 SPI_CS_PIN[2] = {SPI1_CS_PIN, SPI2_CS_PIN};
/*******************************************************************************
* Function Name	: SPI_GpioInit											
* Description   : ��ʼ��SPIʹ�õ�MISO,MOSI,SCK��������	
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : None													
* Return        : None
*				��NOne											
*******************************************************************************/
void SPI_GpioInit(u8 SPIx)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	if(SPIx == USE_SPI1)
	{
		// SPIx_CS_GPIO, SPIx_MOSI_GPIO, SPIx_MISO_GPIO and SPIx_SCK_GPIO Periph clock enable 
		RCC_APB2PeriphClockCmd(SPI1_CS_GPIO_CLK | SPI1_MOSI_GPIO_CLK | SPI1_MISO_GPIO_CLK |
		             			SPI1_SCK_GPIO_CLK, ENABLE);
		
		// SPI1 Periph clock enable 
		RCC_APB2PeriphClockCmd(SPI1_CLK, ENABLE); 
		
		
		// Configure SPI1 pins: SCK 
		GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStructure);
		
		//Configure SPI1 pins: MISO
		GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
		GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStructure);
		
		// Configure SPI1 pins: MOSI
		GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
		GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStructure);
	}
	else if(SPIx == USE_SPI2)
	{
		// SPIx_CS_GPIO, SPIx_MOSI_GPIO, SPIx_MISO_GPIO and SPIx_SCK_GPIO Periph clock enable 
		RCC_APB2PeriphClockCmd(SPI2_CS_GPIO_CLK | SPI2_MOSI_GPIO_CLK | SPI2_MISO_GPIO_CLK |
		             			SPI2_SCK_GPIO_CLK, ENABLE);
		
		//SPI2 Periph clock enable 
		RCC_APB1PeriphClockCmd(SPI2_CLK, ENABLE); 
		
		
		//Configure SPI2 pins: SCK 
		GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(SPI2_SCK_GPIO_PORT, &GPIO_InitStructure);
		
		// Configure SPI2 pins: MISO 
		GPIO_InitStructure.GPIO_Pin = SPI2_MISO_PIN;
		GPIO_Init(SPI2_MISO_GPIO_PORT, &GPIO_InitStructure);
		
		// Configure SPI2 pins: MOSI 
		GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN;
		GPIO_Init(SPI2_MOSI_GPIO_PORT, &GPIO_InitStructure);
	}
}


/*******************************************************************************
* Function Name	: SPI_SetSpiSoftCS											
* Description   : SPI NSS����ͨGPIOʹ��		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : None													
* Return        : None
*				��NOne											
*******************************************************************************/
void SPI_SetSpiSoftCS(u8 SPIx)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	
	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN[SPIx];
	GPIO_Init(SPI_CS_PORT[SPIx], &GPIO_InitStructure);
	
	SPI_SSOutputCmd(SPI_TypeBuffer[SPIx], ENABLE);	//ʹ���������NSS,����NSS��������ͨGPIOʹ��
}

/*******************************************************************************
* Function Name	: SPI_WriteByte											
* Description   : дһ���ֽ�  		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : u8 spi_mode		->	����SPI��ģʽ(ʱ�ӺͲ���)
					0					->	�����زɼ���������(ʱ�ӿ���Ϊ��)	
				 	1					->	�½��زɼ�����(ʱ�ӿ���Ϊ��)
				 	2					->	�½��زɼ�����(ʱ�ӿ���Ϊ��)
				 	3					->	�����زɼ���������(ʱ�ӿ���Ϊ��)				
* Return        : None
*				��NOne											
*******************************************************************************/
void SPI_MsterConfig(u8 SPIx, u8 spi_mode, u8 data_len)
{
	//����ģʽ,˫��ȫ˫��,����λ��ѡ(8/16bit),MSB
	//CPOL��CPHA�ɲ���spi_mode����
	SPI_InitTypeDef   SPI_InitStructure;
	switch(spi_mode)
	{
		case 0:		//�����زɼ���������(ʱ�ӿ���Ϊ��)
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case 1:		//�½��زɼ�����(ʱ�ӿ���Ϊ��)
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
			break;
		case 2:		//�½��زɼ�����(ʱ�ӿ���Ϊ��)
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case 3:		//�����زɼ���������(ʱ�ӿ���Ϊ��)
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
			break;
		default:
			break;
	}
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����ģʽ
	if(data_len == 8)
	{
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	//8λ����
	}
	else if(data_len == 16)
	{
		SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;	//16λ����
	}
	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;	//Ԥ��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//MSB
	SPI_InitStructure.SPI_CRCPolynomial = 7;			//CRC����ʽ,CRCĬ��Ϊ�ر�
	
	SPI_Init(SPI_TypeBuffer[SPIx], &SPI_InitStructure);	//��ʼ����Ӧ��SPIģ��
	SPI_Cmd(SPI_TypeBuffer[SPIx], ENABLE);				//ʹ��SPI
}
/*******************************************************************************
* Function Name	: SPI_WriteByte											
* Description   : дһ���ֽ�  		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : u8 tx_data		->	�����͵�����													
* Return        : u8 data			->	��MISO��ȡ������
*				��NOne											
*******************************************************************************/
u16 SPI_WriteByte(u8 SPIx, u16 tx_data)
{
  	u16 data = 0;
	if(SPIx == USE_SPI1)
  	{
  	 	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
  		{
  			;
		}
		/*!< Send the byte */
		SPI_I2S_SendData(SPI1, tx_data);
		
		/*!< Wait to receive a byte*/
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		{
		}
		/*!< Return the byte read from the SPI bus */ 
		data = SPI_I2S_ReceiveData(SPI1);
		return data;
  	}
	else if(SPIx == USE_SPI2)
	{
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
  		{
  			;
		}
  
		/*!< Send the byte */
		SPI_I2S_SendData(SPI2, tx_data);
		
		/*!< Wait to receive a byte*/
		while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
		{
		}
		/*!< Return the byte read from the SPI bus */ 
		data = SPI_I2S_ReceiveData(SPI2);
		return data;
	}
	return 0;
}
/*******************************************************************************
* Function Name	: SPI_ReadByte											
* Description   : ��һ���ֽ�		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : u8 dummy_data		->	dummy data(ͨ��Ϊ0xff)													
* Return        : u8 data			->	��MISO��ȡ������
*				��NOne											
*******************************************************************************/
u16 SPI_ReadByte(u8 SPIx, u16 dummy_data)
{
  	u16 data = 0;
  	if(SPIx == USE_SPI1)
  	{
  	 	/*!< Wait until the transmit buffer is empty */
	  	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
	  	{
	  	}
		/*!< Send the byte */
		SPI_I2S_SendData(SPI1, dummy_data);
		
		/*!< Wait until a data is received */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		{
		}
		/*!< Get the received data */
		data = SPI_I2S_ReceiveData(SPI1);
		
		/*!< Return the shifted data */
		return data;
  	}
	else if(SPIx == USE_SPI2)
	{
			/*!< Wait until the transmit buffer is empty */
	  	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	  	{
	  	}
		/*!< Send the byte */
		SPI_I2S_SendData(SPI2, dummy_data);
		
		/*!< Wait until a data is received */
		while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
		{
		}
		/*!< Get the received data */
		data = SPI_I2S_ReceiveData(SPI2);
		
		/*!< Return the shifted data */
		return data;
	}
	return 0;
}

/*******************************************************************************
* Function Name	: SPI_SetCS											
* Description   : SPI NSS����ͨGPIOʹ��,NSS����ߵ�ƽ 		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : None													
* Return        : None
*				��NOne											
*******************************************************************************/
void SPI_SetCS(u8 SPIx)
{	
	GPIO_SetBits(SPI_CS_PORT[SPIx], SPI_CS_PIN[SPIx]);
}
/*******************************************************************************
* Function Name	: SPI_ClrCS											
* Description   : SPI NSS����ͨGPIOʹ��,NSS����͵�ƽ 		
* Input         : u8 SPIx			->	��ʹ�õ�SPI 															
* 		        : None													
* Return        : None
*				��NOne											
*******************************************************************************/
void SPI_ClrCS(u8 SPIx)
{	
	GPIO_ResetBits(SPI_CS_PORT[SPIx], SPI_CS_PIN[SPIx]);
}

/*******************************************************************************
* Function Name	: SPI_SetSpeed											
* Description   : ����SPI�ٶ� 		
* Input         : u8 SPIx					->	��ʹ�õ�SPI															
* 		        : u8 baud_rate_prescaler 	-> 	��Ƶ��(SPI1���4��Ƶ,SPI2���2��Ƶ)												
* Return        : None			
*				��None										
*******************************************************************************/
void SPI_SetSpeed(u8 SPIx, u8 baud_rate_prescaler)
{
	uint16_t tmpreg = 0;

	u16 rate_buffer[] = 
					{
						SPI_BaudRatePrescaler_2,     
						SPI_BaudRatePrescaler_4,      
						SPI_BaudRatePrescaler_8,     
						SPI_BaudRatePrescaler_16,     
						SPI_BaudRatePrescaler_32,     
						SPI_BaudRatePrescaler_64,     
						SPI_BaudRatePrescaler_128,      
						SPI_BaudRatePrescaler_256,    
					};
					
	if(baud_rate_prescaler > 7)
	{
		baud_rate_prescaler = 7;
	}
	if(SPIx == USE_SPI1)
	{	
		SPI_Cmd(SPI1, DISABLE);
		tmpreg = SPI1->CR1;
		tmpreg &= 0XFFC7;
		tmpreg |=  rate_buffer[baud_rate_prescaler];
		SPI1->CR1 = tmpreg;
		SPI_Cmd(SPI1, ENABLE);
	}
	else if(SPIx == USE_SPI2)
	{
		SPI_Cmd(SPI2, DISABLE);
		tmpreg = SPI2->CR1;
		tmpreg &= 0XFFC7;
		tmpreg |=  rate_buffer[baud_rate_prescaler];
		SPI2->CR1 = tmpreg;
		SPI_Cmd(SPI2, ENABLE);
	}
}

void SPI_WriteReg(u16 reg, u16 val)
{
		u8 retry;
		reg |= 0x8000;
		while((SPI2->SR&1<<1)==0)//�ȴ���������	
		{
			retry++;
			if(retry>200)
			{
				break;
			}
		}		
		SPI2->DR = reg;
		while((SPI2->SR&1<<0)==0) //�ȴ�������һ��byte  
		{	
			;
		}	  						    
		reg = SPI2->DR;          //�����յ�������
		while((SPI2->SR&1<<1)==0)//�ȴ���������	
		{
			retry++;
			if(retry>200)
			{
				break;
			}
		}		
		SPI2->DR = val;
		while((SPI2->SR&1<<0)==0) //�ȴ�������һ��byte  
		{	
			;
		}	  						    
		reg = SPI2->DR;          //�����յ�������
}

void SPI_Test(u8 SPIx)
{
	u16 i = 0;
	u8 retry;
	u32 reg_val = 0;
	
	SPI_GpioInit(SPIx);
	SPI_SetSpiSoftCS(SPIx);
	SPI_MsterConfig(SPIx, 0, 16);
	SPI_SetSpeed(SPIx, 3);

	//WiFi_SpiWriteReg(0x70, 0x02);
	while(1)
	{
		//Wifi_SpiReadReg(0x6c, &reg_val);
		Delay_nUS(10);
	}
}


