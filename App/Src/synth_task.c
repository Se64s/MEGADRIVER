/**
  ******************************************************************************
  * @file           : synth_task.c
  * @brief          : Task to handle synth chip interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "synth_task.h"

#include "sys_rtos.h"

#include "cli_task.h"
// #include "ui_task.h"
#include "midi_task.h"

#include "printf.h"
#include "user_error.h"

/* Private define ------------------------------------------------------------*/

/* Timeout for rx a cmd */
#define SYNTH_CMD_TIMEOUT                   ( 1000U )

/* Event queue size */
#define SYNTH_EVENT_QUEUE_SIZE              ( 15U )

/* Event queue item size */
#define SYNTH_EVENT_QUEUE_ELEMENT_SIZE      ( sizeof(SynthCmd_t) )

/* Special registers for midi CC conversion */
#define SYNTH_CC_REG_NONE                   ( 0x00U )
#define SYNTH_CC_REG_VOICE                  ( 0xF0U )
#define SYNTH_CC_REG_OPERATOR               ( 0xF1U )
#define SYNTH_CC_REG_NOT_FOUND              ( 0xFFU )

/* Private typedef -----------------------------------------------------------*/

/** Voice data structure */
typedef struct
{
    uint8_t u8Note;
    uint8_t u8Velocity;
} SynthVoice_t;

/** Voice mono control structure */
typedef struct
{
    SynthVoice_t xVoice[SYNTH_MAX_NUM_VOICE];
    SynthVoice_t xVoiceTmp[SYNTH_MAX_NUM_VOICE];
} SynthCtrlMono_t;

/** Voice poly control structure */
typedef struct
{
    SynthVoice_t xVoice[SYNTH_MAX_NUM_VOICE];
    SynthVoice_t xVoiceTmp;
} SynthCtrlPoly_t;

/** Handler for synth task */
typedef struct
{
    uint8_t u8CcVoice;
    uint8_t u8CcOperator;
    SynthCtrlMono_t xCtrlMono;
    SynthCtrlPoly_t xCtrlPoly;
} SynthCtrl_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/** Queue event handler */
QueueHandle_t xSynthEventQueueHandle = NULL;

/** Synth device handler */
SynthCtrl_t xSynthDevHandler = { 0U };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Handle synth cmd voice update mono.
  * @param pxCmdData pointer to event data.
  * @retval None
  */
static void vHandleCmdVoiceUpdateMono(SynthCmdPayloadVoiceUpdateMono_t * pxCmdData);

/**
  * @brief Handle synth cmd voice update poly.
  * @param pxCmdData pointer to event data.
  * @retval None
  */
static void vHandleCmdVoiceUpdatePoly(SynthCmdPayloadVoiceUpdatePoly_t * pxCmdData);

/**
  * @brief Handle synth cmd parameter update.
  * @param pxCmdData pointer to event data.
  * @retval None
  */
static void vHandleCmdParameterUpdate(SynthCmdPayloadParamUpdate_t * pxCmdData);

/**
 * @brief Cast Cc command id to YM register id
 * @param u8CcId Midi Cc value to cast
 * @return uint8_t YM register value, 0xFF if not match found.
 */
static uint8_t u8CcToRegId(uint8_t u8CcId);

/**
 * @brief Cast Cc value to register value.
 * @param u8CcId Midi Cc Id.
 * @param u8CcData Midi Cc value to cast.
 * @return uint8_t YM register value, 0xFF if not match found.
 */
static uint8_t u8CcToRegData(uint8_t u8CcId, uint8_t u8CcData);

/**
  * @brief Handle synth cmd preset update.
  * @param pxCmdData pointer to event data.
  * @retval None
  */
static void vHandleCmdPresetUpdate(SynthCmdPayloadPresetUpdate_t * pxCmdData);

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
static bool bSavePresetFlash(uint8_t u8Position, uint8_t * pu8Name, xFmDevice_t * pxRegData);

/**
  * @brief Load preset.
  * @param u8Position Position where save the preset.
  * @retval true, preset loaded, false, error on loading.
  */
static bool bLoadPresetFlash(uint8_t u8Position);

