/*
 * BoardSupport.c
 *
 *  Created on: Feb 2, 2020
 *      Author: bnezu
 */

#include "BoardSupport.h"

void initGPIO(GPIO_TypeDef* GPIOx, uint32_t pin, uint32_t pinPosition, uint32_t speed)
{
	GPIOx->BSRR |= (uint32_t)pin << 16U;//set BSRR high

	  if(pin > 7)
	  {
		  uint32_t pinOffset = (pinPosition - 8) * 4;
		  uint32_t temp = GPIOx->CRH/*reset value*/ & ~((uint32_t)0x0F << pinOffset);
		  temp |= speed << pinOffset;
		  GPIOx->CRH = temp;
	  }else
	  {
		  uint32_t pinOffset = (pinPosition - 1) * 4;
		  uint32_t temp = GPIOx->CRL & ~((uint32_t)0x0F << pinOffset);
		  temp |= speed << pinOffset;
		  GPIOx->CRL = temp;
	  }

	  //GPIOx->ODR |= (uint32_t)(0x1 << pin/*pin*/);
}

void initEXTI(uint32_t GPIO_PortSourceGPIOx, uint32_t GPIO_PinSource, uint32_t EXTI_Line, uint32_t EXTI_Mode, uint32_t EXTI_Trigger, uint32_t EXTI_IRQn)
{
	/* Connect Button EXTI Line to Button GPIO Pin */
	AFIO->EXTICR[GPIO_PinSource >> 0x02] &= ~((uint32_t)0x0F) << (0x04 * (GPIO_PinSource & (uint8_t)0x03));
	AFIO->EXTICR[GPIO_PinSource >> 0x02] |= (((uint32_t)GPIO_PortSourceGPIOx) << (0x04 * (GPIO_PinSource & (uint8_t)0x03)));

	/* Clear EXTI line configuration */
	EXTI->IMR &= ~EXTI_Line;
	EXTI->EMR &= ~EXTI_Line;

	uint32_t tmp = (uint32_t)EXTI_BASE;
	tmp += EXTI_Mode;

	*(__IO uint32_t *) tmp |= EXTI_Line;

	/* Clear Rising Falling edge configuration */
	EXTI->RTSR &= ~EXTI_Line;
	EXTI->FTSR &= ~EXTI_Line;

	/* Select the trigger for the selected external interrupts */
	tmp = (uint32_t)EXTI_BASE;
	tmp += EXTI_Trigger;

	*(__IO uint32_t *) tmp |= EXTI_Line;

	/* Enable and set Button EXTI Interrupt to the lowest priority */
	uint32_t tmppriority = 0x00, tmppre = 0x00, tmpsub = 0x0F;

	/* Compute the Corresponding IRQ Priority --------------------------------*/
	tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
	tmppre = (0x4 - tmppriority);
	tmpsub = tmpsub >> tmppriority;

	tmppriority = (uint32_t)0x0F << tmppre;
	tmppriority |=  0x0F & tmpsub;
	tmppriority = tmppriority << 0x04;

	NVIC->IP[EXTI_IRQn] = tmppriority;

	/* Enable the Selected IRQ Channels --------------------------------------*/
	NVIC->ISER[EXTI_IRQn>> 0x05] =
	(uint32_t)0x01 << (EXTI_IRQn & (uint8_t)0x1F);
}

void initI2C()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef  I2C_InitStructure;

	// I2C Clock Enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	// GPIOx clock enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	// Reset I2C peripheral
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);

	// Connect I2C pins to AF
	//GPIO_PinAFConfig(GPIOB, LCD_SCL_GPIO_PinSourcex, LCD_GPIO_AF_I2Cx);
	//GPIO_PinAFConfig(GPIOB, LCD_SDA_GPIO_PinSourcex, LCD_GPIO_AF_I2Cx);

	// I2C GPIO pin configuration
	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_10;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_11;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// I2C struct init
	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_ClockSpeed          = 80000;//100 khz
	I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;//16_9;
	I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
	I2C_InitStructure.I2C_OwnAddress1		  = 0x20;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C2,&I2C_InitStructure);
	I2C_Cmd(I2C2, ENABLE);

}
