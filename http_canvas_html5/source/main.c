#include "mcu_init.h"
#include "config.h"
#include "device.h"
#include "w5500/spi2.h"

#include "weibo.h"
#include "httputil.h"

#include "iic.h"
#include "ov2640.h"

//html5
#include "sha1.h"
#include "base64.h"
#include "websocket.h"

#include <stdio.h>


vu32 ms=0;
vu8 sec=0,min=0;
vu8 hour=0;
vu32 sysTick=0;

uint8 reboot_flag=0;

//camera

u32 jpg_len=0;
u32 rd_ptr=0;
u32 jpg_offset=0;

u32 JPEGCnt=0;

u8 JPEGBuffer[JPG_BUF_SIZE];
u8 Flag_JPEGrec_Completed=0;
uint8 jpg_flag=0;
uint8 jpg_requested=0;
//#pragma location = "PictureAddress"
//__root const char CAMERA_PICTURE[];
//camera



void main()
{
  
  RCC_Configuration(); // Configure the system clocks
  GPIO_Configuration(); //GPIO configuration
  NVIC_Configuration(); //NVIC Configuration
  Timer_Configuration();
  
  iic_init();
  USART1_Init(); //115200@8-n-1
  //UART4_Init(38400);//camera
  printf("MCU initialized.\r\n");
  Reset_W5500();
  printf("Ethernet initialized over.\r\n");
  WIZ_SPI_Init();
  
  set_network();
  
  printf("W5500 is ready!\r\n");
  
  while(1)
  {
    do_websocket_server(SOCK_WEBSOCKET);
    
    do_http();   
  }
}
/**************************/
/*****Timer2 interrupt*****/
/**************************/
void Timer2_ISR(void)
{
 
  ms++;//mili second
  sysTick++;//system tick
  if(ms>=1000)//second
  {
    ms=0;
    sec++;

  }
  if(sec>=60)//minute
  {
    sec=0;
    min++;

  }
  if(min>=60)//hour
  {
    min=0;
    hour++;
  }
  if(hour>=24)//day
  {
    hour=0;
    
  }
}