/**
  * @brief Load default preset.
  * @param u8Position Position where save the preset.
  * @retval true, preset loaded, false, error on loading.
  */
static bool bLoadPresetRom(uint8_t u8Position);

/**
  * @brief  Default setup for YM2612
  * @retval Operation result: true OK, false any error
  */
static bool bInitPreset(void);

/**
  * @brief Deactivate all voices.
  * @retval None
  */
static void vCmdVoiceOffAll(void);

/**
 * @brief Handle voice on in mono mode.
 * @param u8Voice Voice to update
 * @param u8Note note to activate
 * @param u8Velocity velocity of action
 */
static void vHandleVoiceMonoOn(uint8_t u8Voice, uint8_t u8Note, uint8_t u8Velocity);

/**
 * @brief Handle voice off in mono mode.
 * @param u8Voice Voice to update
 * @param u8Note note to activate
 * @param u8Velocity velocity of action
 */
static void vHandleVoiceMonoOff(uint8_t u8Voice, uint8_t u8Note, uint8_t u8Velocity);

/**
 * @brief Handle voice on in poly mode.
 * @param u8Note note to activate
 * @param u8Velocity velocity of action
 */
static void vHandleVoicePolyOn(uint8_t u8Note, uint8_t u8Velocity);

/**
 * @brief Handle voice off in poly mode.
 * @param u8Note note to activate
 * @param u8Velocity velocity of action
 */
static void vHandleVoicePolyOff(uint8_t u8Note, uint8_t u8Velocity);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vSynthTaskMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vHandleCmdVoiceUpdateMono(SynthCmdPayloadVoiceUpdateMono_t * pxCmdData)
{
    ERR_ASSERT(pxCmdData);

    if ( pxCmdData->u8VoiceState == (uint8_t)SYNTH_VOICE_STATE_ON )
    {
        if (pxCmdData->u8Velocity != 0U)
        {
            vHandleVoiceMonoOn(pxCmdData->u8VoiceDst, pxCmdData->u8Note, pxCmdData->u8Velocity);
        }
        else
        {
            vHandleVoiceMonoOff(pxCmdData->u8VoiceDst, pxCmdData->u8Note, pxCmdData->u8Velocity);
        }
    }
    else if ( pxCmdData->u8VoiceState == (uint8_t)SYNTH_VOICE_STATE_OFF )
    {
        vHandleVoiceMonoOff(pxCmdData->u8VoiceDst, pxCmdData->u8Note, pxCmdData->u8Velocity);
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "Not valid state for voice command: %d", pxCmdData->u8VoiceState);
    }
}

static void vHandleCmdVoiceUpdatePoly(SynthCmdPayloadVoiceUpdatePoly_t * pxCmdData)
{
    ERR_ASSERT(pxCmdData);

#ifdef SYNTH_DBG_VERBOSE
    vCliPrintf(SYNTH_TASK_NAME, "VOICE_UPDATE_POLY: State %02X, Note %02X, Vel %02X, ", pxCmdData->u8VoiceState, pxCmdData->u8Note, pxCmdData->u8Velocity);
#endif

    if ( pxCmdData->u8VoiceState == (uint8_t)SYNTH_VOICE_STATE_ON )
    {
        if (pxCmdData->u8Velocity != 0U)
        {
            vHandleVoicePolyOn(pxCmdData->u8Note, pxCmdData->u8Velocity);
        }
        else
        {
            vHandleVoicePolyOff(pxCmdData->u8Note, pxCmdData->u8Velocity);
        }
    }
    else if ( pxCmdData->u8VoiceState == (uint8_t)SYNTH_VOICE_STATE_OFF )
    {
        vHandleVoicePolyOff(pxCmdData->u8Note, pxCmdData->u8Velocity);
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "Not valid state for voice command: %d", pxCmdData->u8VoiceState);
    }
}

