# include "FIFO.h"
void FIFO2_PortInit(void);
void FIFO_PortInit(void)
{
	RCC->APB2ENR |= BIT(3);				//PBʱ��ʱ��
	GPIOB->CRL &= 0xfffffff0;
	GPIOB->CRL |= 0x00000003;			//PB0 FIFO_WEN
	GPIOB->ODR |= BIT(0);

	GPIOB->CRL &= 0xff0fffff;			//PB5 FIFO_WRST
	GPIOB->CRL |= 0x00300000;			
	GPIOB->ODR |= BIT(5);
	
	GPIOB->CRL &= 0xfffff0ff;
	GPIOB->CRL |= 0x00000300;			//PB2 FIFO_RRST
	GPIOB->ODR |= BIT(2);
	
	GPIOB->CRH = 0x88888888;			//PB8-15 ����
	GPIOB->ODR |= 0xff00;				//����
	
	RCC->APB2ENR |= BIT(2);				//PAʱ��ʱ��
	GPIOA->CRL &= 0xfffff0ff;			//PA2 FIFO_RCLK
	GPIOA->CRL |= 0x00000300;
	GPIOA->ODR |= BIT(2);
	
	GPIOA->CRL &= 0xffffff0f;			//PA1 FIFO_OE
	GPIOA->CRL |= 0x00000030;
	GPIOA->ODR |= BIT(1);
}

void FIFO_WrRdReset(void)
{
	FIFO_RRST_L;				//��ָ�븴λ						
	FIFO_RCLK_L;				//������Ҫһ��ʱ�����ڵ�������ܸ�λ
	FIFO_RCLK_H;
	FIFO_RCLK_L;				//������Ҫһ��ʱ�����ڵ�������ܸ�λ
	FIFO_RCLK_H;
	FIFO_RRST_H;
								//дָ�븴λ
	FIFO_WEN_L;					//�����ֲ���˵��,��λʱWENҪ�ø�,�� 
	FIFO_WRST_L;				//MCU��WEN��HREF��������߼���������								
	__nop();					//��AL422B��,����MCU��WEN�õ��������422��WENһ��Ϊ��
	__nop();					//дָ�븴λ��Ҫһ������ʱ			
	FIFO_WRST_H;
}
u8 FIFO_GetData(void)
{
	u8 temp = 0;
	FIFO_DATA_IN;
	temp = FIFO_DATA_PIN;
	FIFO_DATA_OUT;
	return temp;
}
