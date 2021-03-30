/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* System Resources */
#include "sys_mcu.h"
#include "sys_rtos.h"

/* Tasks */
#include "cli_task.h"
#include "synth_task.h"
#include "ui_task.h"
#include "midi_task.h"
#include "mapping_task.h"

/* Main app ------------------------------------------------------------------*/

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/
    
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    SYS_Init();

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

/* EOF */
