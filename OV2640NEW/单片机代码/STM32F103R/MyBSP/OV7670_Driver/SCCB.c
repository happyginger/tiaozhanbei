
/*

wangguanfu
2009-08-06

*/


#include "SCCB.h"
#include "MyBspInc.h"

/*
-----------------------------------------------
   ����: ��ʼ��ģ��SCCB�ӿ�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void InitSCCB(void)
{
	//SCCB_DDR|=(1<<SCCB_SIO_C)|(1<<SCCB_SIO_D);
	//SCCB_PORT|=(1<<SCCB_SIO_C)|(1<<SCCB_SIO_D);
	RCC->APB2ENR |= (1 <<3 );
	GPIOB->CRL &= 0x00ffffff;
	GPIOB->CRL |= 0x33000000;
	GPIOB->ODR |= 0x00C0;
	
}
/*
-----------------------------------------------
   ����: start����,SCCB����ʼ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void startSCCB(void)
{
	SIO_D_SET;     //�����߸ߵ�ƽ
    Delay_nUS(10);

    SIO_C_SET;	   //��ʱ���߸ߵ�ʱ���������ɸ�����
    Delay_nUS(10);
 
    SIO_D_CLR;
    Delay_nUS(10);

    SIO_C_CLR;	 //�����߻ָ��͵�ƽ��������������Ҫ
    Delay_nUS(10);


}
/*
-----------------------------------------------
   ����: stop����,SCCB��ֹͣ�ź�
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void stopSCCB(void)
{
	SIO_D_CLR;
    Delay_nUS(10);
 
    SIO_C_SET;
    Delay_nUS(10);
  

    SIO_D_SET;
    Delay_nUS(10);
 //   SCCB_DDR&= ~(SCCB_SIO_D);

}

/*
-----------------------------------------------
   ����: noAck,����������ȡ�е����һ����������
   ����: ��
 ����ֵ: ��
-----------------------------------------------
*/
void noAck(void)
{
	
	SIO_D_SET;
	Delay_nUS(10);
	
	SIO_C_SET;
	Delay_nUS(10);
	
	SIO_C_CLR;
	Delay_nUS(10);
	
	SIO_D_CLR;
	Delay_nUS(10);

}

/*
-----------------------------------------------
   ����: д��һ���ֽڵ����ݵ�SCCB
   ����: д������
 ����ֵ: ���ͳɹ�����1������ʧ�ܷ���0
-----------------------------------------------
*/
uchar SCCBwriteByte(uchar m_data)
{
	unsigned char j,tem;

	for(j=0;j<8;j++) //ѭ��8�η�������
	{
		if((m_data<<j)&0x80)
		{
			SIO_D_SET;
		}
		else
		{
			SIO_D_CLR;
		}
		Delay_nUS(10);
		SIO_C_SET;
		Delay_nUS(10);
		SIO_C_CLR;
		Delay_nUS(10);

	}
	Delay_nUS(10);
	
	SIO_D_IN;/*����SDAΪ����*/
	Delay_nUS(10);
	SIO_C_SET;
	//Delay_nMS(2);
	Delay_nUS(10);
	if(SIO_D_STATE)
	{
		tem=0;   //SDA=1����ʧ�ܣ�����0
	}
	else
	{
		tem=1;   //SDA=0���ͳɹ�������1
	}
	SIO_C_CLR;
	Delay_nUS(10);	
    SIO_D_OUT;/*����SDAΪ���*/

	return(tem);  
}

/*
-----------------------------------------------
   ����: һ���ֽ����ݶ�ȡ���ҷ���
   ����: ��
 ����ֵ: ��ȡ��������
-----------------------------------------------
*/
uchar SCCBreadByte(void)
{
	unsigned char read,j;
	read=0x00;
	
	SIO_D_IN;/*����SDAΪ����*/
	Delay_nUS(10);
	for(j=8;j>0;j--) //ѭ��8�ν�������
	{		     
		Delay_nUS(10);
		SIO_C_SET;
		Delay_nUS(10);
		read=read<<1;
		if(SIO_D_STATE) 
		{
			read=read+1;
		}
		SIO_C_CLR;
		Delay_nUS(10);
	}	
	return(read);
}
