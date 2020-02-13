/*
 * BoardSupport.h
 *
 *  Created on: Feb 2, 2020
 *      Author: bnezu
 */
#define GPIO_PIN_0                 ((uint16_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint16_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint16_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint16_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint16_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint16_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint16_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint16_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint16_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint16_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint16_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint16_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint16_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint16_t)0x8000)  /* Pin 15 selected   */

#define  GPIO_NOPULL        0x00000000U   /*!< No Pull-up or Pull-down activation  */
#define  GPIO_PULLUP        0x00000001U   /*!< Pull-up activation                  */
#define  GPIO_PULLDOWN      0x00000002U   /*!< Pull-down activation                */

#define GPIO_MODE             0x00000003U

#define GPIO_OUTPUT_TYPE      0x00000010U

#define GPIO_BUZZER					GPIO_PIN_9
#define GPIO_BUZZER_PIN_NUMBER		9
#define GPIO_BUZZER_PORT			GPIOC
#define GPIO_BUZZER_RCC				RCC_APB2ENR_IOPCEN


#define GPIO_BUTTON 				GPIO_PIN_9
#define GPIO_BUTTON_PORT 			GPIO_PIN_9

#ifndef BOARDSUPPORT_H_
#define BOARDSUPPORT_H_

#include "stm32f10x.h"

void initGPIO(GPIO_TypeDef* GPIOx, uint32_t pin, uint32_t pinPosition, uint32_t speed);
void initEXTI(uint32_t GPIO_PortSourceGPIOx, uint32_t GPIO_PinSource, uint32_t EXTI_Line, uint32_t EXTI_Mode, uint32_t EXTI_Trigger, uint32_t EXTI_IRQn);

#endif /* BOARDSUPPORT_H_ */
