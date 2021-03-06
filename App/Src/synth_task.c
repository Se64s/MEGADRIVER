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
#include "printf.h"
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for rx a cmd */
#define SYNTH_CMD_TIMEOUT                   (1000U)

/* Event queue size */
#define SYNTH_EVENT_QUEUE_SIZE              (5U)

/* Event queue item size */
#define SYNTH_EVENT_QUEUE_ELEMENT_SIZE      (sizeof(SynthEvent_t))

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/** Queue event handler */
QueueHandle_t xSynthEventQueueHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Handle Midi event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleMidiEvent(SynthEventPayloadMidi_t * pxEventData);

/**
  * @brief Handle SysEx Midi event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleMidiSysExEvent(SynthEventPayloadMidiSysEx_t * pxEventData);

/**
  * @brief Handle Note On Off event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleNoteOnOffEvent(SynthEventPayloadNoteOnOff_t * pxEventData);

/**
  * @brief Handle Change Note event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleChangeNoteEvent(SynthEventPayloadChangeNote_t * pxEventData);

/**
  * @brief Handle Change Parameter event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleChangeParameterEvent(SynthEventPayloadChangeParameter_t * pxEventData);

/**
  * @brief Handle Update Preset parameter event.
  * @param pxEventData pointer to event data.
  * @retval None
  */
static void vHandleUpdatePresetEvent(SynthEventPayloadUpdatePreset_t * pxEventData);

/**
  * @brief Activate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOn(SynthEventPayloadMidi_t * pxCmdMsg);

/**
  * @brief Deactivate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOff(SynthEventPayloadMidi_t * pxCmdMsg);

/**
  * @brief Deactivate all voices.
  * @retval None
  */
static void vCmdVoiceOffAll(void);

/**
  * @brief Handler for SysEx commands.
  * @param pu8SysExData pointer to sysex data.
  * @param u32LenData sysex payload len.
  * @retval None
  */
static void vCmdMidiSysEx(uint8_t * pu8SysExData, uint32_t u32LenData);

/**
  * @brief  Init user preset.
  * @retval True if preset has been initiated correctly, false inc.
  */
static bool bInitUserPreset(void);

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
  * @brief  Default setup for YM2612
  * @retval Operation result: true OK, false any error
  */
static bool bInitPreset(void);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vSynthTaskMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vHandleMidiEvent(SynthEventPayloadMidi_t * pxEventData)
{
    if (pxEventData != NULL)
    {
        switch (pxEventData->xType)
        {
        case SYNTH_CMD_NOTE_ON:
            vCmdVoiceOn(pxEventData);
            break;

        case SYNTH_CMD_NOTE_OFF:
            vCmdVoiceOff(pxEventData);
            break;

        case SYNTH_CMD_NOTE_OFF_ALL:
            vCmdVoiceOffAll();
            break;

        default:
            break;
        }
    }
}

static void vHandleMidiSysExEvent(SynthEventPayloadMidiSysEx_t * pxEventData)
{
    if ((pxEventData != NULL) && (pxEventData->pu8Data != NULL))
    {
        vCmdMidiSysEx(pxEventData->pu8Data, pxEventData->u32Len);
    }
}

static void vHandleNoteOnOffEvent(SynthEventPayloadNoteOnOff_t * pxEventData)
{
    ERR_ASSERT(pxEventData != NULL);

    uint8_t u8VoiceChannel = pxEventData->u8VoiceId;

    /* Check voice range */
    if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
    {
        if (pxEventData->bGateState)
        {
            vYM2612_key_on(u8VoiceChannel);
        }
        else
        {
            vYM2612_key_off(u8VoiceChannel);
        }
    }
    else if (u8VoiceChannel == SYNTH_MAX_NUM_VOICE)
    {
        for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
        {
            if (pxEventData->bGateState)
            {
                vYM2612_key_on(u8Index);
            }
            else
            {
                vYM2612_key_off(u8Index);
            }
        }
    }
}

