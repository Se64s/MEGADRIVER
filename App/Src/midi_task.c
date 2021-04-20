/**
  ******************************************************************************
  * @file           : midi_task.c
  * @brief          : Task to handle midi interface
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "midi_task.h"

#include "serial_driver.h"
#include "user_error.h"

#include "app_lfs.h"
#include "midi_lib.h"

#include "cli_task.h"
#include "synth_task.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for store midi messages */
#define MIDI_MSG_TIMEOUT                    ( 100U )

/* Size used fo midi channels to handle */
#define MIDI_NUM_CHANNEL                    ( SYNTH_MAX_NUM_VOICE )

/* Serial interface */
#define MIDI_SERIAL                         ( SERIAL_0 )

/* Queue item size */
#define MIDI_TASK_CMD_QUEUE_ELEMENT_SIZE    ( sizeof(MidiTaskCmd_t) )

/* Queue size */
#define MIDI_TASK_CMD_QUEUE_SIZE            ( 5U )

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** Midi control structure */
lfs_midi_data_t xMidiHandler = { 0U };

/** Task handler */
TaskHandle_t xMidiTaskHandle = NULL;

/** Queue for midi in_cmd */
QueueHandle_t xMidiTaskCmdQueueHandle = NULL;

#ifdef MIDI_DBG_STATS
volatile uint32_t u32NoteOnCount = 0U;
volatile uint32_t u32NoteOffCount = 0U;
volatile uint32_t u32MidiCmdCount = 0U;
volatile uint32_t u32MidiByteCount = 0U;
#endif

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Update mode in midi control structure.
 * @param pxCmdPayload 
 */
static void vHandleCmdSetMode(MidiCmdTaskPayloadSetMode_t * pxCmdPayload);

/**
 * @brief Update channel in midi control structure.
 * @param pxCmdPayload 
 */
static void vHandleCmdSetChannel(MidiCmdTaskPayloadSetCh_t * pxCmdPayload);

/**
 * @brief Update bank and program parameters in midi control structure.
 * @param pxCmdPayload 
 */
static void vHandleCmdSetPreset(MidiCmdTaskPayloadSetPreset_t * pxCmdPayload);

/**
 * @brief Save current cfg into flash.
 */
static void vHandleCmdSaveMidiCfg(void);

/**
  * @brief Reset medi control structure too default values.
  * @retval None.
  */
static void vResetMidiCfg(void);

/**
  * @brief Load data from persistence memory.
  * @retval Operation result.
  */
static void vRestoreMidiCfg(void);

/**
  * @brief Handle note on.
  * @param pu8MidiCmd pointer to midi command.
  * @retval None.
  */
static void vMidiCmdOn(uint8_t * pu8MidiCmd);

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

static void vResetMidiCfg(void)
{
    xMidiHandler.u8Mode = LFS_MIDI_CFG_DEFAULT_MODE;
    xMidiHandler.u8Bank = LFS_MIDI_CFG_DEFAULT_BANK;
    xMidiHandler.u8Program = LFS_MIDI_CFG_DEFAULT_PROG;
    xMidiHandler.u8BaseChannel = LFS_MIDI_CFG_DEFAULT_CH;

    /* Send mute all channels to synth task */
    SynthCmd_t xSynthCmd = { 0U };

    xSynthCmd.eCmd = SYNTH_CMD_VOICE_MUTE;

    (void)bSynthSendCmd(xSynthCmd);
}

