/**
  ******************************************************************************
  * @file           : ui_task.c
  * @brief          : Task to handle ui interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ui_task.h"
#include "cli_task.h"

#include "encoder_driver.h"
#include "display_driver.h"

#include "ui_sys.h"
#include "ui_menu_main.h"

#include "main.h"
#include "printf.h"

#include "user_error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout definition */
#define UI_DISPLAY_UPDATE_RATE_MS           ( pdMS_TO_TICKS(40U) )
#define UI_DISPLAY_IDLE_MS                  ( pdMS_TO_TICKS(5000U) )
#define UI_DISPLAY_CC_RESTORE_MS            ( pdMS_TO_TICKS(1000U) )
#define UI_DISPLAY_INIT_DELAY_MS            ( pdMS_TO_TICKS(500U) )
#define UI_DISPLAY_FIRST_MSG_DELAY_MS       ( pdMS_TO_TICKS(2000U) )

/* Private macro -------------------------------------------------------------*/

/* Display initial msg  */
#define DISPLAY_INIT_MSG                "MEGADRIVER"

/* Event queue size */
#define UI_CMD_QUEUE_SIZE               ( 5U )

/* Event queue item size */
#define UI_CMD_QUEUE_ELEMENT_SIZE       ( sizeof(UiCmd_t) )

/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t ui_task_handle = NULL;

/* Timer to handle screen refresh */
TimerHandle_t xUpdateTimer = NULL;

/* Timer to handle idle event */
TimerHandle_t xIdleTimer = NULL;

/* Timer to handle CC idle message */
TimerHandle_t xCcCmdTimer = NULL;

/* Queue event handler */
QueueHandle_t xUiCmdQueueHandle = NULL;

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
  * @brief Timer callback to generate cc restore event for clearing CC windows from UI.
  * @param xTimer handler that generates the event.
  * @retval None.
  */
static void vScreenCcCmdTimeoutCallback(TimerHandle_t xTimer);

/**
 * @brief Show init message
 * 
 */
static void vScreenInitMsg( u8g2_t * pxDisplayHandler);

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

static void vScreenCcCmdTimeoutCallback(TimerHandle_t xTimer)
{
    xTaskNotify(ui_task_handle, UI_SIGNAL_RESTORE_CC, eSetBits);
}

static void vScreenInitMsg( u8g2_t * pxDisplayHandler)
{
    // Init message
    char pcInitMsg0[UI_STR_MAX_LEN] = {0};
    char pcInitMsg1[UI_STR_MAX_LEN] = {0};
    char pcInitMsg2[UI_STR_MAX_LEN] = {0};

    (void)snprintf(pcInitMsg0, UI_STR_MAX_LEN, "%s", DISPLAY_INIT_MSG);
    (void)snprintf(pcInitMsg1, UI_STR_MAX_LEN, "Build   %s", MAIN_APP_VERSION);
    (void)snprintf(pcInitMsg2, UI_STR_MAX_LEN, "Rev     %x", GIT_REVISION);

    /* Update display data */
    u8g2_ClearBuffer(pxDisplayHandler);

    u8g2_SetFont(pxDisplayHandler, u8g2_font_amstrad_cpc_extended_8r);
    u8g2_DrawStr(pxDisplayHandler, 0, 10, pcInitMsg0);
    u8g2_DrawStr(pxDisplayHandler, 0, 25, pcInitMsg1);
    u8g2_DrawStr(pxDisplayHandler, 0, 40, pcInitMsg2);

    u8g2_SendBuffer(pxDisplayHandler);
}

