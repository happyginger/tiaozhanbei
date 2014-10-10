#ifndef NVIC_EXIT_H_H_
#define NVIC_EXIT_H_H_

#include "stm32f10x.h"
#include "Rename_Pin.h"

#define SET_PORT_L(PORT, Pin)	(PORT)->BRR = (Pin)
#define SET_PORT_H(PORT, Pin)	(PORT)->BSRR = (Pin)

//ͨ�����ų�ʼ������
void GpioInit(GPIO_TypeDef* gpio_port, 
			   u16 gpio_pin,
			   GPIOMode_TypeDef gpio_pin_mode,
			   GPIOSpeed_TypeDef gpio_pin_speed);

//����NVIC�����Table���λ��
void NVIC_Config(uint32_t NVIC_PriorityGroup);
//���жϳ�ʼ��
void Exit_Init(GPIO_TypeDef* gpio_port,
		 	   u16 gpio_pin,
			   GPIOMode_TypeDef gpio_pin_mode,
			   EXTITrigger_TypeDef exit_type,
			   u8 pre_pri,
			   u8 sub_pri);

#endif
