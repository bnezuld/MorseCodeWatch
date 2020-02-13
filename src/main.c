/*
******************************************************************************
File:     main.c
Info:     Generated by Atollic TrueSTUDIO(R) 9.0.0   2020-02-01

The MIT License (MIT)
Copyright (c) 2018 STMicroelectronics

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
#include <stddef.h>
#include "BoardSupport.h"
#include "LinkedList.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Private typedef */
/* Private define  */
#define mainQUEUE_LENGTH 10
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

#define mainQUEUE_SEND_FREQUENCY_MS			( 50 / portTICK_PERIOD_MS )


/* Private macro */
/* Private variables */
extern uint32_t BEEP_TICK_LENGTH;
extern uint32_t SPACE_TICK_LENGTH;

static QueueHandle_t buttonQueue = NULL, messageQueue = NULL, sendMessageQueue = NULL, displayQueue = NULL, uart1Queue = NULL;
static SemaphoreHandle_t  semaphorePolling = NULL, semaphoreISR = NULL;
static TimerHandle_t buttonReleaseTimer = NULL, DisplaySpaceTimer = NULL, DisplayBeepTimer = NULL;

TaskHandle_t  RecordButtonPressesTask = NULL;

struct ButtonPress{
	uint32_t time;
	uint8_t buttonState;
};

/* Private function prototypes */
//task function(s)
static void PollingTask( void *pvParameters );
static void RecordButtonPresses( void *pvParameters );
static void Menu( void *pvParameters );
static void SendMessage( void *pvParameters );
static void UartMessage( void *pvParameters );


//timer callback function(s)
static void TranslateMorseCode( TimerHandle_t xTimer );
static void DisplayOn( TimerHandle_t xTimer );
static void DisplayOff( TimerHandle_t xTimer );


/* Private functions */

static void DisplayOn( TimerHandle_t xTimer )
{
	int8_t ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

	if(ulCount == 0 )
	{
		if(xQueueReceive( displayQueue, &ulCount, 0) == pdTRUE)
		{
			vTimerSetTimerID( DisplaySpaceTimer, ( void * ) ulCount );
			GPIOC->BSRR = (uint32_t)GPIO_BUZZER_OUT << 16U;
			xTimerReset(DisplaySpaceTimer, 0);
			return;
		}
		else{
			GPIOC->BSRR = (uint32_t)GPIO_BUZZER_OUT << 16U;
		}
	}else{
		ulCount--;
		vTimerSetTimerID( xTimer, ( void * ) ulCount );
		xTimerReset(DisplayBeepTimer, 0);
	}
	//TODO- give semaphore to send message
}

static void DisplayOff( TimerHandle_t xTimer )
{
	int8_t ulCount = ( uint32_t ) pvTimerGetTimerID( xTimer );

	if(ulCount == 0 )
	{
		if(xQueueReceive( displayQueue, &ulCount, 0) == pdTRUE)
		{
			vTimerSetTimerID( DisplayBeepTimer, ( void * ) ulCount );
			GPIOC->BSRR = (uint32_t)GPIO_BUZZER_OUT;
			xTimerReset(DisplayBeepTimer, 0);
			return;
		}else{
			GPIOC->BSRR = (uint32_t)GPIO_BUZZER_OUT << 16U;
		}
	}else{
		ulCount--;
		vTimerSetTimerID( xTimer, ( void * ) ulCount );
		xTimerReset(DisplaySpaceTimer, 0);
	}
	//TODO- give semaphore to send message
}

static void UartMessage( void *pvParameters ){
	char *message;
	char c;
	uint32_t size = 1, place = 0;
	uint8_t queued = 1;
	for(;;){
		xQueueReceive( uart1Queue, &c, portMAX_DELAY );
		if(c == '\r'){
			queued = 1;
			size = 1;
			place = 0;
			xQueueSend( sendMessageQueue, &message, 0);
			//queue message(will be freed here)
		}else{
			char *tmp = message;
			message = malloc((size++ + 1) * sizeof(char));
			if(queued == 0){
				for(uint32_t i = 0; i < place; i++)
				{
					message[i] = tmp[i];
				}
				free(tmp);
			}else{
				queued = 0;
			}
			message[place++] = c;
			message[place] = '\0';
		}
	}
}

