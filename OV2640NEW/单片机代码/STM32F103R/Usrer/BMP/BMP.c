#include "BMP.h"
#include "MyBspInc.h"
#include "string.h"

static u16 MenuBuf[1][1];	//4���ַ�ռ��������2�����ؿ�ȵı߿�(64*4 +(16+4)*4)
u16* P_ImageBuf = (u16*)(&(MenuBuf[0][0]));

//# include "ILI9320.h"
//# include "USART.h"
//# include "integer.h"
//# include "IND_LED.h"
//				R		G		B			
//RGB555����Ϊ0x7C00��0x03E0��0x001F		
//RGB565����Ϊ0xF800��0x07E0��0x001F
#define LINE_SIZE_CNT		12			//���ֵ����Ϊż����С�ڵ���16
#define LINE_PIXEL_CNT		320
#define IMAGE_BUFFER_SIZE	(LINE_SIZE_CNT*LINE_PIXEL_CNT*2)
extern u16* P_ImageBuf;
#define RGB555_R_MASK	0x7C00
#define RGB555_G_MASK	0x03E0
#define RGB555_B_MASK	0x001F

#define RGB565_R_MASK	0xF800
#define RGB565_G_MASK	0x07E0
#define RGB565_B_MASK	0x001F

#define RGB555toRGB565_(rgb555)	((((rgb555) & 0x7C00) << 1) | (((rgb555) & 0x03E0) << 1) | ((rgb555) & 0x001F))
#define RGB24toRGB565_(b, g, r)	((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define RGB32toRGB565_(b, g, r)	((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))



typedef struct RGB_MASK{
	DWORD rMask;  											
	DWORD gMask;  					
	DWORD bMask;  																			
}RGB_MASK;

const RGB_MASK rgb555_mask={
							RGB555_R_MASK,
							RGB555_G_MASK,
							RGB555_B_MASK
};
const RGB_MASK rgb565_mask={
							RGB565_R_MASK,
							RGB565_G_MASK,
							RGB565_B_MASK
};
void Get_BmpFileHeader(BITMAPFILEHEADER* pf_header, void* pdata)
{
	u8* pbuf = (u8*)pdata;
	
	pf_header->bfType = *(u16*)pbuf;
	pbuf += 2;
	
}

