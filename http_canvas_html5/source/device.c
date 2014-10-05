
#include "config.h"
#include "w5500/socket.h"
#include "util.h"
#include "w5500/w5500.h"
#include "device.h"
#include "dhcp.h"

#include <stdio.h> 
#include <string.h>


CONFIG_MSG  ConfigMsg, RecvMsg;

extern uint8 reboot_flag;

uint8 txsize[MAX_SOCK_NUM] = {8,0,8,0,0,0,0,0};
uint8 rxsize[MAX_SOCK_NUM] = {2,2,2,2,2,2,2,2};
//public buffer for DHCP, DNS, HTTP
uint8 BUFPUB[1460];
uint8 * strDHCPERR[]  = {"DHCP:NONE", "DHCP:ERR", "DHCP:TIMEOUT", "DHCP:UPDATE", "DHCP:CONFLICT", 0}; 


void Reset_W5500(void)
{
  GPIO_ResetBits(GPIOB, WIZ_RESET);
  delay_us(2);  
  GPIO_SetBits(GPIOB, WIZ_RESET);
  delay_ms(150); 
}

void USART1_Init(void)
{
  USART_InitTypeDef USART_InitStructure;

  /* USARTx configuration ------------------------------------------------------*/
  /* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //Enable rx enable, 
  
  /* Configure the USARTx */ 
  USART_Init(USART1, &USART_InitStructure);
  /* Enable the USARTx */
  USART_Cmd(USART1, ENABLE);
}

void UART4_Init(uint16 baudrate)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

  /* Clock configuration -------------------------------------------------------*/

  /* Configure the GPIO ports( UART4 Transmit and Receive Lines) */
  /* Configure the UART4_Tx as Alternate function Push-Pull */
  /* Configure UART4_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Configure UART4_Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  


  /* UART4 configuration ------------------------------------------------------*/
  /* UART4 configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure the UART4 */
  USART_Init(UART4, &USART_InitStructure);

        /* Enable UART4 interrupt */
        USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

  /* Enable the UART4 */
  USART_Cmd(UART4, ENABLE);
}
//reboot 
void reboot(void)
{
  pFunction Jump_To_Application;
  uint32 JumpAddress;
  JumpAddress = *(vu32*) (0x00000004);
  Jump_To_Application = (pFunction) JumpAddress;
  Jump_To_Application();
}

void maxHttpTxBuf16(void)
{
  uint8 txBufSize[8]={16,0,0,0,0,0,0,0};
  uint8 rxBufSize[8]={2,2,2,2,2,2,2,2};
  sysinit(txBufSize, rxBufSize); 
}
void normalSocketBuf(void)
{
  uint8 txBufSize[8]={2,2,2,2,2,2,2,2};
  uint8 rxBufSize[8]={2,2,2,2,2,2,2,2};
  sysinit(txBufSize, rxBufSize); 
}

void set_network(void)
{
  uint8 ip[4];
  uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
  uint8 lip[4]={192,168,1,111};
  uint8 sub[4]={255,255,255,0};
  uint8 gw[4]={192,168,1,1};
  setSHAR(mac);
  setSUBR(sub);
  setGAR(gw);
  setSIPR(lip);

  //Init. TX & RX Memory size of w5500
  sysinit(txsize, rxsize); 
  
  setRTR(2000);//200ms
  setRCR(3);
  
  ConfigMsg.debug=1;
  getSIPR (ip);
  if (ConfigMsg.debug)
  {
    printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  }
  getSUBR(ip);
  if (ConfigMsg.debug) 	
  {
    printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  }
  getGAR(ip);
  if (ConfigMsg.debug) 	
  {
    printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  }
}

void do_dhcp(void)
{
  uint8 dhcpret=0;

  dhcpret = check_DHCP_state(SOCK_DHCP);
  if((dhcpret != DHCP_RET_NONE) && ConfigMsg.debug) 
    printf("%s\r\n",strDHCPERR[dhcpret]);         
  switch(dhcpret)
  {
    case DHCP_RET_NONE:
      break;
    case DHCP_RET_TIMEOUT:

      set_network();//set network again
      break;
    case DHCP_RET_UPDATE:
      write_config_to_eeprom();
      get_config();
      set_network();
      
      break;
    case DHCP_RET_CONFLICT:
      //reboot();
      reboot_flag = 1;
      if(ConfigMsg.debug) printf("ip address conflict\r\n");
      
    default:
      break;
  }

}