static void SendMessage( void *pvParameters )
{
	char* message;
	for(;;){
		//TODO- wait for semaphore to send message
		xQueueReceive( sendMessageQueue, &message, portMAX_DELAY );
		char* tmpMsg = message;
		int resetTimer = 1;//only reset timer once, although there is still a possible race condition if the queue empties before it finishes processing this message
		while(*tmpMsg != '\0')
		{
			USART_SendData(USART1, *tmpMsg);
			while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
			//translate message
			//queue up message for timers to use
			char* c =  TranslateCharToMorseCode(*tmpMsg);
			uint8_t validNextChar = *(tmpMsg + 1) != ' ';
			while(*c != '\0')
			{
				int8_t val = -1;
				if(*c == '.')
					val = 0;
				else if(*c == '-')
					val = 2;

				if(val >= 0){
					xQueueSend( displayQueue, &val, portMAX_DELAY);//beep
					val = 0;
					if(*(c + 1) == '\0')
					{
						if(validNextChar == 1){
							val = 2;
							xQueueSend( displayQueue, &val, portMAX_DELAY);//space
						}
						else{
							val = 6;
							xQueueSend( displayQueue, &val, portMAX_DELAY);//space
						}
					}else
					{
						xQueueSend( displayQueue, &val, portMAX_DELAY);//space
					}
					if(resetTimer == 1){
						xTimerReset(DisplaySpaceTimer, 0);
						resetTimer = 0;
					}
				}
				c++;
			}
			tmpMsg++;
		}
		free(message);
		//vTimerSetTimerID( DisplayBeepTimer, ( void * )  tmpInt);
	}
}


static void TranslateMorseCode( TimerHandle_t xTimer )
{
	//stop recorButtonPresses task(can i use semaphore, i think beter to do this since is activated by a timer, block time must be zero if using semaphore)
	vTaskSuspend(RecordButtonPressesTask);
	xSemaphoreTake(semaphoreISR, 0);//try to clear the semaphore for the ISR
	xSemaphoreTake(semaphorePolling, 0);//try to clear the semaphore for the ISR

	char* message = TranslateSelf();

	xQueueSend( messageQueue, &message, 1);

	//start recorButtonPresses task
	vTaskResume(RecordButtonPressesTask);
	xSemaphoreGive(semaphoreISR);//try to clear the semaphore for the ISR
}

static void Menu( void *pvParameters )
{
	char *message = NULL;
	for(;;){
		xQueueReceive( messageQueue, &message, portMAX_DELAY );
		if(strcmp(message, "T") == 0)
		{
			free(message);
			asm("nop");
		}else if(strcmp(message, "M") == 0)
		{
			free(message);
			xQueueReceive( messageQueue, &message, portMAX_DELAY );
			asm("nop");
			free(message);
		}else if(strcmp(message, "N") == 0)
		{
			free(message);
			char* test = malloc(2 * sizeof(char));
			test[0] = 'N';
			test[1] = '\0';
			xQueueSend( sendMessageQueue, &test, 0 );
			xQueueReceive( messageQueue, &message, portMAX_DELAY );
			xQueueSend( sendMessageQueue, &message, 0 );
		}
	}
}

static void RecordButtonPresses( void *pvParameters )
{
	struct ButtonPress buttonRecord;
	char *message;
	for(;;)
	{
		//wait for something to be in the queue for portMAX_DELAY and record it
		xQueueReceive( buttonQueue, &buttonRecord, portMAX_DELAY );
		ButtonPress(buttonRecord.time,buttonRecord.buttonState);
	}
}

