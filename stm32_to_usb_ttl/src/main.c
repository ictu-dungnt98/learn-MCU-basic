/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.3.0   2021-05-05

The MIT License (MIT)
Copyright (c) 2019 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"

#include <string.h>

#include "Debug.h"
#include "lcd_i2c.h"

/* Private macro */
#define MAXIMUM_RX_BUF_SIZE			512

/* Private variables */
static uint8_t message_uart_rx_buf[MAXIMUM_RX_BUF_SIZE];
static uint16_t pc_uart_pointer = 0;
uint32_t time_free_uart_pc	= 0;

/* Private function prototypes */
/* Private functions */
void pc_uart_put_c(char c)
{
	while( USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	USART_SendData(USART1, c);
}


void pc_uart_send_data(uint8_t data[], uint16_t length)
{
	uint16_t i = 0;
	while(i < length)
	{
		pc_uart_put_c(data[i]);
		i++;
	}
}

void clear_pc_uart_buf(void)
{
	memset(message_uart_rx_buf, 0, sizeof(message_uart_rx_buf));
	pc_uart_pointer = 0;
}

/* Interrupt for gsm uart */
void USART1_IRQHandler(void)
{

	char temp;
	if (USART_GetITStatus( USART1, USART_IT_RXNE ) != RESET) {

		/* Obtain a byte from the RX buffer register */
		temp = (char)(USART1->DR & (uint16_t)0x01FF);
		os_trace("%c", temp);
//		if(pc_uart_pointer < MAXIMUM_RX_BUF_SIZE){
//			message_uart_rx_buf[pc_uart_pointer++] = temp;
//		}
//		time_free_uart_pc = 0;

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}

}

void pc_uart_init(void)
{
	GPIO_InitTypeDef usartPin;
	USART_InitTypeDef usartDebug;
	memset(&usartPin, 0, sizeof(usartPin));
	memset(&usartDebug, 0, sizeof(usartDebug));


	usartPin.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	usartPin.GPIO_Mode = GPIO_Mode_AF;
	usartPin.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &usartPin);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	usartDebug.USART_BaudRate 				= 9600;
	usartDebug.USART_WordLength 			= USART_WordLength_8b;
	usartDebug.USART_StopBits 				= USART_StopBits_1;
	usartDebug.USART_Parity 				= USART_Parity_No;
	usartDebug.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
	usartDebug.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &usartDebug);

	/* NVIC configuration */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);

	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}



uint32_t tick_ms;
uint32_t get_tick_ms(void)
{
	return tick_ms;
}

void delay(uint32_t ms)
{
	uint32_t current_tick = get_tick_ms();

	while(get_tick_ms() - current_tick < ms);
}

void delay_ms(uint32_t time_ms)
{
	uint32_t start_time,current_time;

	start_time = get_tick_ms();

	while(1) {
		current_time = get_tick_ms();

		if (current_time >= start_time) {
			if((current_time - start_time) >= time_ms)
				break;
		}
		else {
			if(((0xFFFFFFFF - start_time) + current_time) >= time_ms)
				break;
		}
	}
}


void led_init(void)
{
	GPIO_InitTypeDef led;
	led.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	led.GPIO_Mode = GPIO_Mode_OUT;
	led.GPIO_Speed 	= GPIO_Speed_50MHz;
	led.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &led);

	// GPIO_InitTypeDef led2;
	// led2.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	// led2.GPIO_Mode = GPIO_Mode_OUT;
	// led2.GPIO_Speed 	= GPIO_Speed_50MHz;
	// led2.GPIO_PuPd = GPIO_PuPd_NOPULL;
	// GPIO_Init(GPIOB, &led2);
}

void toggle_led_pin(void)
{
	static uint32_t time_stamp = 0;

	if (get_tick_ms() - time_stamp < 500)
		return;
	
	time_stamp = get_tick_ms();

    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_6 | GPIO_Pin_7) == 0) {
		// os_trace("On\n");
        GPIO_SetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
		// GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
	}
    else {
		GPIO_ResetBits(GPIOA, GPIO_Pin_6 | GPIO_Pin_7);
		// GPIO_ResetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
		// os_trace("Off\n");
	}
}

/**g
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
	RCC_DeInit();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  /**
  *  IMPORTANT NOTE!
  *  The symbol VECT_TAB_SRAM needs to be defined when building the project
  *  if code has been located to RAM and interrupts are used. 
  *  Otherwise the interrupt table located in flash will be used.
  *  See also the <system_*.c> file and how the SystemInit() function updates 
  *  SCB->VTOR register.  
  *  E.g.  SCB->VTOR = 0x20000000;  
  */

  /* TODO - Add your application code here */
	pc_uart_init();
	os_trace("Dungnt98 debugg\n");
	
	led_init();
	lcd_i2c_init();

  /* Infinite loop */
  while (1)
  {
	toggle_led_pin();
	// i2c_send_byte(0xAA);
  }
}
