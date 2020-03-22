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
#include "synth_task.h"

#include "serial_driver.h"

#include "midi_app_data.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for store midi messages */
#define MIDI_MSG_TIMEOUT                (100U)

/* Size for storage midi commands */
#define MIDI_CMD_BUF_STORAGE_SIZE       (32U)

/* Size used fo midi channels to handle */
#define MIDI_NUM_CHANNEL                (SYNTH_MAX_NUM_VOICE)

/* Value for not valid channel */
#define MIDI_VOICE_NOT_VALID            (255U)

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Midi channel control structure */
typedef struct
{
  uint8_t u8Note;
  uint8_t u8Velocity;
} MidiChannel_t;

/* Control strcuture to manage all Midi Channels */
typedef struct
{
    midiMode_t xMode;
    uint8_t u8Bank;
    uint8_t u8Program;
    uint8_t u8BaseChannel;
    MidiChannel_t pxChannelList[MIDI_NUM_CHANNEL];
    MidiChannel_t pxTmpChannelList[MIDI_NUM_CHANNEL];
    MidiChannel_t pxTmpPolyChannel;
} MidiCtrl_t;

/* Private variables ---------------------------------------------------------*/

/* Midi control structure */
MidiCtrl_t xMidiCfg = {0};

/* Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

/* Message buffer control structrure */
MessageBufferHandle_t xMidiMessageBuffer;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Reset medi control structure too default values.
  * @retval None.
  */
static void vResetMidiCtrl(void);

/**
  * @brief Load data from persistence memory.
  * @retval None.
  */
static void vRestoreMidiCtrl(void);

/**
  * @brief Send synth cmd to synth task.
  * @param pxSynthCmd pointer to synth command.
  * @retval Operation result, true OK, false, error.
  */
static bool bSendSynthCmd(SynthMsg_t * pxSynthCmd);

/**
  * @brief Handle note on.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOn(uint8_t * pu8MidiCmd);

/**
  * @brief Handle note on in mono mode.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOnMono(uint8_t * pu8MidiCmd);

/**
  * @brief Handle note on in poly mode.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOnPoly(uint8_t * pu8MidiCmd);

/**
  * @brief Handle note off.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOff(uint8_t * pu8MidiCmd);

/**
  * @brief Handle note off in mono mode.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOffMono(uint8_t * pu8MidiCmd);

/**
  * @brief Handle note off in poly mode.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOffPoly(uint8_t * pu8MidiCmd);

/**
  * @brief Handle midi cmd.
  * @param pu8MidiCmd array with midi cmd.
  * @retval None.
  */
static void vHandleMidiCmd(uint8_t * pu8MidiCmd);

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

static void vResetMidiCtrl(void)
{
    xMidiCfg.xMode = MidiMode4;      /* Mono mode */
    xMidiCfg.u8Bank = 0U;
    xMidiCfg.u8Program = 0U;
    xMidiCfg.u8BaseChannel = 1U;

    /* Tmp values */
    for (uint32_t u32Index = 0U; u32Index < MIDI_NUM_CHANNEL; u32Index++)
    {
        xMidiCfg.pxChannelList[u32Index].u8Note = MIDI_DATA_NOT_VALID;
        xMidiCfg.pxChannelList[u32Index].u8Velocity = MIDI_DATA_NOT_VALID;

        xMidiCfg.pxTmpChannelList[u32Index].u8Note = MIDI_DATA_NOT_VALID;
        xMidiCfg.pxTmpChannelList[u32Index].u8Velocity = MIDI_DATA_NOT_VALID;
    }
    xMidiCfg.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
    xMidiCfg.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;

    /* Clear voices on synth */
    SynthMsg_t xTmpCmd = {0U};
    xTmpCmd.xType = SYNTH_CMD_NOTE_OFF_ALL;
    (void)bSendSynthCmd(&xTmpCmd);
}