static void vHandleChangeNoteEvent(SynthEventPayloadChangeNote_t * pxEventData)
{
    ERR_ASSERT(pxEventData != NULL);

    uint8_t u8VoiceChannel = pxEventData->u8VoiceId;
    uint8_t u8Note = pxEventData->u8Note;

    /* Check voice range */
    if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
    {
        if (!bYM2612_set_note(u8VoiceChannel, u8Note))
        {
            vCliPrintf(SYNTH_TASK_NAME, "VOICE: %d  SET NOTE %03d - ERROR", u8VoiceChannel, u8Note);
        }
    }
    else if (u8VoiceChannel == SYNTH_MAX_NUM_VOICE)
    {
        for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
        {
            if (!bYM2612_set_note(u8Index, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "VOICE: %d  SET NOTE %03d - ERROR", u8Index, u8Note);
            }
        }
    }
}

static void vHandleChangeParameterEvent(SynthEventPayloadChangeParameter_t * pxEventData)
{
    ERR_ASSERT(pxEventData != NULL);

    xFmDevice_t * pxDevCfg = pxYM2612_get_reg_preset();
    bool bRegUpdate = false;

    switch (pxEventData->u8ParameterId)
    {
    case FM_VAR_LFO_ON:
        pxDevCfg->u8LfoOn = pxEventData->u8Value;
        bRegUpdate = true;
        break;

    case FM_VAR_LFO_FREQ:
        pxDevCfg->u8LfoFreq = pxEventData->u8Value;
        bRegUpdate = true;
        break;

    case FM_VAR_VOICE_FEEDBACK:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].u8Feedback = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8Feedback = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_ALGORITHM:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].u8Algorithm = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8Algorithm = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_AUDIO_OUT:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].u8AudioOut = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8AudioOut = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_AMP_MOD_SENS:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].u8AmpModSens = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8AmpModSens = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_PHA_MOD_SENS:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].u8PhaseModSens = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8PhaseModSens = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_DETUNE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8Detune = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8Detune = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_MULTIPLE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8Multiple = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8Multiple = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_TOTAL_LEVEL:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8TotalLevel = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8TotalLevel = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_KEY_SCALE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8KeyScale = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8KeyScale = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_ATTACK_RATE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8AttackRate = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8AttackRate = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_AMP_MOD:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8AmpMod = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8AmpMod = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_DECAY_RATE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8DecayRate = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8DecayRate = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_RATE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8SustainRate = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8SustainRate = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_LEVEL:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8SustainLevel = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8SustainLevel = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_RELEASE_RATE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8ReleaseRate = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8ReleaseRate = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SSG_ENVELOPE:
        if (pxEventData->u8VoiceId < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[pxEventData->u8VoiceId].xOperator[pxEventData->u8operatorId].u8SsgEg = pxEventData->u8Value;
            bRegUpdate = true;
        }
        else if (pxEventData->u8VoiceId == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[pxEventData->u8operatorId].u8SsgEg = pxEventData->u8Value;
            }
            bRegUpdate = true;
        }
        break;

    default:
        break;
    }

    /* If value updated, apply new cfg */
    if (bRegUpdate)
    {
        vYM2612_set_reg_preset(pxDevCfg);
    }
}

static void vHandleUpdatePresetEvent(SynthEventPayloadUpdatePreset_t * pxEventData)
{
    ERR_ASSERT(pxEventData != NULL);

    /* Update preset values */
    vYM2612_set_reg_preset(pxEventData->pxPreset);
}

static void vCmdVoiceOn(SynthEventPayloadMidi_t * pxCmdMsg)
{
    ERR_ASSERT(pxCmdMsg != NULL);

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
            }
        }
    }
}

