/*
 * app.c
 *
 *  Created on: Feb 07, 2024
 *      Author: Frank McDermott
 */

#include "app.h"

/************************************
 * 			Static Variables		*
 ************************************/

volatile static GPIO_PinState 		buttonState;
volatile static gyroRotationRate 	rotation;
volatile static int16_t 			rawVelocity;


#ifdef LAB3_USE_TASK
static StaticTask_t				taskCtrlBlock;
static uint32_t					taskStack[TASK_STACK_SIZE];
static osThreadId_t				taskID;
static const osThreadAttr_t		taskAttrib = {"appTask", osThreadDetached, &taskCtrlBlock, sizeof(taskCtrlBlock),
											  &taskStack, sizeof(taskStack), osPriorityNormal};
static osThreadState_t			taskStatus;
static osStatus_t 				osState;
#endif

#ifdef LAB3_USE_OS
static osTimerId_t 				timerID;
static StaticTimer_t			timerCtrlBlock;
static const osTimerAttr_t		timerAttr = {"appTimer", 0, &timerCtrlBlock, sizeof(timerCtrlBlock)};
static uint32_t 				exec1;
static osStatus_t				timerStatus;
#endif

#ifdef LAB2_USE_INTERRUPT
volatile static int8_t 				sysTicks = 0;
#endif

/************************************
 * 	Static Function Prototypes		*
 ************************************/

static gyroRotationRate getGyroRateOfRotation(void);

#ifdef LAB3_USE_TASK
static void appTaskInit(void);
static void appTask(void* arg);
static void validateOSStatus(void);
#endif
#ifdef LAB3_USE_OS
static void appTimer_cb(void* arg);
#endif

/************************************
 * 		Function Definitions		*
 ************************************/


/*
 * 	@brief 	Initialize application code
 * 	@retval None
 */
void appInit(void)
{
	#ifdef LAB3_USE_TASK
	appTaskInit();
	#endif

	#ifdef LAB3_USE_OS_TIMER
	appTimerInit();
	timerStatus = osTimerStart(timerID, TIMER_PERIOD);

	validateAppTimerState();
	#endif


}


/*
 *  @brief Sample User Button
 *  @retval none
 */
void sampleUserButton(void)
{
	// Sample current button state
	buttonState = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
}


/*
 *  @brief Get Gyro Rate of Rotation
 *  @retval gyroRate
 * */
static gyroRotationRate getGyroRateOfRotation(void)
{
	// Variable to store and return the gyro rotation rate
	gyroRotationRate gyroRate;

	// Get the gyro velocity
	rawVelocity = Gyro_Get_Velocity();

	// Set the gyro rate based upon where the raw value
	// of the gyro velocity fits within the enumerated
	// gyroRotationRate ranges:
	// 		velocity <= -15000 			= counterClockwiseFast
	//		-15000 < velocity <= -2000 	= counterClockwiseSlow
	//		-2000 < velocity < 2000		= nearlyZero (treated as clockwise)
	//		150 <= velocity < 15000		= clockwiseSlow
	//		velocity >= 15000			= clockwiseFast
	if(rawVelocity <= counterClockwiseFast)
	{
		gyroRate = counterClockwiseFast;
	}
	else if(rawVelocity <= counterClockwiseSlow)
	{
		gyroRate = counterClockwiseSlow;
	}
	else if(rawVelocity < clockwiseSlow)
	{
		gyroRate = nearlyZero;
	}
	else if(rawVelocity < clockwiseFast)
	{
		gyroRate = clockwiseSlow;
	}
	else // rawVelocity > clockwiseFast
	{
		gyroRate = clockwiseFast;
	}

	return gyroRate;
}


/*
 * @brief	Drive the User LEDs based on button and gyro inputs
 * @retval	None
 */