static void vRestoreMidiCtrl(void)
{
    const midi_app_data_t * pxFlasData = NULL;

    if (bMIDI_APP_DATA_read(&pxFlasData))
    {
        xMidiCfg.xMode = pxFlasData->u8Mode;
        xMidiCfg.u8BaseChannel = pxFlasData->u8BaseChannel;
        xMidiCfg.u8Program = pxFlasData->u8Program;

        vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Mode %02X", xMidiCfg.xMode);
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Channel %02X", xMidiCfg.u8BaseChannel);
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Program %02X", xMidiCfg.u8Program);
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Error reading flash data");
    }

}

static bool bSendSynthCmd(SynthMsg_t * pxSynthCmd)
{
    bool bRetVal = false;
    size_t xBytesSent;
    /* Send command to synth task */
    xBytesSent = xMessageBufferSend(xMidiMessageBuffer,(void *)pxSynthCmd, sizeof(SynthMsg_t), pdMS_TO_TICKS(MIDI_MSG_TIMEOUT));
    if (xBytesSent == sizeof(SynthMsg_t))
    {
        bRetVal = true;
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "CMD: Memory Error");
    }
    return bRetVal;
}

static void vMidiCmdOn(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        /* Process in MONO mode */
        if (xMidiCfg.xMode == MidiMode4)
        {
            vMidiCmdOnMono(pu8MidiCmd);
        }
        /* Process in POLY mode */
        else if (xMidiCfg.xMode == MidiMode3)
        {
            vMidiCmdOnPoly(pu8MidiCmd);
        }
    }
}

static void vMidiCmdOff(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        /* Process in MONO mode */
        if (xMidiCfg.xMode == MidiMode4)
        {
            vMidiCmdOffMono(pu8MidiCmd);
        }
        /* Process in POLY mode */
        else if (xMidiCfg.xMode == MidiMode3)
        {
            vMidiCmdOffPoly(pu8MidiCmd);
        }
    }
}

static void vMidiCmdOnMono(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8Status = pu8MidiCmd[0U];
        uint8_t u8Note = pu8MidiCmd[1U];
        uint8_t u8Velocity = pu8MidiCmd[2U];

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON)
        {
            /* Check channel value */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

            if (u8Velocity != 0U)
            {
                if ((u8Channel >= xMidiCfg.u8BaseChannel) && (u8Channel < (xMidiCfg.u8BaseChannel + MIDI_NUM_CHANNEL)))
                {
                    uint8_t u8Voice = xMidiCfg.u8BaseChannel - u8Channel;

                    /* Check if voice is in use */
                    if (xMidiCfg.pxChannelList[u8Voice].u8Note == MIDI_DATA_NOT_VALID)
                    {
                        /* Send cmd to synth task */
                        uint32_t u32Idata = 0U;
                        SynthMsg_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_ON;
                        xSynthCmd.u8Data[u32Idata++] = u8Voice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiCfg.pxChannelList[u8Voice].u8Note = u8Note;
                            xMidiCfg.pxChannelList[u8Voice].u8Velocity = u8Velocity;
                        }
                    }
                    else if (xMidiCfg.pxChannelList[u8Voice].u8Note != u8Note)
                    {
                        /* If note is not already pressed, store in temporal position */
                        xMidiCfg.pxTmpChannelList[u8Voice].u8Note = u8Note;
                        xMidiCfg.pxTmpChannelList[u8Voice].u8Velocity = u8Velocity;
                    }
                }
            }
            else
            {
                pu8MidiCmd[0U] = MIDI_STATUS_NOTE_OFF | u8Channel;
                vMidiCmdOffMono(pu8MidiCmd);
            }
        }
    }
}