static void vCmdVoiceOff(SynthEventPayloadMidi_t * pxCmdMsg)
{
    ERR_ASSERT(pxCmdMsg != NULL);

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

static bool bInitUserPreset(void)
{
    bool bRetVal = false;

    bRetVal = bSYNTH_APP_DATA_init();

    /* If init fails, data could be empty o corrupted so try to write default data on it */
    if (!bRetVal)
    {
        uint8_t u8PresetId = 0U;
        const xFmDevice_t * pxInitPreset = pxSYNTH_APP_DATA_CONST_get(u8PresetId);

        if (pxInitPreset != NULL)
        {
            bRetVal = true;
            synth_app_data_t xInitPresetData = {0};

            sprintf((char*)xInitPresetData.pu8Name, "%s", "NOT INIT");
            xInitPresetData.xPresetData = *pxInitPreset;

            /* Write init preset in all user data */
            for (uint8_t u8PresetIndex = 0U; u8PresetIndex < SYNTH_APP_DATA_NUM_PRESETS; u8PresetIndex++)
            {
                if (!bSYNTH_APP_DATA_write(u8PresetIndex, &xInitPresetData))
                {
                    bRetVal = false;
                    break;
                }
            }
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "Error loading flash preset");
        }
    }

    if (bRetVal)
    {
        vCliPrintf(SYNTH_TASK_NAME, "User Preset Data Init: OK");
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "User Preset Data Init: ERROR");
    }

    return bRetVal;
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
    const synth_app_data_t * pxPresetData = pxSYNTH_APP_DATA_read(u8Position);

    if (pxPresetData != NULL)
    {
        if (bSynthSetPreset(&pxPresetData->xPresetData))
        {
            vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET %d - %s: OK", u8Position, pxPresetData->pu8Name);
            bRetVal = true;
        }
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
        if (bSynthSetPreset(pxPresetData))
        {
            vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d", u8Position);
            bRetVal = true;
        }
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d: ERROR", u8Position);
    }

    return bRetVal;
}

static void vCmdMidiSysEx(uint8_t * pu8SysExData, uint32_t u32LenData)
{
    if (u32LenData >= SYNTH_LEN_MIN_SYSEX_CMD)
    {
        SynthSysExCmd_t * pxSysExCmd = (SynthSysExCmd_t *)pu8SysExData;

        vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD %02X LEN %d", pxSysExCmd->xSysExCmd, u32LenData);

        if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_SET_PRESET) && (u32LenData == SYNTH_LEN_SET_REG_CMD))
        {
            void * pvSysExData = &pxSysExCmd->pu8CmdData;
            xFmDevice_t * pxPresetData = pvSysExData;
            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD SET PRESET");
            vYM2612_set_reg_preset(pxPresetData);
        }
        else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_SAVE_PRESET) && (u32LenData == SYNTH_LEN_SAVE_PRESET_CMD))
        {
            void * pvSysExData = &pxSysExCmd->pu8CmdData;
            SynthSysExCmdSavePreset_t * pxSavePresetData = pvSysExData;
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
        else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_LOAD_PRESET) && (u32LenData == SYNTH_LEN_LOAD_PRESET_CMD))
        {
            void * pvSysExData = &pxSysExCmd->pu8CmdData;
            SynthSysExCmdLoadPreset_t * pxLoadPresetData = pvSysExData;

            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD LOAD PRESET");

            /* Process data */
            (void)bLoadPreset(pxLoadPresetData->u8Position);
        }
        else if ((pxSysExCmd->xSysExCmd == SYNTH_SYSEX_CMD_LOAD_DEFAULT_PRESET) && (u32LenData == SYNTH_LEN_LOAD_DEFAULT_PRESET_CMD))
        {
            void * pvSysExData = &pxSysExCmd->pu8CmdData;
            SynthSysExCmdLoadPreset_t * pxLoadPresetData = pvSysExData;

            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD LOAD DEFAULT PRESET");

            (void)bLoadDefaultPreset(pxLoadPresetData->u8Position);
        }
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD too short");
    }
}

static bool bInitPreset(void)
{
    bool bRetval = false;
    uint8_t u8PresetId = 0U;
    const xFmDevice_t * pxInitPreset = pxSYNTH_APP_DATA_CONST_get(u8PresetId);

    if (pxInitPreset != NULL)
    {
        vCliPrintf(SYNTH_TASK_NAME, "Load initial preset: OK");
        vYM2612_set_reg_preset(pxInitPreset);
        bRetval = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "Load initial preset: ERROR");
    }

    return bRetval;
}

