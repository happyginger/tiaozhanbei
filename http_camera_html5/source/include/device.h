#ifndef _DEVICE_H_
#define _DEVICE_H_

#define DEVICE_ID "W5500"

#define FW_VER_HIGH  2
#define FW_VER_LOW    0


#define CONFIG_MSG_SIZE		sizeof(CONFIG_MSG)-4	

extern uint8 BUFPUB[1460];
#define gBUFPUBLIC BUFPUB;

typedef  void (*pFunction)(void);


void set_network(void);

void write_config_to_eeprom(void);

void Reset_W5500(void);

void USART1_Init(void);

void UART4_Init(uint16 baudrate);

void reboot(void);

void get_config(void);

void do_dhcp(void);

void normalSocketBuf(void);
void maxHttpTxBuf16(void);
#endif