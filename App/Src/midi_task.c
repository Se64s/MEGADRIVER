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

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Midi control structure */
typedef struct {
    midiMode_t xMode;
    uint8_t u8Channel;
    uint8_t u8Program;
    uint8_t u8Bank;
} MidiCtrl_t;

/* Private define ------------------------------------------------------------*/

/* Timeout for store midi messages */
#define MIDI_MSG_TIMEOUT                (100U)

/* Size for storage midi commands */
#define MIDI_CMD_BUF_STORAGE_SIZE       (32U)

/* Signal definition */
#define MIDI_SIGNAL_RX_DATA             (1UL << 0)
#define MIDI_SIGNAL_ERROR               (1UL << 2)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Midi control structure */
MidiCtrl_t xMidiCfg = {0};

/* Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

/* Message buffer control structrure */
MessageBufferHandle_t xMidiMessageBuffer;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Decide if the following message should be processed.
  * @param u8MidiStatus Midi status cmd.
  * @param pxMidiCfg pointer to midi control structure.
  * @retval True if cmd should be preocess, false if not.
  */
static bool bProcessMidiCmd(uint8_t u8MidiStatus, MidiCtrl_t * pxMidiCfg);

/**
  * @brief Callback for midi commands detection.
  * @param pu8Data pointer of SysEx cmd.
  * @param u32LenData len of SysEx cmd.
  * @retval None.
  */
static void vMidiCmdSysExCallBack(uint8_t *pu8Data, uint32_t u32LenData);

/**
  * @brief Callback for midi commands detection.
  * @param u8Cmd Midi CMD byte.
  * @param u8Data0 Midi DATA byte.
  * @retval None.
  */
static void vMidiCmd1CallBack(uint8_t u8Cmd, uint8_t u8Data);

/**
  * @brief Callback for midi commands detection.
  * @param u8Cmd Midi CMD byte.
  * @param u8Data0 Midi DATA0 byte.
  * @param u8Data1 Midi DATA1 byte.
  * @retval None.
  */
static void vMidiCmd2CallBack(uint8_t u8Cmd, uint8_t u8Data0, uint8_t u8Data1);

/**
  * @brief Callback for midi commands detection.
  * @param u8RtCmd Real time cmd.
  * @retval None.
  */
static void vMidiCmdRtCallBack(uint8_t u8RtCmd);

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

static bool bProcessMidiCmd(uint8_t u8MidiStatus, MidiCtrl_t * pxMidiCfg)
{
    bool bRetVal = false;

    if (pxMidiCfg != NULL)
    {
        /* Omni mode process al channels */
        if ((pxMidiCfg->xMode == MidiMode1) || (pxMidiCfg->xMode == MidiMode2))
        {
            bRetVal = true;
        }
        /* Check channel */
        else if ((pxMidiCfg->xMode == MidiMode3) || (pxMidiCfg->xMode == MidiMode4))
        {
            if ((u8MidiStatus & MIDI_STATUS_CH_MASK) == pxMidiCfg->u8Channel)
            {
                bRetVal = true;
            }
        }
        else
        {
            /* Not handle */
        }
    }

    return bRetVal;
}

static void vMidiCmdSysExCallBack(uint8_t *pu8Data, uint32_t u32LenData)
{
    vCliPrintf(MIDI_TASK_NAME, "SysEx: ");
    if (pu8Data != NULL)
    {
        while (u32LenData-- != 0)
        {
            vCliRawPrintf("%02X ", *pu8Data++);
        }
    }
}

static void vMidiCmd1CallBack(uint8_t u8Cmd, uint8_t u8Data)
{
    if (bProcessMidiCmd(u8Cmd, &xMidiCfg))
    {
        size_t xBytesSent;
        uint8_t pu8MidiCmd[] = {u8Cmd, u8Data};

        xBytesSent = xMessageBufferSend(xMidiMessageBuffer,(void *)pu8MidiCmd, sizeof(pu8MidiCmd), pdMS_TO_TICKS(MIDI_MSG_TIMEOUT));

        if (xBytesSent == sizeof(pu8MidiCmd))
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD1: %02X-%02X", u8Cmd, u8Data);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD2: Memory Error");
        }
    }
}

static void vMidiCmd2CallBack(uint8_t u8Cmd, uint8_t u8Data0, uint8_t u8Data1)
{
    if (bProcessMidiCmd(u8Cmd, &xMidiCfg))
    {
        size_t xBytesSent;
        uint8_t pu8MidiCmd[] = {u8Cmd, u8Data0, u8Data1};

        xBytesSent = xMessageBufferSend(xMidiMessageBuffer,(void *)pu8MidiCmd, sizeof(pu8MidiCmd), pdMS_TO_TICKS(MIDI_MSG_TIMEOUT));

        if (xBytesSent == sizeof(pu8MidiCmd))
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD2: %02X-%02X-%02X", u8Cmd, u8Data0, u8Data1);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD2: Memory Error");
        }
    }
}

static void vMidiCmdRtCallBack(uint8_t u8RtCmd)
{
    vCliPrintf(MIDI_TASK_NAME, "RT: %02X", u8RtCmd);
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
    (void)midi_init(vMidiCmdSysExCallBack, vMidiCmd1CallBack, vMidiCmd2CallBack, vMidiCmdRtCallBack);

    /* Init control structure */
    xMidiCfg.xMode = MidiMode3;
    xMidiCfg.u8Channel = 1U;
    xMidiCfg.u8Program = 0U;
    xMidiCfg.u8Bank = 0U;

    /* Show init msg */
    vCliPrintf(MIDI_TASK_NAME, "Init");

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

bool bMidiTaskInit(void)
{
    bool bRetval = false;

    /* Create task */
    xTaskCreate(vMidiMain, MIDI_TASK_NAME, MIDI_TASK_STACK, NULL, MIDI_TASK_PRIO, &xMidiTaskHandle);

    /* Init resources */
    xMidiMessageBuffer = xMessageBufferCreate(MIDI_CMD_BUF_STORAGE_SIZE);

    /* Check resources */
    if ((xMidiTaskHandle != NULL) && (xMidiMessageBuffer != NULL))
    {
        bRetval = true;
    }
    return (bRetval);
}

bool bMidiTaskNotify(uint32_t u32Event)
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

MessageBufferHandle_t xMidiGetMessageBuffer(void)
{
    return xMidiMessageBuffer;
}

/*****END OF FILE****/