static void vHandleCmdParameterUpdate(SynthCmdPayloadParamUpdate_t * pxCmdData)
{
    ERR_ASSERT(pxCmdData);

    xFmDevice_t * pxDevCfg = pxYM2612_get_reg_preset();

    bool bRegUpdate = false;

    uint8_t u8RegId = u8CcToRegId(pxCmdData->u8Id);
    uint8_t u8RegData = u8CcToRegData(pxCmdData->u8Id, pxCmdData->u8Data);
    uint8_t u8RegVoice = xSynthDevHandler.u8CcVoice;
    uint8_t u8RegOperator = xSynthDevHandler.u8CcOperator;

    switch (u8RegId)
    {
    case FM_VAR_LFO_ON:
        pxDevCfg->u8LfoOn = u8RegData;
        bRegUpdate = true;
        break;

    case SYNTH_CC_REG_OPERATOR:
        if (u8RegData < YM2612_NUM_OP_CHANNEL)
        {
            xSynthDevHandler.u8CcOperator = u8RegData;
            bRegUpdate = true;
        }
        break;

    case SYNTH_CC_REG_VOICE:
        if (u8RegData <= YM2612_NUM_CHANNEL)
        {
            xSynthDevHandler.u8CcOperator = u8RegData;
            bRegUpdate = true;
        }
        break;

    case FM_VAR_LFO_FREQ:
        pxDevCfg->u8LfoFreq = u8RegData;
        bRegUpdate = true;
        break;

    case FM_VAR_VOICE_FEEDBACK:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].u8Feedback = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8Feedback = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_ALGORITHM:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].u8Algorithm = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8Algorithm = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_AUDIO_OUT:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].u8AudioOut = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8AudioOut = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_AMP_MOD_SENS:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].u8AmpModSens = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8AmpModSens = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_VOICE_PHA_MOD_SENS:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].u8PhaseModSens = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].u8PhaseModSens = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_DETUNE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8Detune = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8Detune = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_MULTIPLE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8Multiple = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8Multiple = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_TOTAL_LEVEL:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8TotalLevel = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8TotalLevel = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_KEY_SCALE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8KeyScale = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8KeyScale = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_ATTACK_RATE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8AttackRate = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8AttackRate = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_AMP_MOD:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8AmpMod = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8AmpMod = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_DECAY_RATE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8DecayRate = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8DecayRate = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_RATE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8SustainRate = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8SustainRate = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_LEVEL:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8SustainLevel = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8SustainLevel = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_RELEASE_RATE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8ReleaseRate = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8ReleaseRate = u8RegData;
            }
            bRegUpdate = true;
        }
        break;

    case FM_VAR_OPERATOR_SSG_ENVELOPE:
        if (u8RegVoice < SYNTH_MAX_NUM_VOICE)
        {
            pxDevCfg->xChannel[u8RegVoice].xOperator[u8RegOperator].u8SsgEg = u8RegData;
            bRegUpdate = true;
        }
        else if (u8RegVoice == SYNTH_MAX_NUM_VOICE)
        {
            for (uint8_t u8Index = 0U; u8Index < SYNTH_MAX_NUM_VOICE; u8Index++)
            {
                pxDevCfg->xChannel[u8Index].xOperator[u8RegOperator].u8SsgEg = u8RegData;
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
#ifdef SYNTH_DBG_VERBOSE
        vCliPrintf(SYNTH_TASK_NAME, "Process parameter: CC %02X - %02X, REG %02X - %02X", pxCmdData->u8Id, pxCmdData->u8Data, u8RegId, u8RegData);
#endif
        vYM2612_set_reg_preset(pxDevCfg);
    }
    else
    {
#ifdef SYNTH_DBG_VERBOSE
        vCliPrintf(SYNTH_TASK_NAME, "Process parameter: CC %02X - %02X, FAIL", pxCmdData->u8Id, pxCmdData->u8Data);
#endif
    }
}

