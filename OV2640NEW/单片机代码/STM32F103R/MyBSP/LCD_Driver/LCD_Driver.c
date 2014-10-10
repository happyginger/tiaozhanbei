#include "LCD_Driver.h"
#include "MyBspInc.h"
#include "OnChip_Driver_Inc.h"
////////////////////////////////////////////////////////////////////////////////
//��������������Ĵ������ݺ�GRAM��������			
//���ú궨����棬˵����							
//  LCD_DATA_OUT()��һ��16λ�����������������, 	
//��д�Ĵ�����дGRAM(д���ص�)�ĺ�������,��ֲʱ 	
//�������ݽӿ���8λ��16λ���ĸú���(��ƺ궨��)		
////////////////////////////////////////////////////////////////////////////////
void LCD_DATA_OUT(u16 data)
{
#if LCD_8_BIT_EN		> 0
	//Ĭ��Ϊ8λ���ݶ˿�д��8λ			
	DATA_8BIT_OUT((u8)(data>>8));		
	LCD_WR_L;
	NOP();
	LCD_WR_H;
	
	//д��8λ							
	DATA_8BIT_OUT((u8)data);	
	LCD_WR_L;
	NOP();
	LCD_WR_H;
#else
	//16λ���ݽӿ�
	DATA_16BIT_OUT(data);
	LCD_WR_L;
	LCD_WR_H;
#endif
}
////////////////////////////////////////////////////////////////////////////////
//�������룬�����Ĵ������ݺ�GRAM��������		 	
//˵��:											 	
//  LCD_DATA_IN()��һ��16λ���ݴ����������϶�ȡ  	
//�����Ĵ����Ͷ�GRAM(�����ص�)�ĺ�������,��ֲʱ  	
//�������ݽӿ���8λ��16λ���ĸú���(������������)	
////////////////////////////////////////////////////////////////////////////////
u16 LCD_DATA_IN(void)
{
	u16 temp = 0;
#if LCD_8_BIT_EN		> 0
	GPIOC->CRL = 0x88888888;					//PC0-7  ��������
	LCD_RD_L;									//����Ч	
	NOP();										//8λ��ʱ��֮�����NOP()��ʱ
	NOP();										
	LCD_RD_H;
	temp = DATA_8BIT_IN;						//��ȡ���ֽ�				
	
	LCD_RD_L;									//����Ч	
	NOP();										//8λ��ʱ��֮�����NOP()��ʱ
	NOP();	
	LCD_RD_H;
	temp <<= 8;
	temp |= DATA_8BIT_IN;						//��ȡ���ֽ�					
	
	GPIOC->CRL = 0X33333333; //PC0-7  			//�������
#else
	//��Ϊ����
	GPIOC->CRL = 0x88888888; //PC0-7  ��������
	GPIOB->CRH = 0X88888888; // PB8-15��������
	GPIOB->ODR |= 0XFF00;    //ȫ�������
	GPIOC->ODR |= 0X00FF; 
	
	LCD_RD_L;									//����Ч						
	LCD_RD_H;
	temp = DATA_16BIT_IN;	
	
	GPIOC->CRL = 0X33333333; //PC0-7  �������
	GPIOB->CRH = 0X33333333; //PB8-15 �������
	GPIOB->ODR |= 0XFF00;    //ȫ�������
	GPIOC->ODR |= 0X00FF;
#endif
	return temp;
}
//дLCD��������,����Ҫ����0x0022��ֵ,һ��LCD����
//IC��ʹ�õĸ�����								
void LCD_WriteGRAM_EN(void)
{
	LCD_CS_L;									//ѡ��Ƭѡ						
	
	LCD_RS_L;									//�����д����RS->0��Ч			
	LCD_DATA_OUT(0x0022);						//д��Ĵ�����ַ				
	LCD_RS_H;
	//LCD_CS_H;//�����
}
//��LCD��������,����Ҫ����0x0022��ֵ,һ��LCD����
//IC��ʹ�õĸ�����								
void LCD_ReadGRAM_EN(void)
{
	LCD_CS_L;									//ѡ��Ƭѡ						
	
	LCD_RS_L;									//�����д����RS->0��Ч			
	LCD_DATA_OUT(0x0022);						//д��Ĵ�����ַ				
	LCD_RS_H;
}
////////////////////////////////////////////////////////////////////////////////
//  LCD_SetCursor ->���ö�дָ��				
//  �ú�������ֲʱ����Ҫ�޸�,һ������ICʹ����20H
//��21H�ֱ�����GARM��x��y��ַ					
////////////////////////////////////////////////////////////////////////////////
void LCD_SetCursor(u16 x,u16 y)
{	//													   	*-------->x			
  	LCD_WriteRegister(0x0020, x); // ��				   		|		(H 127)		
  	LCD_WriteRegister(0x0021, y); // ��				   		|					
	///////////////////////////////							|					
	//R61514��û��20H�Ĵ�����xy����ͨ��21H�Ĵ���ת��	   	|					
	//��ɣ���ʽ:y*256 + x,�μ��ֲ�R21H�Ĵ�������		   	V(V 159)			
# if R61514_EN 		> 0		
	LCD_WriteRegister(0x0021,(y << 8) + x);
# endif
}