//ʮ��λ��bmp��ȡ����ʾ����			
//16λ���ֶ��������ÿ�ж�����һ��	
//��㣬��ɨ��ʱҪ������			
//�����buf_sizeΪ2��������			
void BMP_Read16Clr1(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size)
{
	FRESULT res;
	u32 br = 0;
	
	u16 *p_rgb555 = 0;
	u16 temp_rgb565 = 0;
	
	u32 i = 0;		
	u16 real_pix_width = 			//�ֶ�����ʵ�ʿ��(16λΪһ������)
					(((p_iheader->biWidth * p_iheader->biBitCount)+31)>>5)<<1;
							
	unsigned int pix_cnt = 0;				//���������¼ɨ��������ص�		
	buf_size -= (buf_size%2);				//��2���ֽ�Ϊ��������ȡ
	if(p_iheader->biCompression == BI_RGB)
	{										//RGB555							
		if(p_iheader->biWidth < real_pix_width)
		{									//�����ֶ��봦��					
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}							//ǿ��ת��Ϊ16λ��rgb����			
				p_rgb555 = (u16 *)p_buf;
											//��ʾһ��buf������					
				for(i = 0; i < (br >> 1); i ++)
				{
					pix_cnt ++;
					if(pix_cnt == real_pix_width)
					{
						pix_cnt = 0;
						continue;	
					}
					temp_rgb565 = RGB555toRGB565_(*(p_rgb555 + i));
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
		else								//�������ֶ��봦��					
		{
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				p_rgb555 = (u16 *)p_buf;
				for(i = 0; i < (br  >> 1); i ++)
				{
					temp_rgb565 = RGB555toRGB565_(*(p_rgb555 + i));
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
	}										//���δ�����첻��	RGB565			
	else if(p_iheader->biCompression == BI_BITFIELDS)
	{										
		if(p_iheader->biWidth < real_pix_width)
		{									//�����ֶ��봦��					
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				temp_rgb565 = *(u16 *)p_buf;
			
				for(i = 0; i < (br >> 1); i ++)
				{
					pix_cnt ++;
					if(pix_cnt == real_pix_width)
					{
						pix_cnt = 0;
						continue;	
					}				
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
		else								//�������ֶ��봦��					
		{
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				p_rgb555 = (u16 *)p_buf;
				for(i = 0; i < (br  >> 1); i ++)
				{
					LCD_DATA_OUT(*(p_rgb555 + i));
				}
			}
		}
	}
}
//ʮ��λ��bmp��ȡ����ʾ����			
//16λ���ֶ��������ÿ�ж�����һ��	
//��㣬��ɨ��ʱҪ������			
//�����buf_sizeΪ2��������			
void BMP_Read16Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size)
{
	FRESULT res;
	u32 br = 0;
	
	u16 *p_rgb555 = 0;
	u16 temp_rgb565 = 0;
	u16 cnt = 0;
	u16 line_cnt = 0;
	static u32 LineCnt = 0;
	u32 i = 0;		
	u32 j = 0;
	u16 real_pix_width = 			//�ֶ�����ʵ�ʿ��(16λΪһ������)
					(((p_iheader->biWidth * p_iheader->biBitCount)+31)>>5)<<1;
							
	unsigned int pix_cnt = 0;				//���������¼ɨ��������ص�		
	buf_size -= (buf_size%2);				//��2���ֽ�Ϊ��������ȡ
	if(p_iheader->biCompression == BI_RGB)
	{										//RGB555							
		if(p_iheader->biWidth < real_pix_width)
		{									//�����ֶ��봦��					
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}							//ǿ��ת��Ϊ16λ��rgb����			
				p_rgb555 = (u16 *)p_buf;
											//��ʾһ��buf������					
				for(i = 0; i < (br >> 1); i ++)
				{
					pix_cnt ++;
					if(pix_cnt == real_pix_width)
					{
						pix_cnt = 0;
						continue;	
					}
					temp_rgb565 = RGB555toRGB565_(*(p_rgb555 + i));
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
		else								//�������ֶ��봦��					
		{
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				p_rgb555 = (u16 *)p_buf;
				for(i = 0; i < (br  >> 1); i ++)
				{
					temp_rgb565 = RGB555toRGB565_(*(p_rgb555 + i));
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
	}										//���δ�����첻��	RGB565			
	else if(p_iheader->biCompression == BI_BITFIELDS)
	{										
		if(p_iheader->biWidth < real_pix_width)
		{									//�����ֶ��봦��					
			while(1)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				temp_rgb565 = *(u16 *)p_buf;
			
				for(i = 0; i < (br >> 1); i ++)
				{
					pix_cnt ++;
					if(pix_cnt == real_pix_width)
					{
						pix_cnt = 0;
						continue;	
					}				
					LCD_DATA_OUT(temp_rgb565);
				}
			}
		}
		else								//�������ֶ��봦��					
		{
			for(j = 0; j < 240; j +=LINE_SIZE_CNT)
			{								//��ȡSD��������					
				res = f_read(p_fbmp, p_buf, buf_size, &br);
				if(res || (br == 0))
				{
					return;
				}
				for(cnt = 0; cnt < LINE_SIZE_CNT; cnt +=2)
				{
					p_rgb555 = (u16 *)p_buf + 320*cnt;
					for(i = 0; i < 320 - 8; i += 2,p_rgb555 += 2)
					{
						LCD_DATA_OUT(*p_rgb555 );
					}
					line_cnt ++;
					if((line_cnt > (240>>1) - 5))
					{	//��ͼ���
						return;
					}
				}
			}
		}
	}
}
//��ʮ��λ��bmp��ȡ����ʾ����			
//24λ���ֶ��������ÿ������1,2,��3����	
//��,������ɨ��ʱ��ͬ��16λ��ɨ�裬Ҫ��	
//�������ֽڣ����������ص�				
//�����buf_sizeΪ3��������				
void BMP_Read24Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size)
{
	FRESULT res;
	u32 br = 0;
	
	u8 r,g,b;
	u16 temp_rgb565 = 0;
	
	u32 i = 0;		
	u16 real_char_width = 					//�ֶ�����ʵ���ֽ���,������16λbmp
					(((p_iheader->biWidth * p_iheader->biBitCount)+31)>>5)<<2;
											
	u16 char_cnt = 0;						//���������¼ɨ������ֽ�����������16λ
	u16 temp = (p_iheader->biWidth)*3;	
	
																
	if(temp < real_char_width)				//��real_char_width���ֽ�Ϊ��������ȡ
	{	buf_size = buf_size -(buf_size%real_char_width);								
		//���һ�е��ֽڿ�ȴ����ֶ���		
		while(1)
		{
			res = f_read(p_fbmp, p_buf, buf_size, &br);
			if(res || (br == 0))
			{
				return;
			}
			for(i = 0; i < br; )
			{
											//�����ֶ������ӵ��ֽ�����		
				if((char_cnt >= temp) && 
				   (char_cnt <= real_char_width))
				{
					char_cnt ++;
					i++;					//i������ż���ʵ�ִ�������		
					if(char_cnt == real_char_width)
					{
						char_cnt = 0;
					}
					continue;	
				}
				
			
				char_cnt += 3;				//������ʾʱһ����ʾ�����ֽ�		
				
				b = *(p_buf + i);			//��ȡһ�����ص�rgb					
				i++;
				g = *(p_buf + i);
				i ++;
				r = *(p_buf + i);
				i ++;
				
				temp_rgb565= RGB24toRGB565_(b, g, r);
				/*DATA_L = temp_rgb565;
				DATA_H = temp_rgb565 >> 8;
				WR_L;
				WR_H;*/
				LCD_DATA_OUT(temp_rgb565);
			}
		}
	}
	else									//��3���ֽ�Ϊ��������ȡ
	{	buf_size = buf_size -(buf_size%3);
		while(1)
		{
			res = f_read(p_fbmp, p_buf, buf_size, &br);
			if(res || (br == 0))
			{
				return;
			}
			for(i = 0; i < br; )
			{
				b = *(p_buf + i);
				i++;
				g = *(p_buf + i);
				i ++;
				r = *(p_buf + i);
				i ++;
				
				temp_rgb565= RGB24toRGB565_(b, g, r);
				/*DATA_L = temp_rgb565;
				DATA_H = temp_rgb565 >> 8;
				WR_L;
				WR_H;*/
				LCD_DATA_OUT(temp_rgb565);
				
			}
		}
	}
}

//��ʮ��λ��bmp��ȡ����ʾ����			
//32Ϊ��bmp�������ֶ�����ÿ����������	
//�ڣ���Ϊ32λ��Ȼ���ֶ���				
//�����buf_sizeΪ4��������				
void BMP_Read32Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size)
{
	FRESULT res;
	u32 br = 0;
	
	u8 r,g,b;
	u16 temp_rgb565 = 0;
	
	u32 i = 0;		
	buf_size -= (buf_size%4);					//��4���ֽ�Ϊ��������ȡ
	p_iheader->biClrUsed = p_iheader ->biClrUsed;
	while(1)
	{
		res = f_read(p_fbmp, p_buf, buf_size, &br);
		if(res || (br == 0))
		{
			return;
		}
		for(i = 0; i < br; )
		{
			b = *(p_buf + i);
			i++;
			g = *(p_buf + i);
			i ++;
			r = *(p_buf + i);
			i ++;
			i ++;
			
			temp_rgb565= RGB32toRGB565_(b, g, r);
			/*DATA_L = temp_rgb565;
			DATA_H = temp_rgb565 >> 8;
			WR_L;
			WR_H;*/
			LCD_DATA_OUT(temp_rgb565);
		}
	}
}


u32 BMP_MakeBmpFile(long bmp_width, 
					long bmp_height,
					FIL* p_bmp,
					const char *p_path,
					u8 bmp_type)
{
	BITMAPFILEHEADER f_header;
	BITMAPINFOHEADER info_header;
	
	RGB_MASK rgb_mask;
	FRESULT res;
	u32 bw = 0;
										//��ʼ��bmp���ļ�ͷ����Ϣͷ							
	BMP_bmpInit(bmp_width, bmp_height, 16, &f_header, &info_header, bmp_type);
	res = f_open(p_bmp, p_path, FA_CREATE_ALWAYS | FA_WRITE);
	
	if(res)
	{
		res = f_close(p_bmp);
		return 0;
	}
	res = f_lseek(p_bmp, f_header.bfSize);	//�½���bmp�ļ���С
	res = f_lseek(p_bmp, 0);				//��λ���ļ���ͷ
	//д���ļ�ͷ
	res = f_write(p_bmp, &f_header, sizeof(BITMAPFILEHEADER), &bw);
	//д����Ϣͷ
	res = f_write(p_bmp, &info_header, sizeof(BITMAPINFOHEADER), &bw);
	
	//дRGB MASK
	if(bmp_type == RGB565)
	{
		res = f_write(p_bmp, &rgb565_mask, sizeof(RGB_MASK), &bw);	
	}
	else if(bmp_type == RGB555)
	{
		res = f_write(p_bmp, &rgb555_mask, sizeof(RGB_MASK), &bw);	
	}
	
	res = f_sync(p_bmp);
	return info_header.biSizeImage;
}

u8 BMP_FillBmp(FIL* p_bmp,
				 u32 fill_size,
				 void* p_buf,
				 u32 buf_size)
{	
	static u32 Cnt = 0;
	FRESULT res;
	u32 bw = 0;
	fill_size -= Cnt;
	if(fill_size > buf_size)
	{
		res = f_write(p_bmp, p_buf, buf_size, &bw);
		Cnt += buf_size;
		return 0;
	}
	else
	{
		res = f_write(p_bmp, p_buf, fill_size, &bw);
		res = f_close(p_bmp);
		Cnt = 0;
		return 1;
	}
}


void BMP_bmpInit(long bmp_width,
				 long bmp_height,
				 unsigned int bmp_bitcount,
				 BITMAPFILEHEADER *p_fheader, BITMAPINFOHEADER *p_info_header,
				 u8 bmp_type)
{
	long bmp_width_temp = 				//4�ֽڶ�����ʵ�ʿ��
				(((bmp_width * bmp_bitcount)+31)>>5)<<1;
	p_fheader->bfType = 0x4D42;			//���ļ�ͷ,��Ϣͷ��ͼ������(û�е�ɫ��)		
	p_fheader->bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmp_width_temp*bmp_height*2;
	if((bmp_type == RGB555) || (bmp_type== RGB565))
	{
		p_fheader->bfSize += sizeof(RGB_MASK);
	}
	p_fheader->bfReserved1 = 0;			//����λΪ0								
	p_fheader->bfReserved2 = 0;			//��û�в�ɫ��������λͼ���ݴ��ڴ�	
	p_fheader->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	if((bmp_type == RGB555) || (bmp_type== RGB565))
	{
		p_fheader->bfOffBits += sizeof(RGB_MASK);
	}
										//ͨ���������Ϣͷ��СΪ��ֵ			
	p_info_header->biSize = sizeof(BITMAPINFOHEADER);
	p_info_header->biWidth = bmp_width;	//��ʼ��Ϊ0								
	p_info_header->biHeight = -bmp_height;	//��ʼ��Ϊ0							
	p_info_header->biPlanes = 1;		//��ʼ��Ϊ16λλͼ						
	p_info_header->biBitCount = bmp_bitcount;
	if((bmp_type == RGB555) || (bmp_type== RGB565))
	{									//16λģʽ
		p_info_header->biCompression = BI_BITFIELDS;
	}
	else
	{
		p_info_header->biCompression = BI_RGB;	
	}				
										//λͼ�����¹�ʽ����					
	p_info_header->biSizeImage = bmp_width_temp * bmp_height*2;
	p_info_header->biXPelsPerMeter = 0;	//�ֱ��ʳ�ʼΪ0							
	p_info_header->biYPelsPerMeter = 0;
	p_info_header->biClrUsed = 0;		//ʹ�õ�����ɫ��Ϊ0��ʾ��ɫ��Ϊ2��biBitCount�η�����16λ��Ϊ65Kɫ
	p_info_header->biClrImportant = 0;	//������ɫ����Ҫ						
}

////////////////////
////////////////////
signed char BMP_Showbmp1(unsigned int start_x,
			  			unsigned int start_y,
			  			const char *p_path)
{
	FIL f_bmp;
	FRESULT res;
	unsigned int br = 0;
	u8*pbuf = (u8*)P_ImageBuf;			//buffer�Ĵ�С��ҪС�������ļ�ͷ��	
											//��С��ͨ��Ϊ54					
	BITMAPFILEHEADER f_header;
	BITMAPINFOHEADER info_header;
	
	res = f_open(&f_bmp, p_path, FA_OPEN_EXISTING | FA_READ);
	
	if(res)
	{
		return -1;							//��ʧ���򷵻�ʧ�ܱ�־			
	}										//��һ��������������				
	
	res = f_read(&f_bmp, pbuf, 512, &br);				
											//λͼ�ļ�ͷ	�ṹ�帳ֵ			
	f_header = *(BITMAPFILEHEADER *)pbuf;
	//memcpy(&f_header, buffer, sizeof(BITMAPFILEHEADER));
	if(f_header.bfType != 0x4d42)
	{
		return 1;							//����BMPͼ��						
	}	
											//λͼ��Ϣͷ	�ṹ�帳ֵ			
	info_header = *(BITMAPINFOHEADER *)(pbuf + sizeof(BITMAPFILEHEADER));
	if(info_header.biHeight < 0)
	{
		info_header.biHeight = -info_header.biHeight;
	}
	f_lseek(&f_bmp, f_header.bfOffBits);	//��ָ�붨λ��λͼ������ʼ��		
	
#if LCD_CHAR_HORIZONTAL > 0
	//������ʾ
	LCD_WriteRegister(0x0003,0x1010);
	LCD_SetWindow(start_x, start_y, 		//����GRAM��ͼ����					
				   info_header.biWidth + start_x -1, 
				   info_header.biHeight + start_y - 1);
#else
	//������ʾ
	LCD_WriteRegister(0x0003,0x1028);
	LCD_SetWindow(start_x, start_y, 		//����GRAM��ͼ����					
				   info_header.biHeight + start_x -1, 
				   info_header.biWidth + start_y - 1);
#endif

	
	LCD_WriteGRAM_EN();							//��ʼ��ͼ							
	switch(info_header.biBitCount)
	{
		case 4:
		case 8:
		case 16:
		{									//��ȥ������Ϊ����ȡ2��������		
			BMP_Read16Clr1(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		case 24:
		{									//��ȥ������Ϊ����ȡ3��������		
			BMP_Read24Clr(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		case 32:
		{
			BMP_Read32Clr(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		default:			
			break;
	}
	LCD_WR_END();							//��ɻ�ͼ							
	LCD_SetWindow(0, 0, 239, 319);			//�ָ�ȫ����ͼ��
#if LCD_CHAR_HORIZONTAL > 0
	//bmpΪ���µߵ��洢,�ָ���������ʾ˳��
	LCD_WriteRegister(0x0003,0x1030);
#else
	//bmpΪ���µߵ��洢,�ָ���������ʾ˳��
	LCD_WriteRegister(0x0003,0x1038);
#endif					
	f_close(&f_bmp);						//�ر��ļ�							
	return 0;								//��������							
	
}

signed char BMP_Showbmp(unsigned int start_x,
			  			unsigned int start_y,
			  			const char *p_path)
{
	FIL f_bmp;
	FRESULT res;

	unsigned int br = 0;
	//unsigned char buffer[320*2*4];		//buffer�Ĵ�С��ҪС�������ļ�ͷ��	
	u8*pbuf = (u8*)P_ImageBuf;					//��С��ͨ��Ϊ54		
				
	BITMAPFILEHEADER f_header;
	BITMAPINFOHEADER info_header;
	
	res = f_open(&f_bmp, p_path, FA_OPEN_EXISTING | FA_READ);
	if(res)
	{
		return -1;							//��ʧ���򷵻�ʧ�ܱ�־			
	}										//��һ��������������				
	
	res = f_read(&f_bmp, pbuf, 512, &br);				
											//λͼ�ļ�ͷ	�ṹ�帳ֵ			
	f_header = *(BITMAPFILEHEADER *)pbuf;
	//memcpy(&f_header, buffer, sizeof(BITMAPFILEHEADER));
	if(f_header.bfType != 0x4d42)
	{
		return 1;							//����BMPͼ��						
	}	
											//λͼ��Ϣͷ	�ṹ�帳ֵ			
	info_header = *(BITMAPINFOHEADER *)(pbuf + sizeof(BITMAPFILEHEADER));
	if(info_header.biHeight < 0)
	{
		info_header.biHeight = -info_header.biHeight;
	}
	f_lseek(&f_bmp, f_header.bfOffBits);	//��ָ�붨λ��λͼ������ʼ��		
	
#if LCD_CHAR_HORIZONTAL > 0
	//������ʾ
	LCD_WriteRegister(0x0003,0x1010);
	LCD_SetWindow(start_x, start_y, 		//����GRAM��ͼ����					
				   info_header.biWidth + start_x -1, 
				   info_header.biHeight + start_y - 1);
#else
	//������ʾ
	LCD_WriteRegister(0x0003,0x1028);		//����������ʾ˳��
	LCD_SetWindow(start_x, start_y, 		//����GRAM��ͼ����					
				   115 + start_x, 
				   155 + start_y);
#endif

	
	LCD_WriteGRAM_EN();							//��ʼ��ͼ							
	switch(info_header.biBitCount)
	{
		case 4:
		case 8:
		case 16:
		{									//��ȥ������Ϊ����ȡ2��������		
			BMP_Read16Clr(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		case 24:
		{									//��ȥ������Ϊ����ȡ3��������		
			BMP_Read24Clr(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		case 32:
		{
			BMP_Read32Clr(&f_bmp, &info_header, pbuf, IMAGE_BUFFER_SIZE);
			break;
		}
		default:			
			break;
	}
	LCD_WR_END();							//��ɻ�ͼ							
	LCD_SetWindow(0, 0, 239, 319);			//�ָ�ȫ����ͼ��
#if LCD_CHAR_HORIZONTAL > 0
	//bmpΪ���µߵ��洢,�ָ���������ʾ˳��
	LCD_WriteRegister(0x0003,0x1030);
#else
	//bmpΪ���µߵ��洢,�ָ���������ʾ˳��
	LCD_WriteRegister(0x0003,0x1038);
#endif					
	f_close(&f_bmp);						//�ر��ļ�							
	return 0;								//��������							
	
}