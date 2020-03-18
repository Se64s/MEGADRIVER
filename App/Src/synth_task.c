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
#include "midi_task.h"

#include "synth_app_data.h"
#include "synth_app_data_const.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for rx a cmd */
#define SYNTH_CMD_TIMEOUT       (1000U)

/* Size of cmd buff */
#define SYNTH_TMP_CMD_SIZE      (3U)

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Activate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOn(SynthMsg_t * pxCmdMsg);

/**
  * @brief Deactivate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOff(SynthMsg_t * pxCmdMsg);

/**
  * @brief Deactivate all voices.
  * @retval None
  */
static void vCmdVoiceOffAll(void);

/**
  * @brief Save preset on fixed position.
  * @param u8Position Position where save the preset.
  * @param pu8Name Pointer with preset name.
  * @param pxRegData Preset reg data.
  * @retval true, preset saved, false, error on saving.
  */
static bool bSavePreset(uint8_t u8Position, uint8_t * pu8Name, xFmDevice_t * pxRegData);

/**
  * @brief Load preset.
  * @param u8Position Position where save the preset.
  * @retval true, preset loaded, false, error on loading.
  */
static bool bLoadPreset(uint8_t u8Position);

/**
  * @brief Load default preset.
  * @param u8Position Position where save the preset.
  * @retval true, preset loaded, false, error on loading.
  */
static bool bLoadDefaultPreset(uint8_t u8Position);

/**
  * @brief Initial YM2612 setup
  * @retval None
  */
static void _init_setup(void);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vSynthTaskMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vCmdVoiceOn(SynthMsg_t * pxCmdMsg)
{
    if (pxCmdMsg->xType == SYNTH_CMD_NOTE_ON)
    {
        uint8_t u8VoiceChannel = pxCmdMsg->u8Data[0U];
        uint8_t u8Note = pxCmdMsg->u8Data[1U];

        /* Check voice range */
        if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
        {
            if (bYM2612_set_note(u8VoiceChannel, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "Key  ON: %02d - %03d", u8VoiceChannel, u8Note);
                vYM2612_key_on(u8VoiceChannel);
                bUiTaskNotify(UI_SIGNAL_SYNTH_ON);
            }
        }
    }
}

static void vCmdVoiceOff(SynthMsg_t * pxCmdMsg)
{
    if (pxCmdMsg->xType == SYNTH_CMD_NOTE_OFF)
    {
        uint8_t u8VoiceChannel = pxCmdMsg->u8Data[0U];
        uint8_t u8Note = pxCmdMsg->u8Data[1U];

        /* Check voice range */
        if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
        {
            if (bYM2612_set_note(u8VoiceChannel, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "Key OFF: %02d - %03d", u8VoiceChannel, u8Note);
                vYM2612_key_off(u8VoiceChannel);
                bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
            }
        }
    }
}

static void vCmdVoiceOffAll(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Clear ALL voices");
    for (uint8_t u8VoiceIndex = 0U; u8VoiceIndex < SYNTH_MAX_NUM_VOICE; u8VoiceIndex++)
    {
        vYM2612_key_off(u8VoiceIndex);
    }
    bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
}

static bool bSavePreset(uint8_t u8Position, uint8_t * pu8Name, xFmDevice_t * pxRegData)
{
    bool bRetVal = false;

    synth_app_data_t xPresetData = {0};

    /* Copy name */
    (void)memcpy(&xPresetData.pu8Name, pu8Name, SYNTH_LEN_PRESET_NAME);
    /* Copy reg */
    (void)memcpy(&xPresetData.xPresetData, pxRegData, sizeof(xFmDevice_t));

    if (bSYNTH_APP_DATA_write(u8Position, &xPresetData))
    {
        vCliPrintf(SYNTH_TASK_NAME, "SAVE PRESET %d - %s: OK", u8Position, pu8Name);

        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "SAVE PRESET %d - %s: ERROR", u8Position, pu8Name);
    }

    return bRetVal;
}

static bool bLoadPreset(uint8_t u8Position)
{
    bool bRetVal = false;
    const synth_app_data_t * pxPresetData = NULL;

    if (bSYNTH_APP_DATA_read(u8Position, &pxPresetData))
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET %d - %s: OK", u8Position, pxPresetData->pu8Name);
        vYM2612_set_reg_preset(&pxPresetData->xPresetData);
        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET %d: ERROR", u8Position);
    }

    return bRetVal;
}

static bool bLoadDefaultPreset(uint8_t u8Position)
{
    bool bRetVal = false;
    const xFmDevice_t * pxPresetData = pxSYNTH_APP_DATA_CONST_get(u8Position);

    if (pxPresetData != NULL)
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d", u8Position);
        vYM2612_set_reg_preset(pxPresetData);
        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d: ERROR", u8Position);
    }

    return bRetVal;
}