static void vMidiCmdOnPoly(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8Status = pu8MidiCmd[0U];
        uint8_t u8Note = pu8MidiCmd[1U];
        uint8_t u8Velocity = pu8MidiCmd[2U];

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON)
        {
            /* Check channel value */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

            if (u8Velocity != 0U)
            {
                if (u8Channel == xMidiCfg.u8BaseChannel)
                {
                    /* Search for voice */
                    uint8_t u8Voice = MIDI_VOICE_NOT_VALID;

                    /* Check if note is already active */
                    for (uint32_t u32IndexVoice = 0U; u32IndexVoice < MIDI_NUM_CHANNEL; u32IndexVoice++)
                    {
                        if (xMidiCfg.pxChannelList[u32IndexVoice].u8Note == u8Note)
                        {
                            u8Voice = MIDI_VOICE_NOT_VALID;
                            break;
                        }
                        /* Check and save free voice index to not iterate after */
                        else if (xMidiCfg.pxChannelList[u32IndexVoice].u8Note == MIDI_DATA_NOT_VALID)
                        {
                            /* Check if voice has been used before */
                            if (u8Voice == MIDI_VOICE_NOT_VALID)
                            {
                                u8Voice = u32IndexVoice;
                            }
                        }
                    }

                    /* Same note not found and free voice found */
                    if ((u8Voice != MIDI_VOICE_NOT_VALID) && (u8Voice < MIDI_NUM_CHANNEL))
                    {
                        /* Send cmd to synth task */
                        uint32_t u32Idata = 0U;
                        SynthMsg_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_ON;
                        xSynthCmd.u8Data[u32Idata++] = u8Voice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiCfg.pxChannelList[u8Voice].u8Note = u8Note;
                            xMidiCfg.pxChannelList[u8Voice].u8Velocity = u8Velocity;
                        }
                    }
                    /* Not free voice found, save voice on temporal voice */
                    else
                    {
                        xMidiCfg.pxTmpPolyChannel.u8Note = u8Note;
                        xMidiCfg.pxTmpPolyChannel.u8Velocity = u8Velocity;
                    }
                }
            }
            else
            {
                pu8MidiCmd[0U] = MIDI_STATUS_NOTE_OFF | u8Channel;
                vMidiCmdOffPoly(pu8MidiCmd);
            }
        }
    }
}

static void vMidiCmdOffMono(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8Status = *pu8MidiCmd++;
        uint8_t u8Note = *pu8MidiCmd++;
        uint8_t u8Velocity = *pu8MidiCmd++;

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            /* Check channel value to be in valid range */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;
            if ((u8Channel >= xMidiCfg.u8BaseChannel) && (u8Channel < (xMidiCfg.u8BaseChannel + MIDI_NUM_CHANNEL)))
            {
                uint8_t u8Voice = xMidiCfg.u8BaseChannel - u8Channel;

                /* Check if note is in use */
                if (xMidiCfg.pxChannelList[u8Voice].u8Note == u8Note)
                {
                    /* Send cmd to synth task */
                    size_t xBytesSent;
                    uint32_t u32Idata = 0U;
                    SynthMsg_t xSynthCmd = {0};

                    /* Build synth NOTE ON command */
                    xSynthCmd.xType = SYNTH_CMD_NOTE_OFF;
                    xSynthCmd.u8Data[u32Idata++] = u8Voice;
                    xSynthCmd.u8Data[u32Idata++] = u8Note;

                    /* Send command to synth task */
                    if (bSendSynthCmd(&xSynthCmd))
                    {
                        /* Update control structure */
                        xMidiCfg.pxChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                        xMidiCfg.pxChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

                        /* Check tmp note */
                        if (xMidiCfg.pxTmpChannelList[u8Voice].u8Note != MIDI_DATA_NOT_VALID)
                        {
                            /* Update control structure */
                            uint8_t pu8TmpMidiCmd[] = {
                                MIDI_STATUS_NOTE_ON | u8Channel, 
                                xMidiCfg.pxTmpChannelList[u8Voice].u8Note,
                                xMidiCfg.pxTmpChannelList[u8Voice].u8Velocity};

                            xMidiCfg.pxTmpChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                            xMidiCfg.pxTmpChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

                            /* Generate new note */
                            vMidiCmdOnMono(pu8TmpMidiCmd);
                        }
                    }
                }
                else if (xMidiCfg.pxTmpChannelList[u8Voice].u8Note == u8Note)
                {
                    /* Clear tmp note */
                    xMidiCfg.pxTmpChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                    xMidiCfg.pxTmpChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;
                }
            }
        }
    }
}

