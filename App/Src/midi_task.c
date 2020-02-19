/**
  ******************************************************************************
  * @file           : midi_task.c
  * @brief          : Task to handle midi interface
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "midi_task.h"
#include "cli_task.h"
#include "ui_task.h"

#include "serial_driver.h"

#include "midi_lib.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Signal definition */
#define MIDI_SIGNAL_RX_DATA     (1UL << 0)
#define MIDI_SIGNAL_ERROR       (1UL << 2)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

/* Buffer for MIDI commands */
volatile static uint8_t pu8MidiCmdBuf[MIDI_CMD_BUF_SIZE] = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Callback for midi commands detection.
  * @param u8Cmd Midi CMD byte.
  * @param u8Data0 Midi DATA0 byte.
  * @param u8Data1 Midi DATA1 byte.
  * @retval None.
  */
static void vMidiCmdCallBack(uint8_t u8Cmd, uint8_t u8Data0, uint8_t u8Data1);

/**
  * @brief Handler for serial port events.
  * @param xEvent Serail event.
  * @retval None.
  */
static void vSerialPortHandlerCallBack(serial_event_t xEvent);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vMidiMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vMidiCmdCallBack(uint8_t u8Cmd, uint8_t u8Data0, uint8_t u8Data1)
{
    /* Fill data buffer with CMD data */
    pu8MidiCmdBuf[0U] = u8Cmd;
    pu8MidiCmdBuf[1U] = u8Data0;
    pu8MidiCmdBuf[2U] = u8Data1;

    cli_printf(MIDI_TASK_NAME, "MIDI CMD RX");

    /* Report to ui task */
    UI_task_notify(UI_SIGNAL_MIDI_DATA);
}

static void vSerialPortHandlerCallBack(serial_event_t xEvent)
{
    BaseType_t xWakeTask;

    if (xEvent == SERIAL_EVENT_RX_IDLE)
    {
        xTaskNotifyFromISR(xMidiTaskHandle, MIDI_SIGNAL_RX_DATA, eSetBits, &xWakeTask);
    }
    else if (xEvent == SERIAL_EVENT_ERROR)
    {
        xEvent = true;
    }
    else if (xEvent == SERIAL_EVENT_TX_DONE)
    {
        xEvent = true;
    }
    else
    {
        /* code */
    }
}

static void vMidiMain(void *pvParameters)
{
    /* Init resources */
    (void)SERIAL_init(SERIAL_0, vSerialPortHandlerCallBack);

    /* Init MIDI library */
    (void)midi_init(NULL, NULL, vMidiCmdCallBack, NULL);

    /* Show init msg */
    cli_printf(MIDI_TASK_NAME, "Init");

    for (;;)
    {
        uint32_t u32TmpEvent;
        BaseType_t xEventWait = xTaskNotifyWait(0, MIDI_SIGNAL_RX_DATA, &u32TmpEvent, portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            /* Process all buffered bytes */
            uint8_t u8RxData = 0;

            while (SERIAL_read(SERIAL_0, &u8RxData, 1) != 0)
            {
                midi_update_fsm(u8RxData);
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool MIDI_task_init(void)
{
    bool bRetval = false;

    /* Create task */
    xTaskCreate(vMidiMain, MIDI_TASK_NAME, MIDI_TASK_STACK, NULL, MIDI_TASK_PRIO, &xMidiTaskHandle);

    /* Check resources */
    if (xMidiTaskHandle != NULL)
    {
        bRetval = true;
    }
    return (bRetval);
}

bool MIDI_task_notify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (xMidiTaskHandle != NULL)
    {
        xTaskNotify(xMidiTaskHandle, u32Event, eSetBits);
        bRetval = true;
    }
    return bRetval;
}

uint8_t * MIDI_get_cmd_buf(void)
{
    return pu8MidiCmdBuf;
}

/*****END OF FILE****/