////////////////////////////////////////////////////////////////////////////////
//  LCD_WriteRegister()->д�Ĵ���												
//  д�Ĵ���,LCD i80�ӿڱ�׼,RSΪ��ʱд�Ĵ���IR	
//(�Ĵ���ָ��),��index��ֵ������;RSΪ��ʱд����,
//��data��ֵ����IRָ��ļĴ���,�ú�����i80�ӿ���
//��Ӧ��,��ֲʱ�����޸�							
////////////////////////////////////////////////////////////////////////////////
void LCD_WriteRegister(u16 index, u16 data)
{
////////////////////////////////////////////////////////////////////////////////
  	// ����8λģʽ,�����ݶ˿�Ҫ�������β�������ͼ��16λģʽ����µĲ���   //
  	// nCS       ----\__________________________________________/-------  //
  	// RS        ------\____________/-----------------------------------  //
  	// nRD       -------------------------------------------------------  //
  	// nWR       --------\_______/--------\_____/-----------------------  //
  	// DB[0:15]  ---------[index]----------[data]-----------------------  //
  	//                                                                    //
////////////////////////////////////////////////////////////////////////////////
	LCD_CS_L;									//ѡ��Ƭѡ						
	
	LCD_RS_L;									//�����д����RS->0��Ч			
	LCD_DATA_OUT(index);						//��д��Ĵ�����ַ				
	
	LCD_RS_H;									//���ݶ�д����RS->1��Ч			
	LCD_DATA_OUT(data);
	
	LCD_CS_H;									//�ر�Ƭѡ						
}

////////////////////////////////////////////////////////////////////////////////
//  LCD_ReadRegister()->���Ĵ���												
//  ���Ĵ���,LCD i80�ӿڱ�׼,RSΪ��ʱд�Ĵ���IR	
//(�Ĵ���ָ��),��index��ֵ������;RSΪ��ʱ������,
//��data��ֵ����IRָ��ļĴ���,�ú�����i80�ӿ���
//��Ӧ��,��ֲʱ�����޸�							
////////////////////////////////////////////////////////////////////////////////
u16 LCD_ReadRegister(u16 index)
{
////////////////////////////////////////////////////////////////////////////////
  	// ����8λģʽ,�����ݶ˿�Ҫ�������β�������ͼ��16λģʽ����µĲ���   //
  	// nCS       ----\__________________________________________/-------  //
  	// RS        ------\____________/-----------------------------------  //
  	// nRD       -------------------------\_____/-----------------------  //
  	// nWR       --------\_______/--------------------------------------  //
  	// DB[0:15]  ---------[index]----------[data]-----------------------  //
  	//                                                                    //
////////////////////////////////////////////////////////////////////////////////
	u16 temp = 0;
  	LCD_CS_L;									//ѡ��Ƭѡ						
	
	LCD_RS_L;									//�����д����RS->0��Ч			
	LCD_DATA_OUT(index);						//д��Ĵ�����ַ				
	
	LCD_RS_H;									//���ݶ�д����RS->1��Ч			
	temp = LCD_DATA_IN();
	LCD_CS_H;									//�ر�Ƭѡ						
	return temp;
}

