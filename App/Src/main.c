/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Peripheral library */
#include "stm32g0xx_hal.h"

/* FreeRTOS kernel */
#include "FreeRTOS.h"
#include "task.h"

/* Tasks */
#include "cli_task.h"
#include "synth_task.h"
#include "ui_task.h"
#include "midi_task.h"
#include "mapping_task.h"

/* Error managing */
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* RTOS Hook functions -------------------------------------------------------*/

void vApplicationTickHook(void)
{
  HAL_IncTick();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  ERR_ASSERT(0U);
}

/* Main app ------------------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  (void)HAL_Init();

  /* Register error output function */
  vErrorInit(vCliRawPrintf);

  /* Task creation */
  (void)bMidiTaskInit();
  (void)bCliTaskInit();
  (void)bSynthTaskInit();
  (void)bUiTaskInit();
  (void)bMapTaskInit();

  /* Start the scheduler so the tasks start executing. */
  vTaskStartScheduler();

  /* Infinite loop */
  for(;;);
}

/*****END OF FILE****/
