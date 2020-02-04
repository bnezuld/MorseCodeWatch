/**
  ******************************************************************************
  * @file    stm32f1xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11-February-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"


/** @addtogroup IO_Toggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern uint32_t ticks, startTick;
uint32_t pressed = 0;
//extern struct node *buttonPressesHead, *buttonPressesEnd;
uint32_t button[300],buttonCount = 0, max = 300;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M Processor Exceptions Handlers                          */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	ticks++;
	if(pressed == 1 && GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_0) == Bit_RESET){//read pin
		GPIOC->BSRR = (uint32_t)GPIO_PIN_9;
		uint32_t dif = ticks - startTick;
		pressed = 0;
		startTick = ticks;
		if(buttonPressesHead == 0)
		{
			buttonPressesHead = initializeNode(dif);
			buttonPressesEnd = buttonPressesHead;
		}else
		{
			addNode(dif, buttonPressesEnd);
			buttonPressesEnd = buttonPressesEnd->next;
		}
	}
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_md.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
		uint32_t dif = ticks - startTick;
    	if(dif > 100){
    		startTick = ticks;
    		pressed = 1;

    		GPIOC->BSRR = (uint32_t)GPIO_PIN_9 << 16U;

    		if(dif > 1500)
    		{
    			char str = ' ';
    			Translate(buttonPressesHead, &str);
    			/*do nothing b/c i want the first node to be how long button is held*/
    			//buttonPressesHead = initializeNode(dif);
    			//buttonPressesEnd = buttonPressesHead;
    		}else
    		{
    			addNode(dif, buttonPressesEnd);
    			buttonPressesEnd = buttonPressesEnd->next;
    		}
    	}
    	/* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