static void PollingTask( void *pvParameters )
{
	TickType_t startTicks = 0;
	struct ButtonPress buttonRecord;
	for(;;)
	{
		//wait for semaphore from interrupt
		if( semaphorePolling != NULL )
		{
			/* See if we can obtain the semaphore.  If the semaphore is not
			available wait 10 ticks(can maybe increase this to max so it waits forever) to see if it becomes free. */
			if( xSemaphoreTake( semaphorePolling, ( TickType_t ) 10 ) == pdTRUE ){
				xTimerStop(buttonReleaseTimer, 0);

				TickType_t endTicks, difference;
				/* Record button press */
				difference = xTaskGetTickCount() - startTicks;

				buttonRecord.buttonState = 0;//time from when it was released
				buttonRecord.time = difference;
				xQueueSend( buttonQueue, &buttonRecord, 0 );

				GPIOC->BSRR |= (uint32_t)GPIO_BUZZER;

				startTicks = xTaskGetTickCount();

				/* Buton release polling */
				while(GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_0) != Bit_RESET){
					//wait for the button to be unpressed(or maybe can connect same button to a interrupt that can release and it will wait for that semaphore?)
					TickType_t tmpTicks = xTaskGetTickCount();
					vTaskDelayUntil(&tmpTicks, mainQUEUE_SEND_FREQUENCY_MS );
					//no operation(used to keep empty while loop working)
					asm("nop");
				}

				/* Record button release time*/
				GPIOC->BSRR = (uint32_t)GPIO_BUZZER << 16U;

				endTicks = xTaskGetTickCount();
				difference = endTicks - startTicks;

				buttonRecord.buttonState = 1;
				buttonRecord.time = difference;
				xQueueSend( buttonQueue, &buttonRecord, 1);

				//start Timer, to call the translate task
				xTimerReset(buttonReleaseTimer, 0);
				xTimerStart(buttonReleaseTimer, 0);

				//record start ticks
				startTicks = xTaskGetTickCount();

				//block so ISR semaphore in ISR cannot be triggerd for a period of time
				vTaskDelayUntil( &endTicks, mainQUEUE_SEND_FREQUENCY_MS );

				//release semaphoreISR (giving the semaphore so ISR can happen and give this task the semaphore it needs)
	            xSemaphoreGive( semaphoreISR );
			}
		}
	}
}

void EXTI0_IRQHandler(void)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
    	if(xSemaphoreTakeFromISR( semaphoreISR, &xHigherPriorityTaskWoken ) == pdTRUE){
	        xSemaphoreGiveFromISR( semaphorePolling, &xHigherPriorityTaskWoken );
    	}
    	/* Clear interrupt flag */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void USART1_IRQHandler(void)
{
	/* USER CODE BEGIN USART1_IRQn 0 */
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		char data = USART_ReceiveData(USART1);
		xQueueSendFromISR( uart1Queue, &data, xHigherPriorityTaskWoken);
	}

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	/* USER CODE END USART1_IRQn 0 */

	/* USER CODE BEGIN USART1_IRQn 1 */

	/* USER CODE END USART1_IRQn 1 */
}