static void vMidiCmdOffPoly(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8Status = *pu8MidiCmd++;
        uint8_t u8Note = *pu8MidiCmd++;
        uint8_t u8Velocity = *pu8MidiCmd;

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            /* Check channel value */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;
            if (u8Channel == xMidiCfg.u8BaseChannel)
            {
                uint8_t u8Voice = MIDI_VOICE_NOT_VALID;

                /* Check if note is already active */
                for (uint32_t u32IndexVoice = 0U; u32IndexVoice < MIDI_NUM_CHANNEL; u32IndexVoice++)
                {
                    if (xMidiCfg.pxChannelList[u32IndexVoice].u8Note == u8Note)
                    {
                        /* Clear channel */
                        uint32_t u32Idata = 0U;
                        SynthMsg_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_OFF;
                        xSynthCmd.u8Data[u32Idata++] = u32IndexVoice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiCfg.pxChannelList[u32IndexVoice].u8Note = MIDI_DATA_NOT_VALID;
                            xMidiCfg.pxChannelList[u32IndexVoice].u8Velocity = MIDI_DATA_NOT_VALID;

                            /* Save slot cleared one time */
                            if (u8Voice == MIDI_VOICE_NOT_VALID)
                            {
                                u8Voice = u32IndexVoice;
                            }
                        }
                    }
                }

                /* Check tmp voice */
                if (xMidiCfg.pxTmpPolyChannel.u8Note == u8Note)
                {
                    xMidiCfg.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
                    xMidiCfg.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;
                }

                /* If there are a free voice, load tmp note */
                if ((u8Voice != MIDI_VOICE_NOT_VALID) && (u8Voice < MIDI_NUM_CHANNEL) && (xMidiCfg.pxTmpPolyChannel.u8Note != MIDI_DATA_NOT_VALID))
                {
                    /* Update control structure */
                    uint8_t pu8TmpMidiCmd[] = {
                        MIDI_STATUS_NOTE_ON | u8Channel, 
                        xMidiCfg.pxTmpPolyChannel.u8Note,
                        xMidiCfg.pxTmpPolyChannel.u8Velocity};

                    xMidiCfg.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
                    xMidiCfg.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;

                    /* Generate new note */
                    vMidiCmdOnPoly(pu8TmpMidiCmd);
                }
            }
        }
    }
}

static void vHandleMidiCmd(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8MidiCmd = pu8MidiCmd[0U];

        if ((u8MidiCmd & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON)
        {
            vMidiCmdOn(pu8MidiCmd);
        }
        else if ((u8MidiCmd & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            vMidiCmdOff(pu8MidiCmd);
        }
    }
}

static void vMidiCmdSysExCallBack(uint8_t *pu8Data, uint32_t u32LenData)
{
    if ((pu8Data) != NULL)
    {
        size_t xBytesSent;
        SynthMsg_t xSynthCmd = {0};

        xSynthCmd.xType = SYNTH_CMD_SYSEX;

        xBytesSent = xMessageBufferSend(xMidiMessageBuffer,(void *)&xSynthCmd, sizeof(SynthMsg_t), pdMS_TO_TICKS(MIDI_MSG_TIMEOUT));

        if (xBytesSent == sizeof(SynthMsg_t))
        {
            vCliPrintf(MIDI_TASK_NAME, "SYSEX: CMD LEN %d", u32LenData);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "SYSEX: Memory Error");
        }
    }
}

static void vMidiCmd1CallBack(uint8_t u8Cmd, uint8_t u8Data)
{
    uint8_t u8MidiCmdBuffer[] = {u8Cmd, u8Data};
    vHandleMidiCmd(u8MidiCmdBuffer);
}

static void vMidiCmd2CallBack(uint8_t u8Cmd, uint8_t u8Data0, uint8_t u8Data1)
{
    uint8_t u8MidiCmdBuffer[] = {u8Cmd, u8Data0, u8Data1};
    vHandleMidiCmd(u8MidiCmdBuffer);
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
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(500U));

    /* Show init msg */
    vCliPrintf(MIDI_TASK_NAME, "Init");

    /* Init resources */
    (void)SERIAL_init(SERIAL_0, vSerialPortHandlerCallBack);

    /* Init MIDI library */
    (void)midi_init(vMidiCmdSysExCallBack, vMidiCmd1CallBack, vMidiCmd2CallBack, vMidiCmdRtCallBack);

    /* Init flash data */
    (void)bMIDI_APP_DATA_init();

    /* Init control structure */
    vResetMidiCtrl();

    /* Restore midi cfg */
    vRestoreMidiCtrl();

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

