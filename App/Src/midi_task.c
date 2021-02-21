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
#include "error.h"
#include "app_lfs.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for store midi messages */
#define MIDI_MSG_TIMEOUT                (100U)

/* Size used fo midi channels to handle */
#define MIDI_NUM_CHANNEL                (SYNTH_MAX_NUM_VOICE)

/* Value for not valid channel */
#define MIDI_VOICE_NOT_VALID            (255U)

/* Midi check signal */
#define MIDI_CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

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
    lfs_midi_data_t xMidiCfg;
    MidiChannel_t pxChannelList[MIDI_NUM_CHANNEL];
    MidiChannel_t pxTmpChannelList[MIDI_NUM_CHANNEL];
    MidiChannel_t pxTmpPolyChannel;
} MidiCtrl_t;

/* Private variables ---------------------------------------------------------*/

/* Midi control structure */
MidiCtrl_t xMidiHandler = { 0 };

/* Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

#ifdef MIDI_DBG_STATS
volatile uint32_t u32NoteOnCount = 0U;
volatile uint32_t u32NoteOffCount = 0U;
volatile uint32_t u32MidiCmdCount = 0U;
volatile uint32_t u32MidiByteCount = 0U;
#endif

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Clear all actual notes.
  * @retval None.
  */
static void vClearChannels(void);

/**
  * @brief Reset medi control structure too default values.
  * @retval None.
  */
static void vResetMidiCtrl(void);

/**
  * @brief Load data from persistence memory.
  * @retval Operation result.
  */
static bool bRestoreMidiCtrl(void);

/**
  * @brief Send synth cmd to synth task.
  * @param pxSynthCmd pointer to synth command.
  * @retval Operation result, true OK, false, error.
  */
static bool bSendSynthCmd(SynthEventPayloadMidi_t * pxSynthCmd);

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
  * @brief Handle MIDI CC.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdCC(uint8_t * pu8MidiCmd);

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

static void vClearChannels(void)
{
    SynthEventPayloadMidi_t xTmpCmd = { 0U };

    for (uint32_t u32Index = 0U; u32Index < MIDI_NUM_CHANNEL; u32Index++)
    {
        uint32_t u32Idata = 0U;

        xTmpCmd.xType = SYNTH_CMD_NOTE_OFF;
        xTmpCmd.u8Data[u32Idata++] = u32Index;
        xTmpCmd.u8Data[u32Idata++] = xMidiHandler.pxChannelList[u32Index].u8Note;
        (void)bSendSynthCmd(&xTmpCmd);

        xMidiHandler.pxChannelList[u32Index].u8Note = MIDI_DATA_NOT_VALID;
        xMidiHandler.pxChannelList[u32Index].u8Velocity = MIDI_DATA_NOT_VALID;

        xMidiHandler.pxTmpChannelList[u32Index].u8Note = MIDI_DATA_NOT_VALID;
        xMidiHandler.pxTmpChannelList[u32Index].u8Velocity = MIDI_DATA_NOT_VALID;
    }

    xMidiHandler.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
    xMidiHandler.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;
}

static void vResetMidiCtrl(void)
{
    xMidiHandler.xMidiCfg.u8Mode = LFS_MIDI_CFG_DEFAULT_MODE;
    xMidiHandler.xMidiCfg.u8Bank = LFS_MIDI_CFG_DEFAULT_BANK;
    xMidiHandler.xMidiCfg.u8Program = LFS_MIDI_CFG_DEFAULT_PROG;
    xMidiHandler.xMidiCfg.u8BaseChannel = LFS_MIDI_CFG_DEFAULT_CH;

    vClearChannels();
}

