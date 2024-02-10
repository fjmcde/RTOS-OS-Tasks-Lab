/*
 * app.h
 *
 *  Created on: Feb 07, 2024
 *      Author: Frank McDermott
 */

#ifndef INC_APP_H
#define INC_APP_H

/************************************
 * 		Header Includes		*
 ************************************/
/* C-STD Headers */
/* Core System Headers */
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
/* User Headers */
#include "Gyro_Driver.h"

/************************************
 * 		Macro Definitions			*
 ************************************/
/* Compile Switches */
//#define LAB1_USE_POLLING
//#define LAB2_USE_INTERRUPT
//#define LAB3_USE_OS_TIMER
#define LAB3_USE_TASK
/* GPIO Definitions */
#define BUTTON_PIN		GPIO_PIN_0
#define BUTTON_PORT		GPIOA
#define BUTTON_IRQn		EXTI0_IRQn
#define RED_LED_PIN		GPIO_PIN_14
#define RED_LED_PORT	GPIOG
#define GREEN_LED_PIN	GPIO_PIN_13
#define GREEN_LED_PORT	GPIOG
/* Timer Definitions */
#ifdef LAB3_USE_OS
#define TIMER_PERIOD	(uint32_t)100		// 100ms period for application timer
#endif
/* Task DEfinitions */
#ifdef LAB3_USE_TASK
#define TASK_STACK_SIZE	(uint16_t)100		// 100 * 4 bytes = 400 byte task stack
#define TASK_DELAY		(uint32_t)100		// 100ms delay
#endif

/************************************
 * 			Enumerations			*
************************************/
typedef enum
{
	counterClockwiseFast 	= -15000,	/* Faster counter-clockwise (-) rotation */
	counterClockwiseSlow 	= -2000,	/* Slow but affirmative counter-clockwise (-) rotation*/
	nearlyZero 				= 0,		/* Nearly zero clockwise (+) rotation */
	clockwiseSlow 			= 2000,		/* Slow but affirmative clockwise (+) rotation */
	clockwiseFast 			= 15000,	/* Faster clockwise (+) rotation */
}gyroRotationRate;


/************************************
 * 		Function Prototypes			*
 ************************************/
void appInit(void);

#ifdef LAB3_USE_OS
void appTimerInit(void);
void validateAppTimerState(void);
#endif

#ifdef LAB3_USE_TASK
void validateAppTaskState(void);
#endif

void sampleUserButton(void);
void driveLEDs(void);


#endif /* INC_APP_H */
