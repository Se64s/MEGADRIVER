/**
  ******************************************************************************
  * @file           : synth_task.c
  * @brief          : Task to handle synth chip interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "synth_task.h"
#include "cli_task.h"
#include "ui_task.h"

#include "YM2612_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

TaskHandle_t synth_task_handle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Initial YM2612 setup
  * @retval None
  */
static void _init_setup(void);

/**
  * @brief Key on command
  * @retval None
  */
static void _init_key_on(void);

/**
  * @brief Key off command
  * @retval None
  */
static void _init_key_off(void);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void _synth_main(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void _init_setup(void)
{
    cli_printf(SYNTH_TASK_NAME, "Initial register setup");

    YM2612_write_reg(0x22, 0x00, 0); // LFO off
    YM2612_write_reg(0x27, 0x00, 0); // CH3 normal
    YM2612_write_reg(0x28, 0x00, 0); //All ch off
    YM2612_write_reg(0x28, 0x01, 0);
    YM2612_write_reg(0x28, 0x02, 0);
    YM2612_write_reg(0x28, 0x03, 0);
    YM2612_write_reg(0x28, 0x04, 0);
    YM2612_write_reg(0x28, 0x05, 0);
    YM2612_write_reg(0x28, 0x06, 0);
    YM2612_write_reg(0x2B, 0x00, 0); // DAC off
    YM2612_write_reg(0x30, 0x71, 0); // DT1/MUL
    YM2612_write_reg(0x34, 0x0d, 0);
    YM2612_write_reg(0x38, 0x33, 0);
    YM2612_write_reg(0x3C, 0x01, 0);
    YM2612_write_reg(0x40, 0x23, 0); // Total level
    YM2612_write_reg(0x44, 0x2d, 0);
    YM2612_write_reg(0x48, 0x26, 0);
    YM2612_write_reg(0x4C, 0x00, 0);
    YM2612_write_reg(0x50, 0x5f, 0); // RS/AR
    YM2612_write_reg(0x54, 0x99, 0);
    YM2612_write_reg(0x58, 0x5f, 0);
    YM2612_write_reg(0x5C, 0x94, 0);
    YM2612_write_reg(0x60, 0x05, 0); // AM/D1R
    YM2612_write_reg(0x64, 0x05, 0);
    YM2612_write_reg(0x68, 0x05, 0);
    YM2612_write_reg(0x6C, 0x05, 0);
    YM2612_write_reg(0x70, 0x02, 0); // D2R
    YM2612_write_reg(0x74, 0x02, 0);
    YM2612_write_reg(0x78, 0x02, 0);
    YM2612_write_reg(0x7C, 0x02, 0);
    YM2612_write_reg(0x80, 0x11, 0); // D1L/RR
    YM2612_write_reg(0x84, 0x11, 0);
    YM2612_write_reg(0x88, 0x11, 0);
    YM2612_write_reg(0x8C, 0xA6, 0);
    YM2612_write_reg(0x90, 0x00, 0); // Propietary
    YM2612_write_reg(0x94, 0x00, 0);
    YM2612_write_reg(0x98, 0x00, 0);
    YM2612_write_reg(0x9C, 0x00, 0);
    YM2612_write_reg(0xB0, 0x32, 0); // FB algorithm
    YM2612_write_reg(0xB4, 0xC0, 0); // Enable L-R output
    YM2612_write_reg(0x28, 0x00, 0); // Key off
    YM2612_write_reg(0xA4, 0x22, 0); // Set freq
    YM2612_write_reg(0xA0, 0x69, 0);
}

static void _init_key_on(void)
{
    YM2612_write_reg(0x28, 0xF0, 0); // Key on

    /* Report to ui task */
    UI_task_notify(UI_SIGNAL_SYNTH_ON);
}

static void _init_key_off(void)
{
    YM2612_write_reg(0x28, 0x00, 0); // Key off

    /* Report to ui task */
    UI_task_notify(UI_SIGNAL_SYNTH_OFF);
}

static void _synth_main( void *pvParameters )
{
    /* Init YM2612 resources */
    (void)YM2612_init();

    /* Show init msg */
    cli_printf(SYNTH_TASK_NAME, "Init");

    /* Basic register init */
    _init_setup();

    for(;;)
    {
      cli_printf(SYNTH_TASK_NAME, "Key ON");
      _init_key_on();

      vTaskDelay(3000 / portTICK_PERIOD_MS);

      cli_printf(SYNTH_TASK_NAME, "Key OFF");
      _init_key_off();

      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool SYNTH_task_init(void)
{
    bool retval = false;

    /* Create task */
    xTaskCreate(_synth_main, SYNTH_TASK_NAME, SYNTH_TASK_STACK, NULL, SYNTH_TASK_PRIO, &synth_task_handle);

    /* Check resources */
    if (synth_task_handle != NULL)
    {
        retval = true;
    }
    return(retval);
}

bool SYNTH_task_notify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (synth_task_handle != NULL)
    {
      xTaskNotify(synth_task_handle, u32Event, eSetBits);
      bRetval = true;
    }
    return bRetval;
}

/*****END OF FILE****/