/**
**===========================================================================
**
**  Abstract: main program
**
**===========================================================================
*/
int main(void)
{
  	int i = 0;

	/**
	*  IMPORTANT NOTE!
	*  The symbol VECT_TAB_SRAM needs to be defined when building the project
	*  if code has been located to RAM and interrupts are used.
	*  Otherwise the interrupt table located in flash will be used.
	*  See also the <system_*.c> file and how the SystemInit() function updates
	*  SCB->VTOR register.
	*  E.g.  SCB->VTOR = 0x20000000;
	*/

  	//give priority for preemption
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

    GPIO_InitTypeDef gpioc_init_struct;

	/* Enable timer for ports */
	RCC->APB2ENR |= GPIO_BUZZER_RCC;//port C
    gpioc_init_struct.GPIO_Pin = GPIO_BUZZER;
    gpioc_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioc_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &gpioc_init_struct);

    gpioc_init_struct.GPIO_Pin = GPIO_BUZZER_OUT;
	gpioc_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpioc_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &gpioc_init_struct);

	//initGPIO(GPIO_BUZZER_PORT, GPIO_BUZZER, GPIO_BUZZER_PIN_NUMBER, GPIO_Speed_50MHz);
	//initGPIO(GPIO_BUZZER_OUT_PORT, GPIO_BUZZER_OUT, GPIO_BUZZER_OUT_PIN_NUMBER, GPIO_Speed_50MHz);

	/* Enable the BUTTON Clock */
	RCC->APB2ENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO;//port A

	/* Configure Button pin as input floating */
	initGPIO(GPIOA, GPIO_Pin_0, 0, GPIO_Mode_IN_FLOATING);
	initEXTI(GPIO_PortSourceGPIOA, GPIO_PinSource0, EXTI_Line0, EXTI_Mode_Interrupt, EXTI_Trigger_Rising, EXTI0_IRQn);

    /* USART configuration structure for USART1 */
    USART_InitTypeDef usart1_init_struct;
    /* Bit configuration structure for GPIOA PIN9 and PIN10 */
    GPIO_InitTypeDef gpioa_init_struct;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enalbe clock for USART1, AFIO and GPIOA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_GPIOA, ENABLE);

    /* GPIOA PIN9 alternative function Tx */
    gpioa_init_struct.GPIO_Pin = GPIO_Pin_9;
    gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioa_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpioa_init_struct);
    /* GPIOA PIN9 alternative function Rx */
    gpioa_init_struct.GPIO_Pin = GPIO_Pin_10;
    gpioa_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
    gpioa_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpioa_init_struct);

    /* Enable USART1 */
    USART_Cmd(USART1, ENABLE);
    /* Baud rate 9600, 8-bit data, One stop bit
     * No parity, Do both Rx and Tx, No HW flow control
     */
    usart1_init_struct.USART_BaudRate = 9600;
    usart1_init_struct.USART_WordLength = USART_WordLength_8b;
    usart1_init_struct.USART_StopBits = USART_StopBits_1;
    usart1_init_struct.USART_Parity = USART_Parity_No ;
    usart1_init_struct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart1_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    /* Configure USART1 */
    USART_Init(USART1, &usart1_init_struct);
    /* Enable RXNE interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0xFF;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable USART1 global interrupt */
    NVIC_EnableIRQ(USART1_IRQn);

	/* Create the timer(s) */
	buttonReleaseTimer = xTimerCreate( 	"buttonTimer", 				/* A text name, purely to help debugging. */
							((SPACE_TICK_LENGTH * 10) / portTICK_PERIOD_MS ),/* The timer period, in this case (SPACE_TICK_LENGTH * 10) ms. */
							pdFALSE,					/* This is a one-shot timer, so xAutoReload is set to pdFALSE. */
							( void * ) 0,				/* The ID is not used, so can be set to anything. */
							TranslateMorseCode			/* The callback function that switches the LED off. */
						);

	DisplaySpaceTimer = xTimerCreate( 	"offTimer", 				/* A text name, purely to help debugging. */
							(SPACE_TICK_LENGTH / portTICK_PERIOD_MS ),/* The timer period, in this case (SPACE_TICK_LENGTH * 10) ms. */
							pdFALSE,					/* This is a one-shot timer, so xAutoReload is set to pdFALSE. */
							( void * ) 0,				/* The ID is not used, so can be set to anything. */
							DisplayOff			/* The callback function that switches the LED off. */
						);

	DisplayBeepTimer = xTimerCreate( 	"onTimer", 				/* A text name, purely to help debugging. */
							(BEEP_TICK_LENGTH / portTICK_PERIOD_MS ),/* The timer period, in this case (SPACE_TICK_LENGTH * 10) ms. */
							pdFALSE,					/* This is a one-shot timer, so xAutoReload is set to pdFALSE. */
							( void * ) 0,				/* The ID is not used, so can be set to anything. */
							DisplayOn			/* The callback function that switches the LED off. */
						);

	/* Create the queue. */
	buttonQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( struct ButtonPress ) );
	messageQueue = xQueueCreate( 10, sizeof( char* ) );
	sendMessageQueue = xQueueCreate( 10, sizeof( char* ) );
	displayQueue = xQueueCreate( 10, sizeof( int8_t ) );
	uart1Queue = xQueueCreate( 10, sizeof( char ) );

	semaphorePolling = xSemaphoreCreateBinary();
	semaphoreISR = xSemaphoreCreateBinary();
	xSemaphoreGive(semaphoreISR);

	/* create the task(s) */
	xTaskCreate( PollingTask, "ButtonPolling", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY + 4, NULL );
	xTaskCreate( RecordButtonPresses, "RecordBP", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, &RecordButtonPressesTask );
	xTaskCreate( Menu, "Menu", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY + 1, NULL );
	xTaskCreate( SendMessage, "sendMessage", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY + 3, NULL );
	xTaskCreate( UartMessage, "uartMessage", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY + 2, NULL );

//	char* test = malloc(10 * sizeof(char));
//	test[0] = 'S';
//	test[1] = 'O';
//	test[2] = 'S';
//	test[3] = '\0';
//	xQueueSend( sendMessageQueue, &test, 0 );

	/*start tasks*/
	vTaskStartScheduler();

	/* Infinite loop (should never hit) */
	while (1)
	{
	}
}

/*
 * Minimal __assert_func used by the assert() macro
 * */
void __assert_func(const char *file, int line, const char *func, const char *failedexpr)
{
  while(1)
  {}
}

/*
 * Minimal __assert() uses __assert__func()
 * */
void __assert(const char *file, int line, const char *failedexpr)
{
   __assert_func (file, line, NULL, failedexpr);
}

/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}