static uint8_t u8CcToRegId(uint8_t u8CcId)
{
    uint8_t u8RegId = SYNTH_CC_REG_NONE;

    switch ( u8CcId )
    {
    case MIDI_CC_C20:
        u8RegId = (uint8_t)FM_VAR_LFO_ON;
        break;

    case MIDI_CC_C21:
        u8RegId = (uint8_t)FM_VAR_LFO_FREQ;
        break;

    case MIDI_CC_C22:
        u8RegId = SYNTH_CC_REG_VOICE;
        break;

    case MIDI_CC_C23:
        u8RegId = SYNTH_CC_REG_OPERATOR;
        break;

    case MIDI_CC_C24:
        u8RegId = (uint8_t)FM_VAR_VOICE_FEEDBACK;
        break;

    case MIDI_CC_C15:
        u8RegId = (uint8_t)FM_VAR_VOICE_ALGORITHM;
        break;

    case MIDI_CC_C25:
        u8RegId = (uint8_t)FM_VAR_VOICE_AUDIO_OUT;
        break;

    case MIDI_CC_C26:
        u8RegId = (uint8_t)FM_VAR_VOICE_AMP_MOD_SENS;
        break;

    case MIDI_CC_C27:
        u8RegId = (uint8_t)FM_VAR_VOICE_PHA_MOD_SENS;
        break;

    case MIDI_CC_C52:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_DETUNE;
        break;

    case MIDI_CC_C53:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_MULTIPLE;
        break;

    case MIDI_CC_C54:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_TOTAL_LEVEL;
        break;

    case MIDI_CC_C55:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_KEY_SCALE;
        break;

    case MIDI_CC_C56:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_ATTACK_RATE;
        break;

    case MIDI_CC_C57:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_AMP_MOD;
        break;

    case MIDI_CC_C58:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_DECAY_RATE;
        break;

    case MIDI_CC_C59:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_SUSTAIN_RATE;
        break;

    case MIDI_CC_C60:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_SUSTAIN_LEVEL;
        break;

    case MIDI_CC_C61:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_RELEASE_RATE;
        break;

    case MIDI_CC_C62:
        u8RegId = (uint8_t)FM_VAR_OPERATOR_SSG_ENVELOPE;
        break;

    default:
        u8RegId = SYNTH_CC_REG_NOT_FOUND;
        break;
    }

    return u8RegId;
}

static uint8_t u8CcToRegData(uint8_t u8CcId, uint8_t u8CcData)
{
    uint8_t u8RegData = 0U;

    switch ( u8CcId )
    {
    case MIDI_CC_C20:
        u8RegData = (u8CcData < MAX_VALUE_LFO_ON) ? u8CcData : (MAX_VALUE_LFO_ON - 1U);
        break;

    case MIDI_CC_C21:
        u8RegData = (u8CcData < MAX_VALUE_LFO_FREQ) ? u8CcData : (MAX_VALUE_LFO_FREQ - 1U);
        break;

    case MIDI_CC_C22:
        u8RegData = (u8CcData <= YM2612_NUM_CHANNEL) ? u8CcData : YM2612_NUM_CHANNEL;
        break;

    case MIDI_CC_C23:
        u8RegData = (u8CcData < YM2612_NUM_OP_CHANNEL) ? u8CcData : (YM2612_NUM_OP_CHANNEL - 1U);
        break;

    case MIDI_CC_C24:
        u8RegData = (u8CcData < MAX_VALUE_FEEDBACK) ? u8CcData : (MAX_VALUE_FEEDBACK - 1U);
        break;

    case MIDI_CC_C15:
        u8RegData = (u8CcData < MAX_VALUE_ALGORITHM) ? u8CcData : (MAX_VALUE_ALGORITHM - 1U);
        break;

    case MIDI_CC_C25:
        u8RegData = (u8CcData < MAX_VALUE_VOICE_OUT) ? u8CcData : (MAX_VALUE_VOICE_OUT - 1U);
        break;

    case MIDI_CC_C26:
        u8RegData = (u8CcData < MAX_VALUE_AMP_MOD_SENS) ? u8CcData : (MAX_VALUE_AMP_MOD_SENS - 1U);
        break;

    case MIDI_CC_C27:
        u8RegData = (u8CcData < MAX_VALUE_PHA_MOD_SENS) ? u8CcData : (MAX_VALUE_PHA_MOD_SENS - 1U);
        break;

    case MIDI_CC_C52:
        u8RegData = (u8CcData < MAX_VALUE_DETUNE) ? u8CcData : (MAX_VALUE_DETUNE - 1U);
        break;

    case MIDI_CC_C53:
        u8RegData = (u8CcData < MAX_VALUE_MULTIPLE) ? u8CcData : (MAX_VALUE_MULTIPLE - 1U);
        break;

    case MIDI_CC_C54:
        u8RegData = (u8CcData < MAX_VALUE_TOTAL_LEVEL) ? u8CcData : (MAX_VALUE_TOTAL_LEVEL - 1U);
        break;

    case MIDI_CC_C55:
        u8RegData = (u8CcData < MAX_VALUE_KEY_SCALE) ? u8CcData : (MAX_VALUE_KEY_SCALE - 1U);
        break;

    case MIDI_CC_C56:
        u8RegData = (u8CcData < MAX_VALUE_ATTACK_RATE) ? u8CcData : (MAX_VALUE_ATTACK_RATE - 1U);
        break;

    case MIDI_CC_C57:
        u8RegData = (u8CcData < MAX_VALUE_AMP_MOD_EN) ? u8CcData : (MAX_VALUE_AMP_MOD_EN - 1U);
        break;

    case MIDI_CC_C58:
        u8RegData = (u8CcData < MAX_VALUE_DECAY_RATE) ? u8CcData : (MAX_VALUE_DECAY_RATE - 1U);
        break;

    case MIDI_CC_C59:
        u8RegData = (u8CcData < MAX_VALUE_SUSTAIN_RATE) ? u8CcData : (MAX_VALUE_SUSTAIN_RATE - 1U);
        break;

    case MIDI_CC_C60:
        u8RegData = (u8CcData < MAX_VALUE_SUSTAIN_LEVEL) ? u8CcData : (MAX_VALUE_SUSTAIN_LEVEL - 1U);
        break;

    case MIDI_CC_C61:
        u8RegData = (u8CcData < MAX_VALUE_RELEASE_RATE) ? u8CcData : (MAX_VALUE_RELEASE_RATE - 1U);
        break;

    case MIDI_CC_C62:
        u8RegData = (u8CcData < MAX_VALUE_SSG_ENVELOPE) ? u8CcData : (MAX_VALUE_SSG_ENVELOPE - 1U);
        break;

    default:
        // Not CC found
        break;
    }

    return u8RegData;
}