static void vRestoreMidiCfg(void)
{
    /* Init flash data */
    if ( LFS_init() == LFS_OK )
    {
        if ( LFS_read_midi_data(&xMidiHandler) == LFS_OK )
        {
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Mode %02X", xMidiHandler.u8Mode);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Channel %02X", xMidiHandler.u8BaseChannel);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Bank %02X", xMidiHandler.u8Bank);
            vCliPrintf(MIDI_TASK_NAME, "FLASH: Load Program %02X", xMidiHandler.u8Program);

            // Load last used preset
            SynthCmd_t xSynthCmd = { 0U };

            xSynthCmd.eCmd = SYNTH_CMD_PRESET_UPDATE;

            xSynthCmd.uPayload.xPresetUpdate.u8Action = SYNTH_PRESET_ACTION_LOAD;
            xSynthCmd.uPayload.xPresetUpdate.u8Bank = xMidiHandler.u8Bank;
            xSynthCmd.uPayload.xPresetUpdate.u8Program = xMidiHandler.u8Program;

            (void)bSynthSendCmd(xSynthCmd);
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
}

static void vHandleCmdSetMode(MidiCmdTaskPayloadSetMode_t * pxCmdPayload)
{
    uint8_t u8NewMode = pxCmdPayload->u8Mode;

    if ( (u8NewMode != xMidiHandler.u8Mode) && ( (u8NewMode == MidiMode3) || (u8NewMode == MidiMode4) ) )
    {
        xMidiHandler.u8Mode = u8NewMode;

        vCliPrintf(MIDI_TASK_NAME, "Set Midi Mode: %d", xMidiHandler.u8Mode);

        SynthCmd_t xSynthCmd = { 0U };
        xSynthCmd.eCmd = SYNTH_CMD_VOICE_MUTE;
        (void)bSynthSendCmd(xSynthCmd);
    }
}

static void vHandleCmdSetChannel(MidiCmdTaskPayloadSetCh_t * pxCmdPayload)
{
    uint8_t u8NewChannel = pxCmdPayload->u8Channel;

    if ( (u8NewChannel <= ( MIDI_CHANNEL_MAX_VALUE - MIDI_NUM_CHANNEL)) && (u8NewChannel != xMidiHandler.u8BaseChannel) )
    {
        xMidiHandler.u8BaseChannel = u8NewChannel;

        vCliPrintf(MIDI_TASK_NAME, "Set Midi Channel: %d", xMidiHandler.u8BaseChannel);

        SynthCmd_t xSynthCmd = { 0U };
        xSynthCmd.eCmd = SYNTH_CMD_VOICE_MUTE;
        (void)bSynthSendCmd(xSynthCmd);
    }
}

static void vHandleCmdSetPreset(MidiCmdTaskPayloadSetPreset_t * pxCmdPayload)
{
    uint8_t u8NewBank = pxCmdPayload->u8Bank;
    uint8_t u8NewProgram = pxCmdPayload->u8Program;
    bool bLoadResult = false;

    if ( (u8NewBank != xMidiHandler.u8Bank) || (u8NewProgram != xMidiHandler.u8Program) )
    {
        switch ( u8NewBank )
        {
        case LFS_MIDI_BANK_ROM:
            if (u8NewProgram < LFS_MIDI_CFG_MAX_PROG_BANK_FIX)
            {
                xMidiHandler.u8Bank = LFS_MIDI_BANK_ROM;
                xMidiHandler.u8Program = u8NewProgram;

                bLoadResult = true;
            }
            break;

        case LFS_MIDI_BANK_FLASH:
            if (u8NewProgram < LFS_MIDI_CFG_MAX_PROG_BANK_FLASH)
            {
                xMidiHandler.u8Bank = LFS_MIDI_BANK_FLASH;
                xMidiHandler.u8Program = u8NewProgram;

                bLoadResult = true;
            }
            break;

        // TODO: SD Driver
        // case LFS_MIDI_BANK_SD:
        //     if (u8NewProgram < LFS_MIDI_CFG_MAX_PROG_BANK_SD)
        //     {
        //         xMidiHandler.u8Bank = LFS_MIDI_BANK_SD;
        //         xMidiHandler.u8Program = u8NewProgram;

        //         bLoadResult = true;
        //     }
        //     break;

        default:
            // None
            break;
        }
    }

    if ( bLoadResult )
    {
        vCliPrintf(MIDI_TASK_NAME, "LOAD BANK %d, PROGRAM %d - OK", u8NewBank, u8NewProgram);

        SynthCmd_t xSynthCmd = { 0U };

        xSynthCmd.eCmd = SYNTH_CMD_PRESET_UPDATE;
        xSynthCmd.uPayload.xPresetUpdate.u8Action = SYNTH_PRESET_ACTION_LOAD;
        xSynthCmd.uPayload.xPresetUpdate.u8Bank = xMidiHandler.u8Bank;
        xSynthCmd.uPayload.xPresetUpdate.u8Program = xMidiHandler.u8Program;

        (void)bSynthSendCmd(xSynthCmd);
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "LOAD BANK %d, PROGRAM %d - ERROR", u8NewBank, u8NewProgram);
    }
}

void vHandleCmdSaveMidiCfg(void)
{
    if ( LFS_write_midi_data(&xMidiHandler) == LFS_OK )
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Save Midi CFG OK");
    }
    else
    {
        vCliPrintf(MIDI_TASK_NAME, "FLASH: Save Midi CFG ERROR");
        ERR_ASSERT(0U);
    }
}