static void vSynthTaskMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(500U));

    /* Show init msg */
    vCliPrintf(SYNTH_TASK_NAME, "Init");

    /* Init YM2612 resources */
    (void)xYM2612_init();

    /* Init user preset */
    if (!bInitUserPreset())
    {
        vCliPrintf(SYNTH_TASK_NAME, "User preset init ERROR");
    }

    /* Basic register init */
    (void)bInitPreset();

    for(;;)
    {
        SynthEvent_t xEvent;

        if (xQueueReceive(xSynthEventQueueHandle, &xEvent, portMAX_DELAY) == pdPASS)
        {
            switch (xEvent.eType)
            {
                case SYNTH_EVENT_MIDI_MSG:
                    vHandleMidiEvent(&xEvent.uPayload.xMidi);
                    break;

                case SYNTH_EVENT_MIDI_SYSEX_MSG:
                    vHandleMidiSysExEvent(&xEvent.uPayload.xMidiSysEx);
                    break;

                case SYNTH_EVENT_NOTE_ON_OFF:
                    vHandleNoteOnOffEvent(&xEvent.uPayload.xNoteOnOff);
                    break;

                case SYNTH_EVENT_CHANGE_NOTE:
                    vHandleChangeNoteEvent(&xEvent.uPayload.xChangeNote);
                    break;

                case SYNTH_EVENT_MOD_PARAM:
                    vHandleChangeParameterEvent(&xEvent.uPayload.xChangeParameter);
                    break;

                case SYNTH_EVENT_UPDATE_PRESET:
                    vHandleUpdatePresetEvent(&xEvent.uPayload.xUpdatePreset);
                    break;

                default:
                    vCliPrintf(SYNTH_TASK_NAME, "Not defined event; %02X", xEvent.eType);
                    break;
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bSynthLoadPreset(SynthPresetSource_t u8PresetSource, uint8_t u8PresetId)
{
    bool bRetval = false;

    if (u8PresetSource < SYNTH_PRESET_SOURCE_MAX)
    {
        switch (u8PresetSource)
        {
            case SYNTH_PRESET_SOURCE_DEFAULT:
                bRetval = bLoadDefaultPreset(u8PresetId);
                break;

            case SYNTH_PRESET_SOURCE_USER:
                bRetval = bLoadPreset(u8PresetId);
                break;

            default:
                break;
        }
    }

    return bRetval;
}

bool bSynthSaveUserPreset(xFmDevice_t * pxPreset, uint8_t u8PresetId)
{
    uint8_t u8UiPresetName[SYNTH_LEN_PRESET_NAME] = "UI User Preset";
    return bSavePreset(u8PresetId, u8UiPresetName, pxPreset);
}

bool bSynthSetPreset(xFmDevice_t * pxPreset)
{
    ERR_ASSERT(pxPreset != NULL);
    ERR_ASSERT(xSynthEventQueueHandle != NULL);

    bool bRetval = true;
    SynthEvent_t xSynthEvent = {.eType = SYNTH_EVENT_UPDATE_PRESET, .uPayload.xUpdatePreset.pxPreset = pxPreset};

    if (xQueueSend(xSynthEventQueueHandle, &xSynthEvent, pdMS_TO_TICKS(SYNTH_QUEUE_TIMEOUT)) != pdPASS)
    {
        vCliPrintf(SYNTH_TASK_NAME, "CMD: Queue Error");
        bRetval = false;
    }

    return bRetval;
}

bool bSynthTaskInit(void)
{
    bool bRetval = false;

    /* Create task */
    xTaskCreate(vSynthTaskMain, SYNTH_TASK_NAME, SYNTH_TASK_STACK, NULL, SYNTH_TASK_PRIO, &xSynthTaskHandle);

    /* Create task queue */
    xSynthEventQueueHandle = xQueueCreate(SYNTH_EVENT_QUEUE_SIZE, SYNTH_EVENT_QUEUE_ELEMENT_SIZE);

    /* Check resources */
    if ((xSynthTaskHandle != NULL) && (xSynthEventQueueHandle != NULL))
    {
        bRetval = true;
    }

    return bRetval;
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

QueueHandle_t pxSynthTaskGetQueue(void)
{
    return xSynthEventQueueHandle;
}

/*****END OF FILE****/