static void vCmdMidiSysEx(void)
{
    uint32_t u32SysExLenData = 0U;
    uint8_t * pu8SysExData;

    if (midi_get_sysex_data(&pu8SysExData, &u32SysExLenData) == midiOk)
    {
        if (u32SysExLenData >= SYNTH_LEN_MIN_SYSEX_CMD)
        {
            SynthSysExCmd_t * pxSysExCmd = (SynthSysExCmd_t *)pu8SysExData;

            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD %02X LEN %d", pxSysExCmd->xSysExCmd, u32SysExLenData);

            if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_SET_PRESET) && (u32SysExLenData == SYNTH_LEN_SET_REG_CMD))
            {
                xFmDevice_t * pxPresetData = &pxSysExCmd->pu8CmdData;
                vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD SET PRESET");
                vYM2612_set_reg_preset(pxPresetData);
            }
            else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_SAVE_PRESET) && (u32SysExLenData == SYNTH_LEN_SAVE_PRESET_CMD))
            {
                SynthSysExCmdSavePreset_t * pxSavePresetData = &pxSysExCmd->pu8CmdData;
                uint8_t pu8PresetName[SYNTH_LEN_PRESET_NAME] = {0};

                vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD SAVE PRESET");

                /* Extract name from sysex frame */
                for (uint32_t u32NameIndex = 0U; u32NameIndex < (SYNTH_LEN_PRESET_NAME - 1U); u32NameIndex++)
                {
                    pu8PresetName[u32NameIndex] = pxSavePresetData->u8CodedName[u32NameIndex * 2U];
                    pu8PresetName[u32NameIndex] |= pxSavePresetData->u8CodedName[(u32NameIndex * 2U) + 1U] << 4U;
                }
                /* Process data */
                bSavePreset(pxSavePresetData->u8Position, pu8PresetName, &pxSavePresetData->xRegData);
            }
            else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_LOAD_PRESET) && (u32SysExLenData == SYNTH_LEN_LOAD_PRESET_CMD))
            {
                SynthSysExCmdLoadPreset_t * pxLoadPresetData = &pxSysExCmd->pu8CmdData;

                vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD LOAD PRESET");

                /* Process data */
                (void)bLoadPreset(pxLoadPresetData->u8Position);
            }
            else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_LOAD_DEFAULT_PRESET) && (u32SysExLenData == SYNTH_LEN_LOAD_DEFAULT_PRESET_CMD))
            {
                SynthSysExCmdLoadPreset_t * pxLoadPresetData = &pxSysExCmd->pu8CmdData;

                vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD LOAD DEFAULT PRESET");

                (void)bLoadDefaultPreset(pxLoadPresetData->u8Position);
            }
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD too short");
        }
    }
}

static void _init_setup(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Initial register setup");

    uint8_t u8PresetId = 0U;
    xFmDevice_t * pxInitPreset = pxSYNTH_APP_DATA_CONST_get(u8PresetId);

    if (pxInitPreset != NULL)
    {
        vYM2612_set_reg_preset(pxInitPreset);
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "Error loading flash preset");
    }
}

static void vSynthTaskMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(500U));

    /* Init YM2612 resources */
    (void)xYM2612_init();

    /* Init preset data */
    (void)bSYNTH_APP_DATA_init();

    /* Show init msg */
    vCliPrintf(SYNTH_TASK_NAME, "Init");

    /* Basic register init */
    _init_setup();

    for(;;)
    {
      MessageBufferHandle_t MsgBuff = xMidiGetMessageBuffer();

      if (MsgBuff != NULL)
      {
        size_t xReceivedBytes;
        SynthMsg_t xSynthCmd = {0};

        /* Get cmd from buffer */
        xReceivedBytes = xMessageBufferReceive(MsgBuff, (void *) &xSynthCmd, sizeof(SynthMsg_t), pdMS_TO_TICKS(SYNTH_CMD_TIMEOUT));

        /* Handle cmd if not empty */
        if (xReceivedBytes == sizeof(SynthMsg_t))
        {
            if (xSynthCmd.xType == SYNTH_CMD_NOTE_ON)
            {
                vCmdVoiceOn(&xSynthCmd);
            }
            else if (xSynthCmd.xType == SYNTH_CMD_NOTE_OFF)
            {
                vCmdVoiceOff(&xSynthCmd);
            }
            else if (xSynthCmd.xType == SYNTH_CMD_NOTE_OFF_ALL)
            {
                vCmdVoiceOffAll();
            }
            else if (xSynthCmd.xType == SYNTH_CMD_SYSEX)
            {
                vCmdMidiSysEx();
            }
        }
      }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bSynthTaskInit(void)
{
    bool retval = false;

    /* Create task */
    xTaskCreate(vSynthTaskMain, SYNTH_TASK_NAME, SYNTH_TASK_STACK, NULL, SYNTH_TASK_PRIO, &xSynthTaskHandle);

    /* Check resources */
    if (xSynthTaskHandle != NULL)
    {
        retval = true;
    }
    return(retval);
}

bool bSynthTaskNotify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (xSynthTaskHandle != NULL)
    {
      xTaskNotify(xSynthTaskHandle, u32Event, eSetBits);
      bRetval = true;
    }
    return bRetval;
}

/*****END OF FILE****/