static void vMidiCmdOn(uint8_t * pu8MidiCmd)
{
    ERR_ASSERT(pu8MidiCmd);

#ifdef MIDI_DBG_STATS
    u32NoteOnCount++;
#endif

    uint8_t u8Status = *pu8MidiCmd++;
    uint8_t u8Note = *pu8MidiCmd++;
    uint8_t u8Velocity = *pu8MidiCmd++;
    uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

#ifdef MIDI_DBG_VERBOSE
    vCliPrintf(MIDI_TASK_NAME, "NOTE_ON : CH x%02X, NOTE x%02X, VEL x%02X", u8Channel, u8Note, u8Velocity);
#endif

    SynthCmd_t xSynthCmd = { 0U };

    // Send new cmd VociceChUpdate
    if ( xMidiHandler.u8Mode == (uint8_t)MidiMode4 )
    {
        uint8_t u8VoiceChannel = u8Channel - xMidiHandler.u8BaseChannel;

        xSynthCmd.eCmd = SYNTH_CMD_VOICE_UPDATE_MONO;

        xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceDst = u8VoiceChannel;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8Note = u8Note;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8Velocity = u8Velocity;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceState = (uint8_t)SYNTH_VOICE_STATE_ON;

        (void)bSynthSendCmd(xSynthCmd);
    }
    else if ( xMidiHandler.u8Mode == (uint8_t)MidiMode3 )
    {
        if ( xMidiHandler.u8BaseChannel == u8Channel )
        {
            xSynthCmd.eCmd = SYNTH_CMD_VOICE_UPDATE_POLY;

            xSynthCmd.uPayload.xVoiceUpdatePoly.u8Note = u8Note;
            xSynthCmd.uPayload.xVoiceUpdatePoly.u8Velocity = u8Velocity;
            xSynthCmd.uPayload.xVoiceUpdatePoly.u8VoiceState = (uint8_t)SYNTH_VOICE_STATE_ON;

            (void)bSynthSendCmd(xSynthCmd);
        }
    }
}

static void vMidiCmdOff(uint8_t * pu8MidiCmd)
{
    ERR_ASSERT(pu8MidiCmd);

#ifdef MIDI_DBG_STATS
    u32NoteOffCount++;
#endif

    uint8_t u8Status = *pu8MidiCmd++;
    uint8_t u8Note = *pu8MidiCmd++;
    uint8_t u8Velocity = *pu8MidiCmd++;
    uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

#ifdef MIDI_DBG_VERBOSE
    vCliPrintf(MIDI_TASK_NAME, "NOTE_OFF: CH x%02X, NOTE x%02X, VEL x%02X", u8Channel, u8Note, u8Velocity);
#endif

    // Send new cmd VociceChUpdate
    SynthCmd_t xSynthCmd = { 0U };

    // Send new cmd VociceChUpdate
    if ( xMidiHandler.u8Mode == (uint8_t)MidiMode4 )
    {
        uint8_t u8VoiceChannel = u8Channel - xMidiHandler.u8BaseChannel;

        xSynthCmd.eCmd = SYNTH_CMD_VOICE_UPDATE_MONO;

        xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceDst = u8VoiceChannel;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8Note = u8Note;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8Velocity = u8Velocity;
        xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceState = (uint8_t)SYNTH_VOICE_STATE_OFF;

        (void)bSynthSendCmd(xSynthCmd);
    }
    else if ( xMidiHandler.u8Mode == (uint8_t)MidiMode3 )
    {
        if ( xMidiHandler.u8BaseChannel == u8Channel )
        {
            xSynthCmd.eCmd = SYNTH_CMD_VOICE_UPDATE_POLY;

            xSynthCmd.uPayload.xVoiceUpdatePoly.u8Note = u8Note;
            xSynthCmd.uPayload.xVoiceUpdatePoly.u8Velocity = u8Velocity;
            xSynthCmd.uPayload.xVoiceUpdatePoly.u8VoiceState = (uint8_t)SYNTH_VOICE_STATE_OFF;

            (void)bSynthSendCmd(xSynthCmd);
        }
    }
}

static void vMidiCmdCC(uint8_t * pu8MidiCmd)
{
    ERR_ASSERT(pu8MidiCmd);

    uint8_t u8Status = *pu8MidiCmd++;
    uint8_t u8CmdCc = *pu8MidiCmd++;
    uint8_t u8Data = *pu8MidiCmd++;
    uint8_t u8Channel = u8Status & MIDI_STATUS_CH_MASK;

#ifdef MIDI_DBG_VERBOSE
    vCliPrintf(MIDI_TASK_NAME, "CC: CH x%02X, CC x%02X, DATA: x%02X", u8Channel, u8CmdCc, u8Data);
#endif

    if ( u8Channel == xMidiHandler.u8BaseChannel )
    {
        // Send new cmd ParamUpdate
        SynthCmd_t xSynthCmd = { 0U };

        xSynthCmd.eCmd = SYNTH_CMD_PARAM_UPDATE;

        xSynthCmd.uPayload.xParamUpdate.u8Id = u8CmdCc;
        xSynthCmd.uPayload.xParamUpdate.u8Data = u8Data;

        (void)bSynthSendCmd(xSynthCmd);
    }
}