static void vHandleCmdPresetUpdate(SynthCmdPayloadPresetUpdate_t * pxCmdData)
{
    ERR_ASSERT(pxCmdData);

    if ( pxCmdData->u8Action == (uint8_t)SYNTH_PRESET_ACTION_LOAD )
    {
        if ( pxCmdData->u8Bank == (uint8_t)LFS_MIDI_BANK_ROM )
        {
            (void)bLoadPresetRom(pxCmdData->u8Program);
        }
        else if ( pxCmdData->u8Bank == (uint8_t)LFS_MIDI_BANK_FLASH )
        {
            (void)bLoadPresetFlash(pxCmdData->u8Program);
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET: Not valid Bank - %d", pxCmdData->u8Bank);
        }
    }
    else if ( pxCmdData->u8Action == (uint8_t)SYNTH_PRESET_ACTION_SAVE )
    {
        if ( pxCmdData->u8Bank == (uint8_t)LFS_MIDI_BANK_FLASH )
        {
            xFmDevice_t * pxDevCfg = pxYM2612_get_reg_preset();
            char pcTestStr[] = "UserPresetLive";

            (void)bSavePresetFlash(pxCmdData->u8Program, (uint8_t *)&pcTestStr, pxDevCfg);
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "SAVE PRESET: Not valid Bank - %d", pxCmdData->u8Bank);
        }
    }
}

static bool bInitUserPreset(void)
{
    bool bRetVal = false;

    /* If init fails, data could be empty o corrupted so try to write default data on it */
    if ( LFS_init() == LFS_OK)
    {
        vCliPrintf(SYNTH_TASK_NAME, "User Preset Data Init: OK");
        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "User Preset Data Init: ERROR");
        ERR_ASSERT(0U);
    }

    return bRetVal;
}

