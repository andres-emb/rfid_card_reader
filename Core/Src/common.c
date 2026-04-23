#include "common.h"
#include "main.h"


void error_handler(void)
{
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
	while(1);
}