static bool bRestoreMidiCtrl(void)
{
    bool bRetVal = false;

    /* Init flash data */
    if ( LFS_init() == LFS_OK )
    {
        if ( LFS_read_midi_data(&xMidiHandler.xMidiCfg) == LFS_OK )
        {
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Mode %02X", xMidiHandler.xMidiCfg.u8Mode);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Channel %02X", xMidiHandler.xMidiCfg.u8BaseChannel);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Bank %02X", xMidiHandler.xMidiCfg.u8Bank);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Program %02X", xMidiHandler.xMidiCfg.u8Program);
            bRetVal = true;
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Error reading flash data");
            ERR_ASSERT(0U);
        }
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Error on init FSM");
        ERR_ASSERT(0U);
    }

    return bRetVal;
}

static bool bSendSynthCmd(SynthEventPayloadMidi_t * pxSynthCmd)
{
    bool bRetVal = false;
    QueueHandle_t xSynthQueue = pxSynthTaskGetQueue();

    if (xSynthQueue != NULL)
    {
        SynthEvent_t xMidiEvent = {0U};

        xMidiEvent.eType = SYNTH_EVENT_MIDI_MSG;
        xMidiEvent.uPayload.xMidi = *pxSynthCmd;

        if (xQueueSend( xSynthQueue, &xMidiEvent, 0U) == pdPASS)
        {
            bRetVal = true;
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD: Queue Error");
        }
    }

    return bRetVal;
}

static void vMidiCmdOn(uint8_t * pu8MidiCmd)
{
#ifdef MIDI_DBG_STATS
    u32NoteOnCount++;
#endif

    if (pu8MidiCmd != NULL)
    {
        /* Process in MONO mode */
        if (xMidiHandler.xMidiCfg.u8Mode == (uint8_t)MidiMode4)
        {
            vMidiCmdOnMono(pu8MidiCmd);
        }
        /* Process in POLY mode */
        else if (xMidiHandler.xMidiCfg.u8Mode == (uint8_t)MidiMode3)
        {
            vMidiCmdOnPoly(pu8MidiCmd);
        }
    }
}

static void vMidiCmdOff(uint8_t * pu8MidiCmd)
{
#ifdef MIDI_DBG_STATS
    u32NoteOffCount++;
#endif

    if (pu8MidiCmd != NULL)
    {
        /* Process in MONO mode */
        if (xMidiHandler.xMidiCfg.u8Mode == (uint8_t)MidiMode4)
        {
            vMidiCmdOffMono(pu8MidiCmd);
        }
        /* Process in POLY mode */
        else if (xMidiHandler.xMidiCfg.u8Mode == (uint8_t)MidiMode3)
        {
            vMidiCmdOffPoly(pu8MidiCmd);
        }
    }
}

