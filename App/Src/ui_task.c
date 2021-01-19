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
#include "timers.h"

#include "encoder_driver.h"
#include "display_driver.h"

#include "ui_sys.h"
#include "ui_menu_main.h"

#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** Update rate in ms */
#define UI_DISPLAY_UPDATE_RATE_MS   (25U)

/** Idle period definition in ms */
#define UI_DISPLAY_IDLE_MS          (10000U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t ui_task_handle = NULL;

/* Timer to handle screen refresh */
TimerHandle_t xUpdateTimer = NULL;

/* Timer to handle idle event */
TimerHandle_t xIdleTimer = NULL;

/* Pointer to display lib handler */
static u8g2_t xDisplayHandler = {0};

/* UI menu structure */
static ui_menu_t xUiMenuHandler = {0};

/* Return screen from idle */
static uint32_t u32ReturnScreen = MENU_LAST_SCREEN_POSITION;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief callback for encoder actions
  * @param event type of event generated
  * @param eventData data associated with event
  * @retval None
  */
static void encoder_cb(encoder_event_t event, uint32_t eventData);

/**
  * @brief Timer callback to generate update event.
  * @param xTimer handler that generates the event.
  * @retval None.
  */
static void vScreenUpdateCallback(TimerHandle_t xTimer);

/**
  * @brief Timer callback to generate idle event.
  * @param xTimer handler that generates the event.
  * @retval None.
  */
static void vScreenIdleCallback(TimerHandle_t xTimer);

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

static void vScreenUpdateCallback(TimerHandle_t xTimer)
{
    xTaskNotify(ui_task_handle, UI_SIGNAL_SCREEN_UPDATE, eSetBits);
}

static void vScreenIdleCallback(TimerHandle_t xTimer)
{
    xTaskNotify(ui_task_handle, UI_SIGNAL_IDLE, eSetBits);
}

static void __ui_main( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(UI_TASK_INIT_DELAY));
    
    /* Init encoder */
    ENCODER_init(ENCODER_ID_0, encoder_cb);

    /* Init display */
    if (DISPLAY_init(DISPLAY_0, &xDisplayHandler) != DISPLAY_STATUS_OK)
    {
        vCliPrintf(UI_TASK_NAME, "Display init ERROR");
        ERR_ASSERT(0U);
    }

    /* Init ui menu engine */
    if (UI_init(&xUiMenuHandler) != UI_STATUS_OK)
    {
        vCliPrintf(UI_TASK_NAME, "UI engine init ERROR");
        ERR_ASSERT(0U);
    }

    /* Show init msg */
    vCliPrintf(UI_TASK_NAME, "Init");

    /* Update display for first time */
    DISPLAY_update(DISPLAY_0, &xDisplayHandler);
    vTaskDelay(pdMS_TO_TICKS(2000U));

    /* Init timers */
    xTimerStart(xUpdateTimer, 0U);
    xTimerStart(xIdleTimer, 0U);

    for(;;)
    {
        uint32_t u32TmpEvent = 0U;

        BaseType_t xEventWait = xTaskNotifyWait(0, 
                                (
                                    UI_SIGNAL_ENC_UPDATE_CW | 
                                    UI_SIGNAL_ENC_UPDATE_CCW | 
                                    UI_SIGNAL_ENC_UPDATE_SW_RESET | 
                                    UI_SIGNAL_ENC_UPDATE_SW_SET | 
                                    UI_SIGNAL_IDLE |
                                    UI_SIGNAL_RESTORE_IDLE |
                                    UI_SIGNAL_SCREEN_UPDATE
                                ), 
                                &u32TmpEvent, 
                                portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            if (CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_SCREEN_UPDATE))
            {
                UI_render(&xDisplayHandler, &xUiMenuHandler);
            }

            if (CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_RESTORE_IDLE))
            {
                if (u32ReturnScreen < MENU_LAST_SCREEN_POSITION)
                {
                    vCliPrintf(UI_TASK_NAME, "Restore from IDLE");
                    xUiMenuHandler.u32ScreenSelectionIndex = u32ReturnScreen;
                }
            }

            if (u32TmpEvent & ( UI_SIGNAL_ENC_UPDATE_CW | 
                                UI_SIGNAL_ENC_UPDATE_CCW | 
                                UI_SIGNAL_ENC_UPDATE_SW_RESET | 
                                UI_SIGNAL_ENC_UPDATE_SW_SET | 
                                UI_SIGNAL_IDLE))
            {
                if (!CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_IDLE))
                {
                    xTimerStart(xIdleTimer, 0U);
                }
                else
                {
                    vCliPrintf(UI_TASK_NAME, "IDLE event");
                    u32ReturnScreen = xUiMenuHandler.u32ScreenSelectionIndex;
                    xUiMenuHandler.u32ScreenSelectionIndex = MENU_IDLE_SCREEN_POSITION;
                }

                UI_action(&xUiMenuHandler, &u32TmpEvent);
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bUiTaskInit(void)
{
    bool retval = false;

    /* Create task */
    xTaskCreate(__ui_main, UI_TASK_NAME, UI_TASK_STACK, NULL, UI_TASK_PRIO, &ui_task_handle);

    /* Create timer resources */
    xUpdateTimer = xTimerCreate("ScreenUpdateTimer", 
                                pdMS_TO_TICKS(UI_DISPLAY_UPDATE_RATE_MS), 
                                pdTRUE, 
                                (void *)0U, 
                                vScreenUpdateCallback);

    xIdleTimer = xTimerCreate("ScreenIdleTimer", 
                                pdMS_TO_TICKS(UI_DISPLAY_IDLE_MS), 
                                pdFALSE, 
                                (void *)0U, 
                                vScreenIdleCallback);

    /* Check resources */
    if ((ui_task_handle != NULL) && (xUpdateTimer != NULL) && (xIdleTimer != NULL))
    {
        retval = true;
    }

    return(retval);
}

bool bUiTaskNotify(uint32_t u32Event)
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
