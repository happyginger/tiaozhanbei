#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "stm32f10x.h"
#include "Types.h"
#include "device.h"

//=================================================
// Port define 

// Port A
#define PA0				GPIO_Pin_0	// out
#define PA1				GPIO_Pin_1	// out
#define PA2				GPIO_Pin_2	// 
#define PA3				GPIO_Pin_3	// 
#define PA4				GPIO_Pin_4	// 
#define PA5				GPIO_Pin_5	// 
#define PA6				GPIO_Pin_6	// 
#define PA7				GPIO_Pin_7	// 
#define PA8				GPIO_Pin_8	// 
#define PA9				GPIO_Pin_9	// 
#define PA10				GPIO_Pin_10	// 
#define PA11				GPIO_Pin_11	// 
#define PA12				GPIO_Pin_12	// 
#define PA13				GPIO_Pin_13	// 
#define PA14				GPIO_Pin_14	// 
#define PA15				GPIO_Pin_15	// 



#define USART1_TX		        PA9	// out
#define USART1_RX		        PA10	// in 
//#define USART2_TX		        PA2	// out
//#define USART2_RX		        PA3	// in 
#define LED1                            PA0
#define LED2                            PA1

//Port B

#define PB0                             GPIO_Pin_0
#define PB1				GPIO_Pin_1	// 
#define PB2				GPIO_Pin_2	// 
#define PB3				GPIO_Pin_3	// 
#define PB4				GPIO_Pin_4	// 
#define PB5				GPIO_Pin_5	// 
#define PB6				GPIO_Pin_6	// 
#define PB7                             GPIO_Pin_7
#define PB8                             GPIO_Pin_8
#define PB9                             GPIO_Pin_9
#define PB10                            GPIO_Pin_10
#define PB11                            GPIO_Pin_11
#define PB12                            GPIO_Pin_12
#define PB13                            GPIO_Pin_13
#define PB14                            GPIO_Pin_14
#define PB15                            GPIO_Pin_15


#define WIZ_SCS			        PB12	// out
#define WIZ_SCLK			PB13	// out
#define WIZ_MISO			PB14	// in
#define WIZ_MOSI			PB15	// out


#define WIZ_RESET		        PB9	// out




//

// Port C
#define PC6                             GPIO_Pin_6
#define PC10                            GPIO_Pin_10
#define PC11                            GPIO_Pin_11
#define PC13				GPIO_Pin_13	//
#define PC14				GPIO_Pin_14	//
#define PC15				GPIO_Pin_15	//

#define WIZ_INT			        PC6	// in

#define UART4_TX                        PC10  //out
#define UART4_RX                        PC11  //in
//Port D

//=================================================
#pragma pack(1)
typedef struct _CONFIG_MSG
{
  uint8 op[4];//header: FIND;SETT;FACT...
  uint8 mac[6];
  uint8 sw_ver[2];
  uint8 lip[4];
  uint8 sub[4];
  uint8 gw[4];
  uint8 dns[4];	
  uint8 dhcp;
  uint8 debug;

  uint16 fw_len;
  uint8 state;
  
}CONFIG_MSG;
#pragma pack()

//



#define MAX_BUF_SIZE		1460
#define KEEP_ALIVE_TIME	        30	// 30sec


#define ON	1
#define OFF	0

#define HIGH		1
#define LOW		0

#define __GNUC__

// SRAM address range is 0x2000 0000 ~ 0x2000 4FFF (20KB)
#define SOCK_BUF_ADDR 	0x20000000


extern CONFIG_MSG  ConfigMsg, RecvMsg;

#define JPG_BUF_SIZE  1024*24 //24KB

#define SOCK_HTTP             0
#define SOCK_WEBSOCKET        2
#define SOCK_DHCP             7
#define PictureAddress          0x08008000 //camera picture stored address in flash: total 64KB, from 32K 

#define AppBackAddress  0x08024000 //app size is 112K, 32KB webpage space included, app program is from 0x08008000 to 0x0801C000

#define WebpageAddress          0x0801C000 //webpage from 0x0801C000 to 0x08024000, 32KB in total


#endif