static void vMidiCmdCC(uint8_t * pu8MidiCmd)
{
    ERR_ASSERT(pu8MidiCmd != NULL);

    uint8_t u8Status = *pu8MidiCmd++;
    uint8_t u8CmdCc = *pu8MidiCmd++;
    uint8_t u8Data = *pu8MidiCmd++;
    uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

    /* Check if midi channel inside range */
    if ((u8Channel >= xMidiHandler.xMidiCfg.u8BaseChannel) && (u8Channel < (xMidiHandler.xMidiCfg.u8BaseChannel + MIDI_NUM_CHANNEL)))
    {
        /* Process in POLY mode */
        if (xMidiHandler.xMidiCfg.u8Mode == (uint8_t)MidiMode3)
        {
            u8Channel = SYNTH_MAX_NUM_VOICE;
        }

        /* Send cmd to synth task */
        uint32_t u32Idata = 0U;
        SynthEventPayloadMidi_t xSynthCmd = {0};

        /* Build synth NOTE ON command */
        xSynthCmd.xType = SYNTH_CMD_CC_MAP;
        xSynthCmd.u8Data[u32Idata++] = u8Channel;
        xSynthCmd.u8Data[u32Idata++] = u8CmdCc;
        xSynthCmd.u8Data[u32Idata++] = u8Data;

        if (!bSendSynthCmd(&xSynthCmd))
        {
            vCliPrintf(MIDI_TASK_NAME, "Error on sending CC message");
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
                if ((u8Channel >= xMidiHandler.xMidiCfg.u8BaseChannel) && (u8Channel < (xMidiHandler.xMidiCfg.u8BaseChannel + MIDI_NUM_CHANNEL)))
                {
                    uint8_t u8Voice = xMidiHandler.xMidiCfg.u8BaseChannel - u8Channel;

                    /* Check if voice is in use */
                    if (xMidiHandler.pxChannelList[u8Voice].u8Note == MIDI_DATA_NOT_VALID)
                    {
                        /* Send cmd to synth task */
                        uint32_t u32Idata = 0U;
                        SynthEventPayloadMidi_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_ON;
                        xSynthCmd.u8Data[u32Idata++] = u8Voice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiHandler.pxChannelList[u8Voice].u8Note = u8Note;
                            xMidiHandler.pxChannelList[u8Voice].u8Velocity = u8Velocity;
                        }
                    }
                    else if (xMidiHandler.pxChannelList[u8Voice].u8Note != u8Note)
                    {
                        /* If note is not already pressed, store in temporal position */
                        xMidiHandler.pxTmpChannelList[u8Voice].u8Note = u8Note;
                        xMidiHandler.pxTmpChannelList[u8Voice].u8Velocity = u8Velocity;
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
                if (u8Channel == xMidiHandler.xMidiCfg.u8BaseChannel)
                {
                    /* Search for voice */
                    uint8_t u8Voice = MIDI_VOICE_NOT_VALID;

                    /* Check if note is already active */
                    for (uint32_t u32IndexVoice = 0U; u32IndexVoice < MIDI_NUM_CHANNEL; u32IndexVoice++)
                    {
                        if (xMidiHandler.pxChannelList[u32IndexVoice].u8Note == u8Note)
                        {
                            u8Voice = MIDI_VOICE_NOT_VALID;
                            break;
                        }
                        /* Check and save free voice index to not iterate after */
                        else if (xMidiHandler.pxChannelList[u32IndexVoice].u8Note == MIDI_DATA_NOT_VALID)
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
                        SynthEventPayloadMidi_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_ON;
                        xSynthCmd.u8Data[u32Idata++] = u8Voice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiHandler.pxChannelList[u8Voice].u8Note = u8Note;
                            xMidiHandler.pxChannelList[u8Voice].u8Velocity = u8Velocity;
                        }
                    }
                    /* Not free voice found, save voice on temporal voice */
                    else
                    {
                        xMidiHandler.pxTmpPolyChannel.u8Note = u8Note;
                        xMidiHandler.pxTmpPolyChannel.u8Velocity = u8Velocity;
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

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            /* Check channel value to be in valid range */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;
            if ((u8Channel >= xMidiHandler.xMidiCfg.u8BaseChannel) && (u8Channel < (xMidiHandler.xMidiCfg.u8BaseChannel + MIDI_NUM_CHANNEL)))
            {
                uint8_t u8Voice = xMidiHandler.xMidiCfg.u8BaseChannel - u8Channel;

                /* Check if note is in use */
                if (xMidiHandler.pxChannelList[u8Voice].u8Note == u8Note)
                {
                    /* Send cmd to synth task */
                    uint32_t u32Idata = 0U;
                    SynthEventPayloadMidi_t xSynthCmd = {0};

                    /* Build synth NOTE ON command */
                    xSynthCmd.xType = SYNTH_CMD_NOTE_OFF;
                    xSynthCmd.u8Data[u32Idata++] = u8Voice;
                    xSynthCmd.u8Data[u32Idata++] = u8Note;

                    /* Send command to synth task */
                    if (bSendSynthCmd(&xSynthCmd))
                    {
                        /* Update control structure */
                        xMidiHandler.pxChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                        xMidiHandler.pxChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

                        /* Check tmp note */
                        if (xMidiHandler.pxTmpChannelList[u8Voice].u8Note != MIDI_DATA_NOT_VALID)
                        {
                            /* Update control structure */
                            uint8_t pu8TmpMidiCmd[] = {
                                MIDI_STATUS_NOTE_ON | u8Channel, 
                                xMidiHandler.pxTmpChannelList[u8Voice].u8Note,
                                xMidiHandler.pxTmpChannelList[u8Voice].u8Velocity};

                            xMidiHandler.pxTmpChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                            xMidiHandler.pxTmpChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

                            /* Generate new note */
                            vMidiCmdOnMono(pu8TmpMidiCmd);
                        }
                    }
                }
                else if (xMidiHandler.pxTmpChannelList[u8Voice].u8Note == u8Note)
                {
                    /* Clear tmp note */
                    xMidiHandler.pxTmpChannelList[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
                    xMidiHandler.pxTmpChannelList[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;
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

        /* Check CMD */
        if ((u8Status & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            /* Check channel value */
            uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;
            if (u8Channel == xMidiHandler.xMidiCfg.u8BaseChannel)
            {
                uint8_t u8Voice = MIDI_VOICE_NOT_VALID;

                /* Check if note is already active */
                for (uint32_t u32IndexVoice = 0U; u32IndexVoice < MIDI_NUM_CHANNEL; u32IndexVoice++)
                {
                    if (xMidiHandler.pxChannelList[u32IndexVoice].u8Note == u8Note)
                    {
                        /* Clear channel */
                        uint32_t u32Idata = 0U;
                        SynthEventPayloadMidi_t xSynthCmd = {0};

                        /* Build synth NOTE ON command */
                        xSynthCmd.xType = SYNTH_CMD_NOTE_OFF;
                        xSynthCmd.u8Data[u32Idata++] = u32IndexVoice;
                        xSynthCmd.u8Data[u32Idata++] = u8Note;

                        if (bSendSynthCmd(&xSynthCmd))
                        {
                            /* Update control structure */
                            xMidiHandler.pxChannelList[u32IndexVoice].u8Note = MIDI_DATA_NOT_VALID;
                            xMidiHandler.pxChannelList[u32IndexVoice].u8Velocity = MIDI_DATA_NOT_VALID;

                            /* Save slot cleared one time */
                            if (u8Voice == MIDI_VOICE_NOT_VALID)
                            {
                                u8Voice = u32IndexVoice;
                            }
                        }
                    }
                }

                /* Check tmp voice */
                if (xMidiHandler.pxTmpPolyChannel.u8Note == u8Note)
                {
                    xMidiHandler.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
                    xMidiHandler.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;
                }

                /* If there are a free voice, load tmp note */
                if ((u8Voice != MIDI_VOICE_NOT_VALID) && (u8Voice < MIDI_NUM_CHANNEL) && (xMidiHandler.pxTmpPolyChannel.u8Note != MIDI_DATA_NOT_VALID))
                {
                    /* Update control structure */
                    uint8_t pu8TmpMidiCmd[] = {
                        MIDI_STATUS_NOTE_ON | u8Channel, 
                        xMidiHandler.pxTmpPolyChannel.u8Note,
                        xMidiHandler.pxTmpPolyChannel.u8Velocity};

                    xMidiHandler.pxTmpPolyChannel.u8Note = MIDI_DATA_NOT_VALID;
                    xMidiHandler.pxTmpPolyChannel.u8Velocity = MIDI_DATA_NOT_VALID;

                    /* Generate new note */
                    vMidiCmdOnPoly(pu8TmpMidiCmd);
                }
            }
        }
    }
}

static void vHandleMidiCmd(uint8_t * pu8MidiCmd)
{
#ifdef MIDI_DBG_STATS
    u32MidiCmdCount++;
#endif

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
        else if ((u8MidiCmd & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_CC)
        {
            vMidiCmdCC(pu8MidiCmd);
        }
    }
}

static void vMidiCmdSysExCallBack(uint8_t *pu8Data, uint32_t u32LenData)
{
    if (pu8Data != NULL)
    {
        QueueHandle_t xSynthQueue = pxSynthTaskGetQueue();

        if (xSynthQueue != NULL)
        {
            SynthEvent_t xMidiSysExEvent = {0U};

            xMidiSysExEvent.eType = SYNTH_EVENT_MIDI_SYSEX_MSG;
            xMidiSysExEvent.uPayload.xMidiSysEx.u32Len = u32LenData;
            xMidiSysExEvent.uPayload.xMidiSysEx.pu8Data = pu8Data;

            if (xQueueSend(xSynthQueue, &xMidiSysExEvent, 0U) == pdPASS)
            {
#ifdef MIDI_DBG_VERBOSE
                vCliPrintf(MIDI_TASK_NAME, "SYSEX: CMD LEN %d", u32LenData);
#endif
            }
            else
            {
                vCliPrintf(MIDI_TASK_NAME, "SYSEX: Queue Error");
            }
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
#ifdef MIDI_DBG_VERBOSE
    vCliPrintf(MIDI_TASK_NAME, "RT: %02X", u8RtCmd);
#endif
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
    vTaskDelay(pdMS_TO_TICKS(MIDI_TASK_INIT_DELAY));

    /* Show init msg */
    vCliPrintf(MIDI_TASK_NAME, "Init");

    /* Init resources */
    (void)SERIAL_init(SERIAL_0, vSerialPortHandlerCallBack);

    /* Init MIDI library */
    (void)midi_init(vMidiCmdSysExCallBack, vMidiCmd1CallBack, vMidiCmd2CallBack, vMidiCmdRtCallBack);

    /* Init control structure */
    vResetMidiCtrl();

    /* Init flash data */
    (void)bRestoreMidiCtrl();

    for (;;)
    {
        uint32_t u32TmpEvent;
        BaseType_t xEventWait = xTaskNotifyWait(0, MIDI_SIGNAL_ALL, &u32TmpEvent, portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            if (MIDI_CHECK_SIGNAL(u32TmpEvent, MIDI_SIGNAL_RX_DATA))
            {
                /* Process all buffered bytes */
                uint8_t u8RxData = 0;

                while (SERIAL_read(SERIAL_0, &u8RxData, 1) != 0)
                {
#ifdef MIDI_DBG_STATS
                    if (u8RxData != 254U)
                    {
                        u32MidiByteCount++;
                    }
#endif
                    midi_update_fsm(u8RxData);
                }
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

uint8_t xMidiTaskGetNote(uint8_t u8Channel)
{
    uint8_t u8Retval = MIDI_DATA_NOT_VALID;

    if (u8Channel < MIDI_NUM_CHANNEL)
    {
        u8Retval = xMidiHandler.pxChannelList[u8Channel].u8Note;
    }

    return u8Retval;
}

midiMode_t xMidiTaskGetMode(void)
{
    midiMode_t eRetval = (midiMode_t)xMidiHandler.xMidiCfg.u8Mode;
    return eRetval;
}

uint8_t u8MidiTaskGetChannel(void)
{
    return xMidiHandler.xMidiCfg.u8BaseChannel;
}

uint8_t u8MidiTaskGetBank(void)
{
    return xMidiHandler.xMidiCfg.u8Bank;
}

uint8_t u8MidiTaskGetProgram(void)
{
    return xMidiHandler.xMidiCfg.u8Program;
}

void vMidiPanic(void)
{
    vClearChannels();
}

bool bMidiTaskSetMode(midiMode_t xNewMode)
{
    bool bRetval = false;

    if ( (xNewMode != xMidiHandler.xMidiCfg.u8Mode) && ( (xNewMode == MidiMode3) || (xNewMode == MidiMode4) ) )
    {
        uint8_t u8Bank = xMidiHandler.xMidiCfg.u8Bank;
        uint8_t u8Program = xMidiHandler.xMidiCfg.u8Program;
        uint8_t u8BaseChannel = xMidiHandler.xMidiCfg.u8BaseChannel;

        vResetMidiCtrl();

        xMidiHandler.xMidiCfg.u8Mode = xNewMode;
        xMidiHandler.xMidiCfg.u8BaseChannel = u8BaseChannel;
        xMidiHandler.xMidiCfg.u8Program = u8Program;
        xMidiHandler.xMidiCfg.u8Bank = u8Bank;

        bRetval = true;
    }

    return bRetval;
}

bool bMidiTaskSetChannel(uint8_t u8NewChannel)
{
    bool bRetval = false;

    if ((u8NewChannel <= MIDI_CHANNEL_MAX_VALUE) && (u8NewChannel != xMidiHandler.xMidiCfg.u8BaseChannel))
    {
        midiMode_t xMode = xMidiHandler.xMidiCfg.u8Mode;
        uint8_t u8Bank = xMidiHandler.xMidiCfg.u8Bank;
        uint8_t u8Program = xMidiHandler.xMidiCfg.u8Program;

        vResetMidiCtrl();

        xMidiHandler.xMidiCfg.u8Mode = xMode;
        xMidiHandler.xMidiCfg.u8BaseChannel = u8NewChannel;
        xMidiHandler.xMidiCfg.u8Program = u8Program;
        xMidiHandler.xMidiCfg.u8Bank = u8Bank;

        bRetval = true;
    }

    return bRetval;
}

bool bMidiTaskSetBank(uint8_t u8NewBank)
{
    bool bRetval = false;

    if ((u8NewBank < MIDI_APP_MAX_BANK) && (xMidiHandler.xMidiCfg.u8Bank != u8NewBank))
    {
        bRetval = bSynthLoadPreset(u8NewBank, 0U);

        if (bRetval)
        {
            midiMode_t xMode = xMidiHandler.xMidiCfg.u8Mode;
            uint8_t u8BaseChannel = xMidiHandler.xMidiCfg.u8BaseChannel;

            vResetMidiCtrl();

            xMidiHandler.xMidiCfg.u8Mode = xMode;
            xMidiHandler.xMidiCfg.u8BaseChannel = u8BaseChannel;
            xMidiHandler.xMidiCfg.u8Program = 0U;
            xMidiHandler.xMidiCfg.u8Bank = u8NewBank;

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

    if ((u8NewProgram <= MIDI_PROGRAM_MAX_VALUE) && (xMidiHandler.xMidiCfg.u8Program != u8NewProgram))
    {
        bRetval = bSynthLoadPreset(xMidiHandler.xMidiCfg.u8Bank, u8NewProgram);

        if (bRetval)
        {
            midiMode_t xMode = xMidiHandler.xMidiCfg.u8Mode;
            uint8_t u8BaseChannel = xMidiHandler.xMidiCfg.u8BaseChannel;
            uint8_t u8Bank = xMidiHandler.xMidiCfg.u8Bank;

            vResetMidiCtrl();

            xMidiHandler.xMidiCfg.u8Mode = xMode;
            xMidiHandler.xMidiCfg.u8BaseChannel = u8BaseChannel;
            xMidiHandler.xMidiCfg.u8Program = u8NewProgram;
            xMidiHandler.xMidiCfg.u8Bank = u8Bank;

            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: OK", xMidiHandler.xMidiCfg.u8Bank, u8NewProgram);
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "LOAD BANK, %d PROGRAM, %d: ERROR", xMidiHandler.xMidiCfg.u8Bank, u8NewProgram);
        }
    }

    return bRetval;
}

bool bMidiTaskSaveCfg(void)
{
    bool bRetval = false;

    if ( LFS_write_midi_data(&xMidiHandler.xMidiCfg) == LFS_OK)
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Save Midi CFG OK");
        ERR_ASSERT(0U);
        bRetval = true;
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Save Midi CFG ERROR");
        ERR_ASSERT(0U);
    }

    return bRetval;
}

bool bMidiTaskInit(void)
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

/*****END OF FILE****/