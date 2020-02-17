/**
  ******************************************************************************
  * @file           : ui_task.c
  * @brief          : Task to handle ui interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ui_task.h"
#include "cli_task.h"

#include "adc_driver.h"
#include "encoder_driver.h"
#include "display_driver.h"

#include "ui_sys.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Update rate in ms */
#define UI_DISPLAY_UPDATE_RATE      (100U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t ui_task_handle = NULL;

/* Pointer to display lib handler */
static u8g2_t xDisplayHandler = {0};

/* UI menu structure */
static ui_menu_t xUiMenuHandler = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief callback for encoder actions
  * @param event type of event generated
  * @param eventData data associated with event
  * @retval None
  */
static void encoder_cb(encoder_event_t event, uint32_t eventData);

/**
  * @brief callback for adc actions
  * @param event type of event generated
  * @retval None
  */
static void adc_cb(adc_event_t xEvent);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void __ui_main(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void encoder_cb(encoder_event_t event, uint32_t eventData)
{
    BaseType_t wakeTask;

    if (event == ENCODER_EVENT_UPDATE)
    {
        if (eventData == ENCODER_0_VALUE_CW)
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_CW, eSetBits, &wakeTask);
        }
        else if (eventData == ENCODER_0_VALUE_CCW)
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_CCW, eSetBits, &wakeTask);
        }
        else
        {
            /* code */
        }
    }
    else if (event == ENCODER_EVENT_SW)
    {
        if (eventData)
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_SW_RESET, eSetBits, &wakeTask);
        }
        else
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_SW_SET, eSetBits, &wakeTask);
        }
    }
    else
    {
        /* code */
    }
}

static void adc_cb(adc_event_t xEvent)
{
    BaseType_t xWakeTask;

    if (xEvent == ADC_EVENT_UPDATE)
    {
        xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ADC_UPDATE, eSetBits, &xWakeTask);
    }
}

static void __ui_main( void *pvParameters )
{
    vTaskDelay(250 / portTICK_PERIOD_MS);
    
    /* Init encoder */
    ENCODER_init(ENCODER_ID_0, encoder_cb);

    /* Init ADC peripheral */
    ADC_init(ADC_0, NULL);

    /* Init ADC conversion */
    ADC_start(ADC_0);

    /* Init display */
    if (DISPLAY_init(DISPLAY_0, &xDisplayHandler) != DISPLAY_STATUS_OK)
    {
        cli_printf(UI_TASK_NAME, "Display init ERROR");
        while (1);
    }

    /* Init ui menu engine */
    if (UI_init(&xUiMenuHandler) != UI_STATUS_OK)
    {
        cli_printf(UI_TASK_NAME, "UI engine init ERROR");
        while (1);
    }

    /* Show init msg */
    cli_printf(UI_TASK_NAME, "Init");

    /* Update display for first time */
    DISPLAY_update(DISPLAY_0, &xDisplayHandler);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    for(;;)
    {
        uint32_t u32TmpEvent;

        BaseType_t xEventWait = xTaskNotifyWait(0, 
                                (
                                    UI_SIGNAL_ENC_UPDATE_CW | 
                                    UI_SIGNAL_ENC_UPDATE_CCW | 
                                    UI_SIGNAL_ENC_UPDATE_SW_RESET | 
                                    UI_SIGNAL_ENC_UPDATE_SW_SET | 
                                    UI_SIGNAL_SYNTH_ON | 
                                    UI_SIGNAL_SYNTH_OFF | 
                                    UI_SIGNAL_ADC_UPDATE
                                ), 
                                &u32TmpEvent, 
                                (UI_DISPLAY_UPDATE_RATE / portTICK_PERIOD_MS));

        if (xEventWait == pdPASS)
        {
            UI_action(&xUiMenuHandler, &u32TmpEvent);
        }

        UI_render(&xDisplayHandler, &xUiMenuHandler);
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool UI_task_init(void)
{
    bool retval = false;

    /* Create task */
    xTaskCreate(__ui_main, UI_TASK_NAME, UI_TASK_STACK, NULL, UI_TASK_PRIO, &ui_task_handle);

    /* Check resources */
    if (ui_task_handle != NULL)
    {
        retval = true;
    }
    return(retval);
}

bool UI_task_notify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (ui_task_handle != NULL)
    {
      xTaskNotify(ui_task_handle, u32Event, eSetBits);
      bRetval = true;
    }
    return bRetval;
}

/*****END OF FILE****/
