#include "API_delay.h"
#include "stm32f4xx_hal.h"  		/* <- HAL include */


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */

  while (1)
  {
  }
}

/**
 * @brief  Validate if the delay pointer is not NULL
 * @param  delay: pointer to the desired delay
 * @retval None
 */
void validateDelay(delay_t * delay) {
	if (delay == NULL) {
		Error_Handler();
	}
}

/**
 * @brief  Initialize the delay according to the duration
 * @param  delay: pointer to the delay
 * @param  duration: desired duration in ms
 * @retval None
 */
void delayInit(delay_t * delay, tick_t duration)
{
	validateDelay(delay);

	delay->startTime = 0;
	delay->duration = duration;
	delay->running = false;
}

/**
 * @brief  Read the current state of the delay
 * @param  delay: pointer to the delay
 * @retval True when the delay has been exceeded
 */
bool_t delayRead(delay_t * delay)
{
	validateDelay(delay);

	if (!delay->running) {
		delay->running = true;
		delay->startTime = HAL_GetTick();
		return false;
	}

	tick_t currentTime = HAL_GetTick();

	if ((currentTime - delay->startTime) >= delay->duration) {
		delay->running = false;
		return true;
	}

	return false;
}

/**
 * @brief  Modify the current duration of the delay
 * @param  delay: pointer to the delay
 * @param  duration: new duration in ms
 * @retval None
 */
void delayWrite(delay_t * delay, tick_t duration)
{
	validateDelay(delay);
	delay->duration = duration;
}


bool_t delayIsRunning(delay_t * delay)
{
	validateDelay(delay);
	return delay->running;
}