static void vHandleMidiCmd(uint8_t * pu8MidiCmd)
{
    ERR_ASSERT(pu8MidiCmd);

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
    ERR_ASSERT(pu8Data);

#ifdef MIDI_DBG_VERBOSE
    vCliPrintf(MIDI_TASK_NAME, "SYSEX: CMD LEN %d", u32LenData);
#endif
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
    (void)SERIAL_init(MIDI_SERIAL, vSerialPortHandlerCallBack);

    /* Init MIDI library */
    (void)midi_init(vMidiCmdSysExCallBack, vMidiCmd1CallBack, vMidiCmd2CallBack, vMidiCmdRtCallBack);

    /* Reset midi control structure */
    vResetMidiCfg();

    /* Init flash data */
    vRestoreMidiCfg();

    for (;;)
    {
        uint32_t u32Event;
        BaseType_t xEventWait = xTaskNotifyWait(0U, MIDI_SIGNAL_ALL, &u32Event, portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            /* 
            * Handle Serial incomming data here.
            */
            if ( RTOS_CHECK_SIGNAL(u32Event, MIDI_SIGNAL_RX_DATA) )
            {
                /* Process all buffered bytes */
                uint8_t u8RxData = 0;

                while (SERIAL_read(MIDI_SERIAL, &u8RxData, 1) != 0)
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

            /* 
            * Handle MIDI parameter change here.
            */
            else if ( RTOS_CHECK_SIGNAL(u32Event, MIDI_SIGNAL_CMD_IN) )
            {
                MidiTaskCmd_t xMidiCmd;

                while (xQueueReceive(xMidiTaskCmdQueueHandle, &xMidiCmd, 0U) == pdPASS)
                {
                    switch (xMidiCmd.eCmd)
                    {
                        case MIDI_CMD_SET_MODE:
                            vHandleCmdSetMode(&xMidiCmd.uPayload.xSetMode);
                            break;

                        case MIDI_CMD_SET_CH:
                            vHandleCmdSetChannel(&xMidiCmd.uPayload.xSetCh);
                            break;

                        case MIDI_CMD_SET_PRESET:
                            vHandleCmdSetPreset(&xMidiCmd.uPayload.xSetPreset);
                            break;

                        case MIDI_CMD_SAVE_MIDI_CFG:
                            vHandleCmdSaveMidiCfg();
                            break;

                        default:
                            vCliPrintf(MIDI_TASK_NAME, "Not defined MidiTask cmd: x%02X", xMidiCmd.eCmd);
                            break;
                    }
                }
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

MidiParam_t xMidiGetParam(MidiParamType_t ePatamId)
{
    MidiParam_t xRetval = { 0U };

    switch (ePatamId)
    {
    case MIDI_PARAM_MODE:
        xRetval.uData.u8Mode = xMidiHandler.u8Mode;
        break;

    case MIDI_PARAM_CHANNEL:
        xRetval.uData.u8Channel = xMidiHandler.u8BaseChannel;
        break;

    case MIDI_PARAM_BANK:
        xRetval.uData.u8Bank = xMidiHandler.u8Bank;
        break;

    case MIDI_PARAM_PROGRAM:
        xRetval.uData.u8Program = xMidiHandler.u8Program;
        break;

    default:
        xRetval.eParam = MIDI_PARAM_NOT_DEF;
        break;
    }

    return xRetval;
}

void vMidiTaskInit(void)
{
    /* Create task */
    xTaskCreate(vMidiMain, MIDI_TASK_NAME, MIDI_TASK_STACK, NULL, MIDI_TASK_PRIO, &xMidiTaskHandle);
    ERR_ASSERT(xMidiTaskHandle);

    /* Create queue */
    xMidiTaskCmdQueueHandle = xQueueCreate(MIDI_TASK_CMD_QUEUE_SIZE, MIDI_TASK_CMD_QUEUE_ELEMENT_SIZE);
    ERR_ASSERT(xMidiTaskCmdQueueHandle);
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

bool bMidiSendCmd(MidiTaskCmd_t xMidiTaskCmd)
{
    bool bRetval = false;

    if ( (xMidiTaskCmdQueueHandle != NULL) && (xMidiTaskHandle != NULL) )
    {
        if ( xQueueSend(xMidiTaskCmdQueueHandle, &xMidiTaskCmd, 0U) == pdPASS )
        {
            xTaskNotify(xMidiTaskHandle, MIDI_SIGNAL_CMD_IN, eSetBits);
            bRetval = true;
        }
        else
        {
            vCliPrintf(MIDI_TASK_NAME, "CMD: Queue Error");
        }
    }

    return bRetval;
}

/* EOF */