/**
 * @file sys_rtos.c
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief Support code for RTOS.
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes -----------------------------------------------------------------*/

#include "sys_rtos.h"
#include "stm32g0xx_hal.h"
#include "user_error.h"
#include "task.h"

/* Private variables --------------------------------------------------------*/
/* Private macro -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private declarations -----------------------------------------------------*/
/* Private definitions ------------------------------------------------------*/
/* RTOS app hook ------------------------------------------------------------*/

void vApplicationTickHook(void)
{
    HAL_IncTick();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    ERR_ASSERT(0U);
}

/* Public functions ---------------------------------------------------------*/

/*EOF*/