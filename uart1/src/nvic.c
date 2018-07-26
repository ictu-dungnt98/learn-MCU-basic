#include "stm32lib.h"

/***************************************************************************************/
volatile unsigned int led_state = 0;
volatile unsigned char rx_data = 0;
volatile unsigned char command[5];
volatile unsigned char num_char = 0; 
/***************************************************************************************/
void Reserved_IRQHandler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void NMI_Handler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void HardFault_Handler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void SVC_Handler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void PendSV_Handler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void SysTick_Handler(void)
{
  while(1)
  {
    /* nothing to be run here */
  }
}

void EXTI0_IRQHandler(void)
{
  unsigned int temp;
  
  temp = read_reg(EXTI_PR,(1 << 0));
  if(temp == 1){
	  if(led_state == 0){
		  led_on(LD3_PIN,GPIO_BSRR(PORT_C));
		  led_state = 1;
	  }else{
		  led_off(LD3_PIN,GPIO_BSRR(PORT_C));
		  led_state = 0;
	  }
  }
  /* Clear pending */
  temp = 1 << 0;
  write_reg(EXTI_PR,temp);
  
  write_reg(NVIC_ICPR,(1<<5));
}

void USART1_IRQHandler(void)
{
    volatile unsigned int temp;
    
    temp = read_reg(USART_ISR, 1 << 6);
    if (0 != temp) /* Tx - TC flag */
    {
        write_reg(USART_ICR, (1<<6));		// xóa cờ ngắt TC trong ISR
    }
    temp = read_reg(USART_ISR, 1 << 5);
    if (0 != temp)  /* Rx - RXNE flag */
    {
        rx_data = read_reg(USART_RDR, 0x000000FFu);
        write_reg(USART_RQR, (1<<3)); /* ghi giá trị 1 tới USART_RQR[RXFRQ] để xóa cờ USART_ISR[RXNE] */
    }
		temp = read_reg(USART_ISR, 1 << 2);
		write_reg(USART_ICR, (1<<2));
		temp = read_reg(USART_ISR, 1 << 4);
		write_reg(USART_ICR, (1<<4));
}
/**************************************************************************************************/

void inti_interrupt(void)
{
	unsigned int tempreg;
	/* EXTI */
	tempreg = read_reg(EXTI_IMR,~(1<<0));
	tempreg |= (1<<0);
	write_reg(EXTI_IMR,tempreg);
	
	tempreg = read_reg(EXTI_RTSR,~(1<<0));
	tempreg |= (1<<0);
	write_reg(EXTI_RTSR,tempreg);
	
	/* SYSCFG */
	tempreg = read_reg(SYSCFG_EXTICR1,~(0x0F << 0));
	tempreg |= (0x00<<0);
	write_reg(SYSCFG_EXTICR1,tempreg);
	
	/* NVIC */
	/* ID interrput is 5 */
	tempreg = read_reg(NVIC_ISER,~(1 << 5));
	tempreg |= (1 << 5);
	write_reg(NVIC_ISER,tempreg);
/** ===============================  UART1  ====================================== **/	
	/* usart1 */
    write_reg(USART_ICR, 0xFFFFFFFF);
    write_reg(USART_RQR, 0xFFFFFFFF);

    /* enable Tx interrupt - TCIE in CR1 */
    tempreg = read_reg(USART_CR1, ~(1 << 6));		
    tempreg |= 1 << 6;
    write_reg(USART_CR1, tempreg);

    /* enable Rx interrupt - RXNEIE in CR1 */
    tempreg = read_reg(USART_CR1, ~(1 << 5));
    tempreg |= 1 << 5;
    write_reg(USART_CR1, tempreg);
    
    /* enable uart1 interrupt in NVIC */
    tempreg = read_reg(NVIC_ISER, ~(1 << 27));
    tempreg |= 1 << 27;
    write_reg(NVIC_ISER, tempreg);

/** ======================================================================================== **/
	
	/* Enable global interrupt*/
	asm("cpsie i");
	
	/* Disable global interrupt*/
	//asm("cpsid i");
}