////////////////////////////////////////////////////////////////////////////////
//  LCD_DrawPoint()->���㺯��					
//  �ú���������GARM��ַ,Ȼ����д����,�����ٽ�
//color��ֵ����GRAM�ж�Ӧ�ĵ�ַ.�ú������ϲ�Ӧ��
//��ֲʱ�����޸�								
////////////////////////////////////////////////////////////////////////////////
void LCD_DrawPoint(u16 x, u16 y, u16 color)
{
	if ((x > X_END) || (y > Y_END)) 			//��ʾ���ܳ����������ֵ		
	{
		return;
	}
	LCD_SetCursor(x, y);
	
	LCD_WriteGRAM_EN();
	LCD_DATA_OUT(color);
	LCD_WR_END();
}
////////////////////////////////////////////////////////////////////////////////
//  LCD_GetPoint()->���㺯��					
//  �ú���������GARM��ַ,Ȼ����д����,������һ
//���Ʋ���,���Ʋ�������8λ��16λ�ӿ���Ҫ���ĺ궨
//��.�ú������ϲ�Ӧ����ֲʱ�����޸�				
////////////////////////////////////////////////////////////////////////////////
u16 LCD_GetPoint(u16 x, u16 y)
{
////////////////////////////////////////////////////////////////////////////////
  	// ����8λģʽ,�����ݶ˿�Ҫ�������β�������ͼ��16λģʽ����µĲ���   //
  	// nCS       ----\__________________________________________/-------  //
  	// RS        ------\____________/-----------------------------------  //
  	// nRD       -------------------------\_____/------\_____/----------  //
  	// nWR       --------\_______/--------------------------------------  //
  	// DB[0:15]  ---------[Address]-------[Dummy]-------[data]----------  //
  	// ע��:�ڶ�GRAMʱ,Ҫ�����ö���ַ����д�����Ȼ����Dummy Read����� //
	//      �������ݡ�ͬʱ����8λģʽ�£�Dummy Read ��һ��Word��ʱ�䣬��ζ//
	//		��RDҪ����2�������أ��������������ݻ�Ҫ����ת��			  //
////////////////////////////////////////////////////////////////////////////////
	u16 temp_color = 0;
  	u16 temp = 0;
  	
	LCD_SetCursor(x,y);
  	
	LCD_ReadGRAM_EN();
	
	DUMMY_READ();						//��һ�β���							
	temp_color = LCD_DATA_IN();
	
	LCD_WR_END();
										
	temp = temp_color >> 11;			//RRRRRGGGGGGBBBBB ��Ϊ 				
	temp |= temp_color << 11;			//BBBBBGGGGGGRRRRR ��ʽ					
	temp |= (temp_color & 0x07E0);		//0000011111100000  = 0x07E0			
  	
	return temp;
}
////////////////////////////////////////////////////////////////////////////////
//  LCD_SetWindow()->����GRAM����				
//  �ú������ڿ�д������,�û�ֻ�����úÿ�д���� 
//Ȼ�����θ�����GRAM����д���ݼ���,GCָ����Զ� 
//���Ӷ������û�����,�����������дЧ��.������ʱ
//GC�����Զ�����(GC->GRAM Counter)				
////////////////////////////////////////////////////////////////////////////////
void LCD_SetWindow(u16 start_x,u16 start_y,
					u16 end_x,u16 end_y)
{ 
  		
  	LCD_WriteRegister(0x0050, start_x);	
  	LCD_WriteRegister(0x0052, start_y);	
  	LCD_WriteRegister(0x0051, end_x);		
  	LCD_WriteRegister(0x0053, end_y);	 
	 	
	//����ICR61514->����GARM��ַ			
  	////////////////////////////////////////
# if R61514_EN		> 0
	end_x <<= 8;					//��8λ�Ǵ�ֱ������ַ						
  	end_x |= (unsigned char)start_x;//��8λ�Ǵ�ֱ��ʼ��ַ(ˮƽGRAM��ַҲ����� )
  	end_y <<= 8;
  	end_y |= (unsigned char)start_y;
	
  	LCD_WriteRegister(0x0044, end_x);
  	LCD_WriteRegister(0x0045, end_y);
# endif
	LCD_SetCursor(start_x, start_y);//������ʼ��ͼ��GRAM��ַ					
	
}
//LCD��������							
void LCD_Clear(u16 color)
{
	unsigned long  i;
	LCD_SetWindow(0, 0, X_END, Y_END);
	LCD_WriteGRAM_EN();
	for(i=0; i< X_SIZE*Y_SIZE; i++)		//R61514Ϊ128*160						
  	{
    	LCD_CS_L;
		LCD_DATA_OUT(color);
		LCD_CS_H;
  	}
	LCD_WR_END();
	
	LCD_SetWindow(0,0, X_END, Y_END);	//�ָ�ȫ����ͼ��						
}
//���һ����������						
void LCD_Fill(u16 start_x,u16 start_y,
			  u16 end_x,u16 end_y,
			  u16 color)
{
	unsigned long  i;

	LCD_SetWindow(start_x, start_y, end_x, end_y);
	
	LCD_WriteGRAM_EN();
	for(i=0; i < (end_x - start_x + 1)*(end_y - start_y + 1); i++)				
  	{
    	LCD_DATA_OUT(color);			//�����ɫ								
  	}
	LCD_WR_END();
	
	LCD_SetWindow(0,0, X_END, Y_END);	//�ָ�ȫ����ͼ��						
}
//Һ���˿ڳ�ʼ��,�ⲿ�������			
void LCD_PortInit(void)
{
	//���ƶ˿ڷ����ʼ��,��ֲʱֻ������йغ궨��
	/*LCD_CS_DDR |= BIT(LCD_CS_BIT);
	LCD_RS_DDR |= BIT(LCD_RS_BIT);
	LCD_WR_DDR |= BIT(LCD_WR_BIT);
	LCD_RD_DDR |= BIT(LCD_RD_BIT);
	LCD_REST_DDR |= BIT(LCD_REST_BIT);*/
	//���ƶ˿ڵ�ƽ��ʼ��						
	/*LCD_CS_H;
	LCD_RS_H;
	LCD_WR_H;
	LCD_RD_H;
	LCD_REST_H;*/
	//�������߶˿ڷ����ʼ��,�ӽӿ���Ҫ���Ļ����
	
	//DATA_L_DDR = 0xff;
	//DATA_L_PORT = 0xff;
	
	RCC->APB2ENR|=1<<3;//��ʹ������PORTBʱ��
 	RCC->APB2ENR|=1<<4;//��ʹ������PORTCʱ��

											 
	//PORTC0~13������� 	
	GPIOC->CRL=0x33333333;
	GPIOC->CRH&=0xFF000000;
	GPIOC->CRH|=0x00333333;
	GPIOC->ODR|=0x3FFF;	
	//PORTB ��8λ������� 
#if LCD_8_BIT_EN		> 0
	
#else
	GPIOB->CRH=0x33333333;
	GPIOB->ODR|=0xFF00;
#endif

}
//Һ��������ʼ��						
void LCD_DriverInit(void)
{
	unsigned short int lcd_id = 0x0000;		//����LCD_ID��							
	LCD_PortInit();						//���ö˿ڳ�ʼ��						
				
    SET_PORT_H(LCD_LED_PORT, LCD_LED_BIT);//������
	LCD_REST_L;							//��λ����								
	Delay_nMS(1);	
    LCD_REST_H;
	Delay_nMS(1);						//��ɸ�λ											
	LCD_WriteRegister(0x0000, 0x0001);	//����ʱ��								
	Delay_nMS(10);
	lcd_id = LCD_ReadRegister(0x0000);	//��ȡLCD_ID							
	lcd_id = 0x9325;
	while(0)
	{
		UART_Transmit(lcd_id >> 8);
		UART_Transmit(lcd_id);
		Delay_nMS(500);
	}
	switch(lcd_id)
	{
		//R61514
		case 0x1514:					//R61514S����IC							
		LCD_WriteRegister(0x0007, 0x0000);
		Delay_nMS(2);
		LCD_WriteRegister(0x0017, 0x0001);
		LCD_WriteRegister(0x0010, 0x4410);
		LCD_WriteRegister(0x0011, 0x0000);
		LCD_WriteRegister(0x0012, 0x0118);
		LCD_WriteRegister(0x0013, 0x0d23);
		Delay_nMS(2);
		LCD_WriteRegister(0x0010, 0x4490);
		LCD_WriteRegister(0x0011, 0x0000);
		LCD_WriteRegister(0x0012, 0x013d);
		Delay_nMS(2);
		LCD_WriteRegister(0x0001, 0x0213);
		LCD_WriteRegister(0x0003, 0x1030);	//0x1030,���ýӿڷ�ʽ,��ɫλ����ɨ�跽��
		LCD_WriteRegister(0x0007, 0x0005);
		LCD_WriteRegister(0x0008, 0x0603);
		LCD_WriteRegister(0x0009, 0x002f);
		LCD_WriteRegister(0x000b, 0x8000);
	
		LCD_WriteRegister(0x0040, 0x0000);
		LCD_WriteRegister(0x0041, 0x0000);
		LCD_WriteRegister(0x0042, 0x9f00);
		LCD_WriteRegister(0x0043, 0x9f00);
		LCD_WriteRegister(0x0044, 0x7f00);	//ˮƽ������ʼ��С(��8λHEA[������ַ],��8λHSA[��ʼ��ַ])
		LCD_WriteRegister(0x0045, 0x9f00);	//��ֱ������ʼ��С(��8λVEA[������ַ],��8λVSA[��ʼ��ַ])
		LCD_WriteRegister(0x0021, 0x0000);	//GRAM��ʼ�����										 
		
		LCD_WriteRegister(0x0030, 0x0103);
		LCD_WriteRegister(0x0031, 0x0001);
		LCD_WriteRegister(0x0032, 0x0304);
		LCD_WriteRegister(0x0033, 0x0204);
		LCD_WriteRegister(0x0034, 0x0406);
		LCD_WriteRegister(0x0035, 0x0707);
		LCD_WriteRegister(0x0036, 0x0407);
		LCD_WriteRegister(0x0037, 0x0303);
		LCD_WriteRegister(0x0038, 0x0403);
		LCD_WriteRegister(0x0039, 0x0102);
		Delay_nMS(1);
											//-----Display On-------------------
		LCD_WriteRegister(0x0002, 0x0600);
		LCD_WriteRegister(0x0007, 0x0015);
		Delay_nMS(1);
		LCD_WriteRegister(0x0007, 0x0055);
		Delay_nMS(1);
		LCD_WriteRegister(0x0002, 0x0700);
		LCD_WriteRegister(0x0007, 0x0077);
		
		break;								//��ɳ�ʼ��						
		//ILI9320��ILI9328
		case 0x9325:
		case 0x9328:
		case 0xD325:
	//************* Start Initial Sequence **********// 
        LCD_WriteRegister(0x0001, 0x0100);        // set SS and SM bit 
        LCD_WriteRegister(0x0002, 0x0700);        // set 1 line inversion 
        #if LCD_CHAR_HORIZONTAL > 0
		//�ַ�����(����),��X->240,CHAR_X->8 
        LCD_WriteRegister(0x0003,0x1010);//65K    ,8λģʽ�����δ���,16λģʽ��1�δ���,BIT[7]OGRΪ0ʱϵͳ����̶�,(H->x:0-219,V->y:0-319)
		#else
		//�ַ�˳ʱ����ת90��(����),��X->240,CHAR_X->16
		LCD_WriteRegister(0x0003,0x1038);	
		#endif 
        LCD_WriteRegister(0x0004, 0x0000);        // Resize register 
        LCD_WriteRegister(0x0008, 0x0207);        // set the back porch and front porch 
        LCD_WriteRegister(0x0009, 0x0000);        // set non-display area refresh cycle ISC[3:0] 
        LCD_WriteRegister(0x000A, 0x0000);        // FMARK function 
        LCD_WriteRegister(0x000C, 0x0000);        // RGB interface setting 
        LCD_WriteRegister(0x000D, 0x0000);        // Frame marker Position 
        LCD_WriteRegister(0x000F, 0x0000);          // RGB interface polarity 
    //*************Power On sequence ****************// 
        LCD_WriteRegister(0x0010, 0x0000);            // SAP, BT[3:0], AP, DSTB, SLP, STB 
        LCD_WriteRegister(0x0011, 0x0007);            // DC1[2:0], DC0[2:0], VC[2:0] 
        LCD_WriteRegister(0x0012, 0x0000);            // VREG1OUT voltage 
        LCD_WriteRegister(0x0013, 0x0000);            // VDV[4:0] for VCOM amplitude 
    	LCD_WriteRegister(0x0007, 0x0001);       
            Delay_nMS(200);                                                        // Dis-charge capacitor power voltage 
        LCD_WriteRegister(0x0010, 0x1290);            // SAP, BT[3:0], AP, DSTB, SLP, STB 
        LCD_WriteRegister(0x0011, 0x0227);            // DC1[2:0], DC0[2:0], VC[2:0] 
            Delay_nMS(50);                                             // Delay 50ms     
        LCD_WriteRegister(0x0012, 0x001A);            // Internal reference voltage= Vci; 
            Delay_nMS(50);                                             // Delay 50ms 
        LCD_WriteRegister(0x0013, 0x1800);            // Set VDV[4:0] for VCOM amplitude 
        LCD_WriteRegister(0x0029, 0x0028);            // Set VCM[5:0] for VCOMH 
        LCD_WriteRegister(0x002B, 0x000C);            // Set Frame Rate   
            Delay_nMS(50);                                                            // Delay 50ms 
        LCD_WriteRegister(0x0020, 0x0000);            // GRAM horizontal Address 
        LCD_WriteRegister(0x0021, 0x0000);            // GRAM Vertical Address   
    // ----------- Adjust the Gamma    Curve ----------// 
        LCD_WriteRegister(0x0030, 0x0000); 
        LCD_WriteRegister(0x0031, 0x0305); 
        LCD_WriteRegister(0x0032, 0x0003); 
        LCD_WriteRegister(0x0035, 0x0304); 
        LCD_WriteRegister(0x0036, 0x000F); 
        LCD_WriteRegister(0x0037, 0x0407); 
        LCD_WriteRegister(0x0038, 0x0204); 
        LCD_WriteRegister(0x0039, 0x0707); 
        LCD_WriteRegister(0x003C, 0x0403); 
        LCD_WriteRegister(0x003D, 0x1604); 
    	//------------------ Set GRAM area ---------------// 
        LCD_WriteRegister(0x0050, 0x0000);          // Horizontal GRAM Start Address 
        LCD_WriteRegister(0x0051, 0x00EF);          // Horizontal GRAM End Address 
        LCD_WriteRegister(0x0052, 0x0000);          // Vertical GRAM Start Address 
        LCD_WriteRegister(0x0053, 0x013F);          // Vertical GRAM Start Address 
		LCD_WriteRegister(0x0060, 0xA700);          // Gate Scan Line 
        LCD_WriteRegister(0x0061, 0x0001);          // NDL,VLE, REV 
        LCD_WriteRegister(0x006A, 0x0000);          // set scrolling line 
    	//-------------- Partial Display Control ---------// 
        LCD_WriteRegister(0x0080, 0x0000); 
        LCD_WriteRegister(0x0081, 0x0000); 
        LCD_WriteRegister(0x0082, 0x0000); 
        LCD_WriteRegister(0x0083, 0x0000); 
        LCD_WriteRegister(0x0084, 0x0000); 
        LCD_WriteRegister(0x0085, 0x0000); 
    	//-------------- Panel Control -------------------// 
        LCD_WriteRegister(0x0090, 0x0010); 
        LCD_WriteRegister(0x0092, 0x0600); 
       
        LCD_WriteRegister(0x0007, 0x0133);          // 262K color and display ON 
		break;
		//ILI9320��ILI9300
		
		case 0x9320:
		case 0x9300:
		LCD_WriteRegister(0x00,0x0000);
		LCD_WriteRegister(0x01,0x0100);	//Driver Output Contral.
		LCD_WriteRegister(0x02,0x0700);	//LCD Driver Waveform Contral.
	#if LCD_CHAR_HORIZONTAL > 0
		//�ַ�����(����),��X->240,CHAR_X->8 
        LCD_WriteRegister(0x0003,0x1010);//65K    ,8λģʽ�����δ���,16λģʽ��1�δ���,BIT[7]OGRΪ0ʱϵͳ����̶�,(H->x:0-219,V->y:0-319)
	#else
		//�ַ�˳ʱ����ת90��(����),��X->240,CHAR_X->16
		LCD_WriteRegister(0x0003,0x1038);	
	#endif
		LCD_WriteRegister(0x04,0x0000);	//Scalling Contral.
		LCD_WriteRegister(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WriteRegister(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WriteRegister(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WriteRegister(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WriteRegister(0x0d,0x0000);	//Frame Maker Position.
		LCD_WriteRegister(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		Delay_nMS(5); 
		LCD_WriteRegister(0x07,0x0101);	//Display Contral.
		Delay_nMS(5); 								  
		LCD_WriteRegister(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WriteRegister(0x11,0x0007);								//Power Control 2.(0x0001)
		LCD_WriteRegister(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
		LCD_WriteRegister(0x13,0x0b00);								//Power Control 4.
		LCD_WriteRegister(0x29,0x0000);								//Power Control 7.
	
		LCD_WriteRegister(0x2b,(1<<14)|(1<<4));	    
		LCD_WriteRegister(0x50,0);	//Set X Star
		//ˮƽGRAM��ֹλ��Set X End.
		LCD_WriteRegister(0x51,239);	//Set Y Star
		LCD_WriteRegister(0x52,0);	//Set Y End.t.
		LCD_WriteRegister(0x53,319);	//
	
		LCD_WriteRegister(0x60,0x2700);	//Driver Output Control.
		LCD_WriteRegister(0x61,0x0001);	//Driver Output Control.
		LCD_WriteRegister(0x6a,0x0000);	//Vertical Srcoll Control.
	
		LCD_WriteRegister(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WriteRegister(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WriteRegister(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WriteRegister(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WriteRegister(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WriteRegister(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		LCD_WriteRegister(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WriteRegister(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WriteRegister(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WriteRegister(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WriteRegister(0x97,(0<<8));	//
		LCD_WriteRegister(0x98,0x0000);	//Frame Cycle Contral.	   
		LCD_WriteRegister(0x07,0x0173);	//(0x0173)
		break;
	}
	
}