static bool bSavePresetFlash(uint8_t u8Position, uint8_t * pu8Name, xFmDevice_t * pxRegData)
{
    bool bRetVal = false;
    lfs_ym_data_t xPresetData = {0};

    if (u8Position < LFS_YM_SLOT_NUM)
    {
        /* Copy name */
        (void)memcpy(&xPresetData.pu8Name, pu8Name, LFS_YM_CF_NAME_MAX_LEN);
        /* Copy reg */
        (void)memcpy(&xPresetData.xPresetData, pxRegData, sizeof(xFmDevice_t));

        if ( LFS_write_ym_data(u8Position, &xPresetData) == LFS_OK )
        {
            vCliPrintf(SYNTH_TASK_NAME, "SAVE PRESET %d - %s: OK", u8Position, pu8Name);
            bRetVal = true;
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "SAVE PRESET %d - %s: ERROR", u8Position, pu8Name);
            ERR_ASSERT(0U);
        }
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "SAVE SLOT NOT VALID");
    }

    return bRetVal;
}

static bool bLoadPresetFlash(uint8_t u8Position)
{
    bool bRetVal = false;
    lfs_ym_data_t xPresetData = {0};

    if ( LFS_read_ym_data(u8Position, &xPresetData) == LFS_OK )
    {
        vYM2612_set_reg_preset(&xPresetData.xPresetData);

        vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET %d - %s: OK", u8Position, xPresetData.pu8Name);
        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD PRESET %d: ERROR", u8Position);
        ERR_ASSERT(0U);
    }

    return bRetVal;
}

static bool bLoadPresetRom(uint8_t u8Position)
{
    bool bRetVal = false;
    xFmDevice_t * pxPresetData = (xFmDevice_t *)pxSYNTH_APP_DATA_CONST_get(u8Position);

    if (pxPresetData != NULL)
    {
        vYM2612_set_reg_preset(pxPresetData);

        vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d", u8Position);
        bRetVal = true;
    }
    else
    {
        vCliPrintf(SYNTH_TASK_NAME, "LOAD DEFAULT PRESET %d: ERROR", u8Position);
    }

    return bRetVal;
}

static bool bInitPreset(void)
{
    bool bRetval = false;
    uint8_t u8PresetId = 0U;
    xFmDevice_t * pxInitPreset = (xFmDevice_t *)pxSYNTH_APP_DATA_CONST_get(u8PresetId);

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

static void vCmdVoiceOffAll(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Clear ALL voices");

    /* Clear voices in synth chip and control structure */
    for (uint8_t u8VoiceIndex = 0U; u8VoiceIndex < SYNTH_MAX_NUM_VOICE; u8VoiceIndex++)
    {
        xSynthDevHandler.xCtrlMono.xVoice[u8VoiceIndex].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlMono.xVoice[u8VoiceIndex].u8Velocity = MIDI_DATA_NOT_VALID;

        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8VoiceIndex].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8VoiceIndex].u8Velocity = MIDI_DATA_NOT_VALID;

        xSynthDevHandler.xCtrlPoly.xVoice[u8VoiceIndex].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlPoly.xVoice[u8VoiceIndex].u8Velocity = MIDI_DATA_NOT_VALID;

        vYM2612_key_off(u8VoiceIndex);
    }

    xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Note = MIDI_DATA_NOT_VALID;
    xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Velocity = MIDI_DATA_NOT_VALID;

    // bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
}

static void vHandleVoiceMonoOn(uint8_t u8Voice, uint8_t u8Note, uint8_t u8Velocity)
{
    /* Check if voice is in use */
    if ( xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Note == MIDI_DATA_NOT_VALID )
    {
        /* Build synth NOTE ON command */
        if ( u8Voice < SYNTH_MAX_NUM_VOICE )
        {
            if ( bYM2612_set_note(u8Voice, u8Note) )
            {
                vYM2612_key_on(u8Voice);

                /* Update control structure */
                xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Note = u8Note;
                xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Velocity = u8Velocity;

#ifdef SYNTH_DBG_VERBOSE
                vCliPrintf(SYNTH_TASK_NAME, "Key  ON : %02d - %03d", u8Voice, u8Note);
#endif
            }
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "Voice id not valid: %d", u8Voice);
        }
    }
    else if ( xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Note != u8Note )
    {
        /* If note is not already pressed, store in temporal position */
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note = u8Note;
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity = u8Velocity;
    }
}