void driveLEDs(void)
{
	#ifdef LAB1_USE_POLLING
	// Sample the user button to update the global buttonState variable
	sampleUserButton();
	#endif

	// Sample the gyro rotation rate
	rotation = getGyroRateOfRotation();

	// Drive green LED if button is pressed or gyro is rotating counter-clockwise
	if((buttonState == GPIO_PIN_SET) || (rotation <= counterClockwiseSlow))
	{
		HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_SET);
	}
	// ... Otherwise turn off green LED
	else
	{
		HAL_GPIO_WritePin(GREEN_LED_PORT, GREEN_LED_PIN, GPIO_PIN_RESET);
	}

	// Drive red LED if button is pressed and gyro is rotating clockwise
	if((buttonState == GPIO_PIN_SET) && (rotation > counterClockwiseSlow))
	{
		HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_SET);
	}
	// ... Otherwise turn off red LED
	else
	{
		HAL_GPIO_WritePin(RED_LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
	}
}


#ifdef LAB3_USE_TASK

/*
 * @brief	Initialize the application task
 * @retval	None
 */
void appTaskInit(void)
{
	// Create a new OS thread of the app task
	taskID = osThreadNew(appTask, NULL, &taskAttrib);

	// Ensure task was created properly
	if(taskID == NULL)
	{
		while(1){}
	}
}


/*
 * @brief	Application task
 * @retval	None
 */
static void appTask(void* arg)
{
	(void) &arg;

	// Execute task function
	while(1)
	{
		// Sample the user button state
		sampleUserButton();

		// Drive LEDs
		driveLEDs();

		// Delay between samples
		osState = osDelay(TASK_DELAY);
		validateOSStatus();
	}
}


/*
 * @brief	Validates the current state of the appTask
 * @retval	None
 */
void validateAppTaskState()
{
	// Catch OS thread errors
	switch(taskStatus)
	{
		case osThreadTerminated:
		case osThreadError:
			while(1){}
			break;
		default:
			break;
	}
}


/*
 * @brief	Validates the current OS status
 * @retval	None
 */
static void validateOSStatus(void)
{
	// Catch OS thread errors
	switch(osState)
	{
		case osOK:
			break;
		default:
			while(1){}
			break;
	}
}
#endif


#ifdef LAB3_USE_OS_TIMER
/*
 *	@brief	Initialse the application timer
 *	@
 */
void appTimerInit(void)
{
	// Initialize the timer
	timerID = osTimerNew(appTimer_cb, osTimerPeriodic, &exec1, &timerAttr);

	// Ensure timer was created properly
	if(timerID == NULL)
	{
		while(1){}
	}
}


/*
 *	@brief
 * 	@retval
 */
void validateAppTimerState(void)
{
	switch(timerStatus)
	{
		case osOK:
			break;
		default:
			while(1){}
	}
}


/*
 * @brief	Timer callback function which samples the gyro
 * 			and drives the LEDs
 * @retval	none
 */
static void appTimer_cb(void* arg)
{
	// Does nothing; removes compiler warning about
	// unused function parameters
	(void) &arg;

	// Sample button state
	sampleUserButton();

	// Sample gyro state
	rawVelocity = Gyro_Get_Velocity();

	// Drive LEDs
	driveLEDs();
}
#endif


#ifdef LAB2_USE_INTERRUPT
/*
 * @brief User Button (GPIO) ISR which samples and
 * 		  sets the buttonState variable
 * @retval none
 */
void EXTI0_IRQHandler(void)
{
	// Disable interrupts
	HAL_NVIC_DisableIRQ(BUTTON_IRQn);

	// Sample the user button
	sampleUserButton();

	// clear interuppt flag
	__HAL_GPIO_EXTI_CLEAR_IT(BUTTON_PIN);

	// Re-enable interrupts
	HAL_NVIC_EnableIRQ(BUTTON_IRQn);
}


/*
 * @brief SYSTick callback function to drive LEDs every 100ms
 * @retval none
 */
void HAL_SYSTICK_Callback(void)
{
	sysTicks++;

	// Drive LEDs every 100ms
	if(sysTicks == 100)
	{
		// Drive LEDs
		driveLEDs();
		sysTicks = 0;
	}
}

#endif
