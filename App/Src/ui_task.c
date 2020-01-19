/**
  ******************************************************************************
  * @file           : ui_task.c
  * @brief          : Task to handle ui interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ui_task.h"
#include "cli_task.h"

#include "FreeRTOS.h"
#include "task.h"

#include "encoder_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Check signals */
#define CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

/* UI signals */
#define UI_SIGNAL_ENC_UPDATE_CCW        (1UL << 0U)
#define UI_SIGNAL_ENC_UPDATE_CW         (1UL << 1U)
#define UI_SIGNAL_ENC_UPDATE_SW_SET     (1UL << 2U)
#define UI_SIGNAL_ENC_UPDATE_SW_RESET   (1UL << 3U)
#define UI_SIGNAL_ERROR                 (1UL << 4U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t ui_task_handle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief callback for encoder actions
  * @param event type of event generated
  * @param eventData data associated with event
  * @retval None
  */
void encoder_cb(encoder_event_t event, uint32_t eventData);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void __ui_main(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

void encoder_cb(encoder_event_t event, uint32_t eventData)
{
    BaseType_t wakeTask;
    static uint32_t u32encoder0Count = 0;

    if (event == ENCODER_EVENT_UPDATE)
    {
        if (u32encoder0Count > eventData)
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_CW, eSetBits, &wakeTask);
        }
        else if (u32encoder0Count < eventData)
        {
            xTaskNotifyFromISR(ui_task_handle, UI_SIGNAL_ENC_UPDATE_CCW, eSetBits, &wakeTask);
        }
        else
        {
            /* code */
        }
        u32encoder0Count = eventData;
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

static void __ui_main( void *pvParameters )
{
    uint32_t tmp_event;

    /* Init encoder */
    ENCODER_init(ENCODER_ID_0, encoder_cb);

    /* Show init msg */
    cli_printf(UI_TASK_NAME, "Init");

    for(;;)
    {
        BaseType_t event_wait = xTaskNotifyWait(0, 
                                (UI_SIGNAL_ENC_UPDATE_CW | UI_SIGNAL_ENC_UPDATE_CCW | UI_SIGNAL_ENC_UPDATE_SW_RESET | UI_SIGNAL_ENC_UPDATE_SW_SET), 
                                &tmp_event, 
                                portMAX_DELAY);

        if (event_wait == pdPASS)
        {
            if (CHECK_SIGNAL(tmp_event, UI_SIGNAL_ENC_UPDATE_CW))
            {
                uint32_t enc_count = 0;
                ENCODER_getCount(ENCODER_ID_0, &enc_count);
                cli_printf(UI_TASK_NAME, "Encoder CW event: %d", enc_count);
            }
            else if (CHECK_SIGNAL(tmp_event, UI_SIGNAL_ENC_UPDATE_CCW))
            {
                uint32_t enc_count = 0;
                ENCODER_getCount(ENCODER_ID_0, &enc_count);
                cli_printf(UI_TASK_NAME, "Encoder CCW event: %d", enc_count);
            }
            else if (CHECK_SIGNAL(tmp_event, UI_SIGNAL_ENC_UPDATE_SW_SET) || CHECK_SIGNAL(tmp_event, UI_SIGNAL_ENC_UPDATE_SW_RESET))
            {
                cli_printf(UI_TASK_NAME, "Encoder SW state %d", ENCODER_getSwState(ENCODER_ID_0));
            }
            else
            {
                cli_printf(UI_TASK_NAME, "Unknown event");
            }
        }
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

/*****END OF FILE****/