static void vHandleVoiceMonoOff(uint8_t u8Voice, uint8_t u8Note, uint8_t u8Velocity)
{
    /* Check if note is in use */
    if ( xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Note == u8Note )
    {
        /* Sed voice off */
        vYM2612_key_off(u8Voice);

#ifdef SYNTH_DBG_VERBOSE
        vCliPrintf(SYNTH_TASK_NAME, "Key  OFF: %02d - %03d", u8Voice, u8Note);
#endif

        /* Update control structure */
        xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlMono.xVoice[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

        /* Check tmp note */
        if ( xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note != MIDI_DATA_NOT_VALID )
        {
            /* Update control structure */
            uint8_t u8TmpNote = xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note;
            uint8_t u8TmpVelocity = xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity;

            xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
            xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

            /* Generate new note */
            vHandleVoiceMonoOn(u8Voice, u8TmpNote, u8TmpVelocity);
        }
    }
    else if ( xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note == u8Note )
    {
        /* Clear tmp note */
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;
    }
}

static void vHandleVoicePolyOn(uint8_t u8Note, uint8_t u8Velocity)
{
    /* Search for voice */
    uint8_t u8Voice = MIDI_DATA_NOT_VALID;

    /* Check if note is already active */
    for ( uint32_t u32IndexVoice = 0U; u32IndexVoice < SYNTH_MAX_NUM_VOICE; u32IndexVoice++ )
    {
        if ( xSynthDevHandler.xCtrlPoly.xVoice[u32IndexVoice].u8Note == u8Note )
        {
            u8Voice = MIDI_DATA_NOT_VALID;
            break;
        }
        /* Check and save free voice index to not iterate after */
        else if ( xSynthDevHandler.xCtrlPoly.xVoice[u32IndexVoice].u8Note == MIDI_DATA_NOT_VALID )
        {
            /* Check if voice has been used before */
            if (u8Voice == MIDI_DATA_NOT_VALID)
            {
                u8Voice = u32IndexVoice;
            }
        }
    }

    /* Same note not found and free voice found */
    if ( (u8Voice != MIDI_DATA_NOT_VALID) && (u8Voice < SYNTH_MAX_NUM_VOICE) )
    {
        if ( bYM2612_set_note(u8Voice, u8Note) )
        {
            vYM2612_key_on(u8Voice);

            /* Update control structure */
            xSynthDevHandler.xCtrlPoly.xVoice[u8Voice].u8Note = u8Note;
            xSynthDevHandler.xCtrlPoly.xVoice[u8Voice].u8Velocity = u8Velocity;

#ifdef SYNTH_DBG_VERBOSE
            vCliPrintf(SYNTH_TASK_NAME, "Key  ON : %02d - %03d", u8Voice, u8Note);
#endif
        }
    }
    /* Not free voice found, save voice on temporal voice */
    else
    {
        xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Note = u8Note;
        xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Velocity = u8Velocity;
    }
}

static void vHandleVoicePolyOff(uint8_t u8Note, uint8_t u8Velocity)
{
    uint8_t u8Voice = MIDI_DATA_NOT_VALID;

    /* Check if note is already active */
    for ( uint32_t u32IndexVoice = 0U; u32IndexVoice < SYNTH_MAX_NUM_VOICE; u32IndexVoice++ )
    {
        if ( xSynthDevHandler.xCtrlPoly.xVoice[u32IndexVoice].u8Note == u8Note )
        {
            /* Clear channel */
            vYM2612_key_off(u32IndexVoice);

#ifdef SYNTH_DBG_VERBOSE
            vCliPrintf(SYNTH_TASK_NAME, "Key  OFF: %02d - %03d", u32IndexVoice, u8Note);
#endif

            xSynthDevHandler.xCtrlPoly.xVoice[u32IndexVoice].u8Note = MIDI_DATA_NOT_VALID;
            xSynthDevHandler.xCtrlPoly.xVoice[u32IndexVoice].u8Velocity = MIDI_DATA_NOT_VALID;

            /* Save slot cleared one time */
            if (u8Voice == MIDI_DATA_NOT_VALID)
            {
                u8Voice = u32IndexVoice;
            }
        }
    }

    /* Check tmp voice */
    if ( xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Note == u8Note )
    {
        xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Velocity = MIDI_DATA_NOT_VALID;
    }

    /* If there are a free voice, load tmp note */
    if ( (u8Voice != MIDI_DATA_NOT_VALID) && (u8Voice < SYNTH_MAX_NUM_VOICE) && (xSynthDevHandler.xCtrlPoly.xVoiceTmp.u8Note != MIDI_DATA_NOT_VALID) )
    {
        /* Update control structure */
        uint8_t u8TmpNote = xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note;
        uint8_t u8TmpVelocity = xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity;

        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Note = MIDI_DATA_NOT_VALID;
        xSynthDevHandler.xCtrlMono.xVoiceTmp[u8Voice].u8Velocity = MIDI_DATA_NOT_VALID;

        /* Generate new note */
        vHandleVoicePolyOn(u8TmpNote, u8TmpVelocity);
    }
}

static void vSynthTaskMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay( pdMS_TO_TICKS(SYNTH_TASK_INIT_DELAY) );

    /* Show init msg */
    vCliPrintf(SYNTH_TASK_NAME, "Init");

    /* Init YM2612 resources */
    (void)xYM2612_init();

    /* Clear and init all voices */
    vCmdVoiceOffAll();

    /* Init user preset */
    if ( !bInitUserPreset() )
    {
        vCliPrintf(SYNTH_TASK_NAME, "User preset init ERROR");
    }

    /* Basic register init */
    (void)bInitPreset();

    for(;;)
    {
        SynthCmd_t xSynthCmd = { 0U };

        if (xQueueReceive(xSynthEventQueueHandle, &xSynthCmd, portMAX_DELAY) == pdPASS)
        {
#ifdef SYNTH_DBG_VERBOSE
            vCliPrintf(SYNTH_TASK_NAME, "Synth CMD: x%02X", xSynthCmd.eCmd);
#endif
            switch (xSynthCmd.eCmd)
            {
                case SYNTH_CMD_VOICE_UPDATE_MONO:
                    vHandleCmdVoiceUpdateMono(&xSynthCmd.uPayload.xVoiceUpdateMono);
                    break;

                case SYNTH_CMD_VOICE_UPDATE_POLY:
                    vHandleCmdVoiceUpdatePoly(&xSynthCmd.uPayload.xVoiceUpdatePoly);
                    break;

                case SYNTH_CMD_PARAM_UPDATE:
                    vHandleCmdParameterUpdate(&xSynthCmd.uPayload.xParamUpdate);
                    break;

                case SYNTH_CMD_PRESET_UPDATE:
                    vHandleCmdPresetUpdate(&xSynthCmd.uPayload.xPresetUpdate);
                    break;

                case SYNTH_CMD_VOICE_MUTE:
                    vCmdVoiceOffAll();
                    break;

                default:
                    vCliPrintf(SYNTH_TASK_NAME, "Not defined command: x%02X", xSynthCmd.eCmd);
                    break;
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

void vSynthTaskInit(void)
{
    /* Create task */
    xTaskCreate(vSynthTaskMain, SYNTH_TASK_NAME, SYNTH_TASK_STACK, NULL, SYNTH_TASK_PRIO, &xSynthTaskHandle);
    ERR_ASSERT(xSynthTaskHandle);

    /* Create task queue */
    xSynthEventQueueHandle = xQueueCreate(SYNTH_EVENT_QUEUE_SIZE, SYNTH_EVENT_QUEUE_ELEMENT_SIZE);
    ERR_ASSERT(xSynthEventQueueHandle);
}

bool bSynthSendCmd(SynthCmd_t xSynthCmd)
{
    bool bRetval = false;

    if ( xSynthEventQueueHandle != NULL )
    {
        if ( xQueueSend(xSynthEventQueueHandle, &xSynthCmd, 0U) == pdPASS )
        {
            bRetval = true;
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "CMD: Queue Error");
        }
    }

    return bRetval;
}

SynthParam_t xSynthGetParam(uint8_t u8ParamId)
{
    SynthParam_t xRetParam = { 0U };
    return xRetParam;
}

/* EOF */