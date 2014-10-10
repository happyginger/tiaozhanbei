#ifndef BMP_H_H_
#define BMP_H_H_

#include "../fatfs/FATFS_008.h"

#include "stm32f10x.h"

#define RGB555		0
#define RGB565		1

//��һ����Ϊλͼ�ļ�ͷBITMAPFILEHEADER,			
//��һ���ṹ���䶨�����£�						
//����ṹ�ĳ����ǹ̶��ģ�Ϊ14���ֽ�			
//WORDΪ�޷���16λ������DWORDΪ�޷���32λ������	
#pragma pack (1)		//�����������������1�ֽڶ���
typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;  		//0x00~0x01//ָ���ļ����ͣ�ͨ��Ϊ0x424D�����ַ���"BM"	
	DWORD bfSize;  		//0x02~0x05//ָ���ļ���С��������14���ֽ�				
	
	WORD bfReserved1; 	//0x06~0x07//Ϊ�����֣����ÿ���							
	WORD bfReserved2; 	//0x08~0x09//Ϊ�����֣����ÿ���							

	DWORD bfOffBits; 	//0x0a~0x0d//Ϊ���ļ�ͷ��ʵ�ʵ�λͼ���ݵ�ƫ���ֽ�����	
						//��ǰ�������ֵĳ���֮��								
} BITMAPFILEHEADER;

//__attribute__ ((packed)) 
//�ڶ�����Ϊλͼ��ϢͷBITMAPINFOHEADER��Ҳ��һ��
//�ṹ�壬�䶨�����£�							
//���������BMPͼ���� BITMAPINFOHEADER�ṹ��	
//������ΪΪ40���ֽڣ�WORDΪ�޷���16λ������	
//DWORD�޷���32λ����,LONGΪ32λ������			
typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize; 	 	//0x0e~0x11//˵��BITMAPINFOHEADER�ṹ����Ҫ���ֽ���(40)	

	LONG biWidth;  		//0x12~0x15//ָ��ͼ��Ŀ�ȣ���λ������					
	LONG biHeight;  	//0x16~0x9//ָ��ͼ��ĸ߶ȣ���λ������					

	WORD biPlanes;  	//0x1a~0xb//ΪĿ���豸˵��λ��������ֵ�����Ǳ���Ϊ1		

	WORD biBitCount; 	//0x1c~0x0d//ָ����ɫλ��,Ϊ1(2ɫͼ),4(16ɫ),8(256ɫ)	
						//		   16(�߲�ɫ),24(���ɫͼ),32(��ǿ�����ɫλͼ)	
	DWORD biCompression;//0x1e~0x21//ѹ��˵���� 								
# define BI_RGB			0x00	//0 - ��ѹ�� (ʹ��BI_RGB��ʾ) 	
# define BI_RLEB		0x01	//1 - RLE 8-ʹ��8λRLEѹ����ʽ(��BI_RLE8��ʾ) 	
# define BI_RLE4		0x02	//2 - RLE 4-ʹ��4λRLEѹ����ʽ(��BI_RLE4��ʾ) 	
# define BI_BITFIELDS	0x03	//3 - Bitfields-λ���ŷ�ʽ(��BI_BITFIELDS��ʾ)
	DWORD biSizeImage; 	//0x22~0x25//ָ��ʵ�ʵ�λͼ����ռ�õ��ֽ�����ʵҲ���Դ����µĹ�ʽ�м��������biSizeImage=biWidth'*biHeightҪע����ǣ�������ʽ�е�biWidth'������4�������������Բ���biWidth������biWidth'����ʾ���ڻ����biWidth�ģ���4����� ���������ٸ����ӣ����biWidth=240����biWidth'=240�����biWidth=241��biWidth'=244����� biCompressionΪBI_RGB����������Ϊ��
						
	LONG biXPelsPerMeter; //0x26~0x29//ָ��Ŀ���豸��ˮƽ�ֱ��ʣ���λ��ÿ�׵����ظ���
	LONG biYPelsPerMeter; //0x2a~0x2d//ָ��Ŀ���豸�Ĵ�ֱ�ֱ��ʣ���λͬ�ϡ�		

	DWORD biClrUsed; 	  //0x2e~0x31//ָ����ͼ��ʵ���õ�����ɫ���������ֵΪ�㣬���õ�����ɫ��Ϊ2��biBitCount�η���
	DWORD biClrImportant; //0x32~0x35//ָ����ͼ������Ҫ����ɫ���������ֵΪ�㣬����Ϊ���е���ɫ������Ҫ�ġ�
}BITMAPINFOHEADER; 
	
//��������Ϊ��ɫ��(Palette),��Ȼ,�����Ƕ���Щ��	
//Ҫ��ɫ���λͼ�ļ����Եġ���Щλͼ�������ɫͼ
//�ǲ���Ҫ��ɫ���,BITMAPINFOHEADER��ֱ����λͼ	
//���ݡ�										
//��ɫ��ʵ������һ�����飬����biClrUsed��Ԫ��	
//(�����ֵΪ�㣬����2��biBitCount�η���Ԫ��)	
//������ÿ��Ԫ�ص�������һ��RGBQUAD�ṹ��ռ4����
//�ڣ��䶨�����£�								
typedef struct tagRGBQUAD{
	BYTE rgbBlue;  					//����ɫ����ɫ����							
	BYTE rgbGreen;  				//����ɫ����ɫ����							
	BYTE rgbRed;  					//����ɫ�ĺ�ɫ����							
	BYTE rgbReserved; 				//����ֵ									
}RGBQUAD;
#pragma pack () 
//���Ĳ��־���ʵ�ʵ�ͼ�������ˡ������õ���ɫ���λͼ��ͼ�����ݾ��Ǹ��������ڵ�ɫ
//���е�����ֵ���������ɫͼ��ͼ�����ݾ���ʵ�ʵ�R,G,Bֵ��						
//Ҫע�����㣺																	
//1��ÿһ�е��ֽ���������4����������������ǣ�����Ҫ���롣����ǰ�����			
//	 biSizeImageʱ�Ѿ��ᵽ�ˡ�													
//2��һ����˵��.BMP�ļ������ݴ��µ��ϣ������ҵġ�Ҳ����˵�����ļ������ȶ�����	
//��ͼ��������һ�е���ߵ�һ�����أ�Ȼ������ߵڶ������ء��������ǵ����ڶ������
//��һ�����أ���ߵڶ������ء��������ƣ����õ�����������һ�е�����һ�����ء�
	
//4������ʾ
signed char BMP_Showbmp(unsigned int start_x,
			  			unsigned int start_y,
			  			const char *p_path);
//��ͼ��ʾ
signed char BMP_Showbmp1(unsigned int start_x,
			  			unsigned int start_y,
			  			const char *p_path);
void BMP_Read16Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size);
void BMP_Read24Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size);
void BMP_Read32Clr(FIL *p_fbmp, BITMAPINFOHEADER *p_iheader, 
				   unsigned char *p_buf, unsigned int buf_size);
				   
//дBMP								
void BMP_bmpInit(long bmp_width,
				 long bmp_height,
				 unsigned int bmp_bitcount,
				 BITMAPFILEHEADER *p_fheader, BITMAPINFOHEADER *p_info_header,
				 u8 bmp_type);					
u32 BMP_MakeBmpFile(long bmp_width, 
					long bmp_height,
					FIL* p_bmp,
					const char *p_path,
					u8 bmp_type);
u8 BMP_FillBmp(FIL* p_bmp,
				 u32 fill_size,
				 void* p_buf,
				 u32 buf_size);
# endif