static void __ui_main( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(UI_DISPLAY_INIT_DELAY_MS);

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

    /* Init msg on ui */
    vScreenInitMsg(&xDisplayHandler);

    /* Update display for first time */
    vTaskDelay(UI_DISPLAY_FIRST_MSG_DELAY_MS);

    /* Init timers */
    xTimerStart(xUpdateTimer, 0U);
    xTimerStart(xIdleTimer, 0U);

    for(;;)
    {
        uint32_t u32TmpEvent = 0U;

        BaseType_t xEventWait = xTaskNotifyWait(0U, UI_SIGNAL_ALL, &u32TmpEvent, portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            /* --- Handle UI commands --- */

            if ( RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_CMD) )
            {
                UiCmd_t xUiCmd;

                while ( xQueueReceive(xUiCmdQueueHandle, &xUiCmd, 0U) == pdPASS )
                {
                    switch (xUiCmd.eCmd)
                    {
                        case UI_CMD_CC_UPDATE:
                            /* Notify change of CC var in system */
                            vUI_screen_idle_set_cc_data(xUiCmd.uPayload.xCcUpdate.u8CcId, xUiCmd.uPayload.xCcUpdate.u8Data);
                            bUiTaskNotify(UI_SIGNAL_MIDI_CC);
                            break;

                        default:
                            vCliPrintf(UI_TASK_NAME, "Not defined UI cmd: x%02X", xUiCmd.eCmd);
                            break;
                    }
                }
            }

            /* --- Events to feed to ui screens --- */

            if (u32TmpEvent & ( UI_SIGNAL_ENC_UPDATE_CW | 
                                UI_SIGNAL_ENC_UPDATE_CCW | 
                                UI_SIGNAL_ENC_UPDATE_SW_SET | 
                                UI_SIGNAL_MIDI_CC |
                                UI_SIGNAL_RESTORE_CC))
            {
                UI_action(&xUiMenuHandler, &u32TmpEvent);
            }

            /* Handle timer events */

            /* Setup timer for restoring idle */
            if ( RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_ENC_UPDATE_CW) || RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_ENC_UPDATE_CCW) )
            {
                xTimerStart(xIdleTimer, 0U);
            }

            /* If CC event, start timer for restoring idle main screen */
            if (RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_MIDI_CC))
            {
                /* If active screen is idle, kick cc restore timer */
                if (xUiMenuHandler.u32ScreenSelectionIndex == MENU_IDLE_SCREEN_POSITION)
                {
                    xTimerStart(xCcCmdTimer, 0U);
                }
            }

            /* If idle event, set IDLE screen */
            if (RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_IDLE))
            {
                vCliPrintf(UI_TASK_NAME, "IDLE event");

                if (xUiMenuHandler.u32ScreenSelectionIndex != MENU_IDLE_SCREEN_POSITION)
                {
                    u32ReturnScreen = xUiMenuHandler.u32ScreenSelectionIndex;
                }
                else
                {
                    u32ReturnScreen = MENU_MAIN_SCREEN_POSITION;
                }

                xUiMenuHandler.u32ScreenSelectionIndex = MENU_IDLE_SCREEN_POSITION;
            }

            /* Restore menu screen in case of restore from idle screen */
            if (RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_RESTORE_IDLE))
            {
                if (u32ReturnScreen < MENU_LAST_SCREEN_POSITION)
                {
                    vCliPrintf(UI_TASK_NAME, "Restore from IDLE");
                    xUiMenuHandler.u32ScreenSelectionIndex = u32ReturnScreen;
                }
            }

            /* Periodic render signal */
            if (RTOS_CHECK_SIGNAL(u32TmpEvent, UI_SIGNAL_SCREEN_UPDATE))
            {
                UI_render(&xDisplayHandler, &xUiMenuHandler);
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

void vUiTaskInit(void)
{
    /* Create task */
    xTaskCreate(__ui_main, UI_TASK_NAME, UI_TASK_STACK, NULL, UI_TASK_PRIO, &ui_task_handle);
    ERR_ASSERT(ui_task_handle);

    /* Create timer resources */
    xUpdateTimer = xTimerCreate("ScreenUpdateTimer",
                                UI_DISPLAY_UPDATE_RATE_MS,
                                pdTRUE,
                                (void *)0U,
                                vScreenUpdateCallback);
    ERR_ASSERT(xUpdateTimer);

    xIdleTimer = xTimerCreate("ScreenIdleTimer", 
                                UI_DISPLAY_IDLE_MS,
                                pdFALSE,
                                (void *)0U,
                                vScreenIdleCallback);
    ERR_ASSERT(xIdleTimer);

    xCcCmdTimer = xTimerCreate("ScreenCcRestoreTimer", 
                                UI_DISPLAY_CC_RESTORE_MS,
                                pdFALSE,
                                (void *)0U,
                                vScreenCcCmdTimeoutCallback);
    ERR_ASSERT(xCcCmdTimer);

    /* Create queue */
    xUiCmdQueueHandle = xQueueCreate(UI_CMD_QUEUE_SIZE, UI_CMD_QUEUE_ELEMENT_SIZE);
    ERR_ASSERT(xUiCmdQueueHandle);
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

bool bUiSendCmd(UiCmd_t xUiTaskCmd)
{
    bool bRetval = false;

    if ( (xUiCmdQueueHandle != NULL) && (ui_task_handle != NULL) )
    {
        if ( xQueueSend(xUiCmdQueueHandle, &xUiTaskCmd, 0U) == pdPASS )
        {
            xTaskNotify(ui_task_handle, UI_SIGNAL_CMD, eSetBits);
            bRetval = true;
        }
        else
        {
            vCliPrintf(UI_TASK_NAME, "CMD: Queue Error");
        }
    }

    return bRetval;
}

/* EOF */