midiMode_t xMidiTaskGetMode(void)
{
    return xMidiCfg.xMode;
}

uint8_t u8MidiTaskGetChannel(void)
{
    return xMidiCfg.u8BaseChannel;
}

uint8_t u8MidiTaskGetBank(void)
{
    return xMidiCfg.u8Bank;
}

uint8_t u8MidiTaskGetProgram(void)
{
    return xMidiCfg.u8Program;
}

bool bMidiTaskSetMode(midiMode_t xNewMode)
{
    bool bRetval = false;

    if ((xNewMode != xMidiCfg.xMode) && ((xNewMode == MidiMode3) || (xNewMode == MidiMode4)))
    {
        uint8_t u8Bank = xMidiCfg.u8Bank;
        uint8_t u8Program = xMidiCfg.u8Program;
        uint8_t u8BaseChannel = xMidiCfg.u8BaseChannel;

        vResetMidiCtrl();

        xMidiCfg.xMode = xNewMode;
        xMidiCfg.u8BaseChannel = u8BaseChannel;
        xMidiCfg.u8Program = u8Program;
        xMidiCfg.u8Bank = u8Bank;

        bRetval = true;
    }

    return bRetval;
}

bool bMidiTaskSetChannel(uint8_t u8NewChannel)
{
    bool bRetval = false;

    if ((u8NewChannel <= MIDI_CHANNEL_MAX_VALUE) && (u8NewChannel != xMidiCfg.u8BaseChannel))
    {
        midiMode_t xMode = xMidiCfg.xMode;
        uint8_t u8Bank = xMidiCfg.u8Bank;
        uint8_t u8Program = xMidiCfg.u8Program;

        vResetMidiCtrl();

        xMidiCfg.xMode = xMode;
        xMidiCfg.u8BaseChannel = u8NewChannel;
        xMidiCfg.u8Program = u8Program;
        xMidiCfg.u8Bank = u8Bank;

        bRetval = true;
    }

    return bRetval;
}

bool bMidiTaskSetBank(uint8_t u8NewBank)
{
    bool bRetval = false;

    if ((u8NewBank < MIDI_APP_MAX_BANK) && (xMidiCfg.u8Bank != u8NewBank))
    {
        bRetval = bSynthLoadPreset(u8NewBank, 0U);

        if (bRetval)
        {
            midiMode_t xMode = xMidiCfg.xMode;
            uint8_t u8BaseChannel = xMidiCfg.u8BaseChannel;

            vResetMidiCtrl();

            xMidiCfg.xMode = xMode;
            xMidiCfg.u8BaseChannel = u8BaseChannel;
            xMidiCfg.u8Program = 0U;
            xMidiCfg.u8Bank = u8NewBank;

            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: OK", u8NewBank, 0U);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: ERROR", u8NewBank, 0U);
        }
    }

    return bRetval;
}

bool bMidiTaskSetProgram(uint8_t u8NewProgram)
{
    bool bRetval = false;

    if ((u8NewProgram <= MIDI_PROGRAM_MAX_VALUE) && (xMidiCfg.u8Program != u8NewProgram))
    {
        bRetval = bSynthLoadPreset(xMidiCfg.u8Bank, u8NewProgram);

        if (bRetval)
        {
            midiMode_t xMode = xMidiCfg.xMode;
            uint8_t u8BaseChannel = xMidiCfg.u8BaseChannel;
            uint8_t u8Bank = xMidiCfg.u8Bank;

            vResetMidiCtrl();

            xMidiCfg.xMode = xMode;
            xMidiCfg.u8BaseChannel = u8BaseChannel;
            xMidiCfg.u8Program = u8NewProgram;
            xMidiCfg.u8Bank = u8Bank;

            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: OK", xMidiCfg.u8Bank, u8NewProgram);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: ERROR", xMidiCfg.u8Bank, u8NewProgram);
        }
    }

    return bRetval;
}

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