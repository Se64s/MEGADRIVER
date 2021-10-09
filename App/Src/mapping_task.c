/**
  ******************************************************************************
  * @file           : mapping_task.c
  * @brief          : Task to handle map interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "mapping_task.h"
#include "user_error.h"

#include "sys_rtos.h"

#include "synth_task.h"
#include "cli_task.h"

#include "adc_driver.h"
#include "YM2612_driver.h"

#include "app_lfs.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** Update rate for mapping actions in miliseconds */
#define MAP_TASK_UPDATE_RATE_MS             ( 25U )

/** Send event timeout */
#define MAP_SEND_EVENT_TIMEOUT              ( 100U )

/** Minimal value to set gate */
#define ADC_STEPS_GATE_ON                   ( 900U )   // aprox 3V

/** ADC full range */
#define ADC_FULL_RANGE                      ( 4096U )

/** ADC 0mV value */
#define ADC_ZERO_VOLT                       ( 2048U ) // ADC_FULL_RANGE / 2

/* Private macro -------------------------------------------------------------*/

/** Get gate status ADC count */
#define GATE_FROM_ADC(C_ADC)                ( (bool) ( ( C_ADC ) < ADC_STEPS_GATE_ON ) )

/* Private variables ---------------------------------------------------------*/

/** List of mappig elements to handle */
MapElement_t pxMapElementList[MAP_CH_NUM] = {0U};

/** Task handler */
TaskHandle_t map_task_handle = NULL;

/** Timer to handle refresh */
TimerHandle_t xMappingUpdateTimer = NULL;

/** Mutex to protect mapping cfg access */
SemaphoreHandle_t xMappingCfgMutex = NULL;

/** List of values to set via gate mapping */
uint8_t u8TmpNotes[SYNTH_MAX_NUM_VOICE] = { 0U };

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Cast value from ADC to midi note.
 * 
 * @param u16AdcValue adc value.
 * @return uint8_t midi note number.
 */
static uint8_t u8GetNoteFromDac(uint16_t u16AdcValue);

/**
  * @brief Handler for mode CV_OCT.
  * @param u8MapChannel mapping channel to apply configuration.
  * @param pxMapChannelCfg pointer to map element to manage.
  * @retval None.
  */
static void vMappingModeVoctHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg);

/**
  * @brief Handler for mode GATE.
  * @param u8MapChannel mapping channel to apply configuration.
  * @param pxMapChannelCfg pointer to map element to manage.
  * @retval None.
  */
static void vMappingModeGateHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg);

/**
  * @brief Handler for mode PARAMETER.
  * @param u8MapChannel mapping channel to apply configuration.
  * @param pxMapChannelCfg pointer to map element to manage.
  * @retval None.
  */
static void vMappingModeParameterHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg);

/**
  * @brief Get parameter value from ADC count.
  * @param u8ParameterId mapping channel to apply configuration.
  * @param u16AdcValue value of ADC count.
  * @retval Parameter value acording ADC input.
  */
static uint8_t u8GetParamValue(uint8_t u8ParameterId, uint16_t u16AdcValue);

/**
  * @brief Execute an update loop on all channels.
  * @retval None.
  */
static void vMappingUpdateLoop(void);

/**
  * @brief Timer callback to generate update event.
  * @param xTimer handler that generates the event.
  * @retval None.
  */
static void vMappingUpdateCallback(TimerHandle_t xTimer);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vMapMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static uint8_t u8GetNoteFromDac(uint16_t u16AdcValue)
{
    uint8_t u8Note = 0U;

    if ( u16AdcValue <= ADC_ZERO_VOLT )
    {
        uint16_t u16TmpNote = 60U - (15U * u16AdcValue) / 512U;

        /* Check remainder */
        uint16_t u16Remainder = (15U * u16AdcValue) % 512U;
        if ( u16Remainder < 256U )
        {
            u16TmpNote++;
        }

        u8Note = (uint8_t)u16TmpNote;
    }

    return u8Note;
}

static void vMappingModeVoctHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if ( ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK )
    {
        if ( u16NewVoltage < ADC_ZERO_VOLT )
        {
            uint8_t u8NewNote = u8GetNoteFromDac(u16NewVoltage);
            uint8_t u8OldNote = u8GetNoteFromDac(pxMapChannelCfg->u16Value);

            // Update new note value to synth task
            if ( u8NewNote != u8OldNote )
            {
#ifdef MAP_DEBUG
                vCliPrintf(MAP_TASK_NAME, "MAP: V_OCT UPDATE: CH: %d, Note: %d", u8MapChannel, u8NewNote);
#endif
                pxMapChannelCfg->u16Value = u16NewVoltage;
                u8TmpNotes[pxMapChannelCfg->u8Voice] = u8NewNote;
            }
        }
    }
}

static void vMappingModeGateHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if ( ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK )
    {
        bool bNewGateStatus = GATE_FROM_ADC(u16NewVoltage);
        bool bOldGateStatus = GATE_FROM_ADC(pxMapChannelCfg->u16Value);

        // Update new note value to synth task
        if ( bNewGateStatus != bOldGateStatus )
        {
            SynthCmd_t xSynthCmd = {
                .eCmd = SYNTH_CMD_VOICE_UPDATE_MONO,
                .uPayload.xVoiceUpdateMono.u8Note = u8TmpNotes[pxMapChannelCfg->u8Voice],
                .uPayload.xVoiceUpdateMono.u8Velocity = 127U,
                .uPayload.xVoiceUpdateMono.u8VoiceDst = pxMapChannelCfg->u8Voice,
                .uPayload.xVoiceUpdateMono.u8VoiceState = 0U,
            };

            if ( bNewGateStatus == true )
            {
                xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceState = SYNTH_VOICE_STATE_ON;
            }
            else
            {
                xSynthCmd.uPayload.xVoiceUpdateMono.u8VoiceState = SYNTH_VOICE_STATE_OFF;
            }

            if ( bSynthSendCmd(xSynthCmd) == true )
            {
#ifdef MAP_DEBUG
                vCliPrintf(MAP_TASK_NAME, "CMD: GATE");
#endif
                pxMapChannelCfg->u16Value = u16NewVoltage;
            }
            else
            {
                vCliPrintf(MAP_TASK_NAME, "CMD: Queue Error");
            }
        }
    }
}

static void vMappingModeParameterHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if ( ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK )
    {
        /* Check if new value is different from old one */
        uint8_t u8NewValue = u8GetParamValue(pxMapChannelCfg->u8ParameterId, u16NewVoltage);
        uint8_t u8OldValue = u8GetParamValue(pxMapChannelCfg->u8ParameterId, pxMapChannelCfg->u16Value);

        /* Update new note value to synth task */
        if ( u8NewValue != u8OldValue )
        {
            SynthCmd_t xSynthCmd = {
                .eCmd = SYNTH_CMD_PARAM_UPDATE,
                .uPayload.xParamUpdate.u8Id = pxMapChannelCfg->u8ParameterId,
                .uPayload.xParamUpdate.u8Data = u8NewValue,
            };

            if ( bSynthSendCmd(xSynthCmd) == true )
            {
    #ifdef MAP_DEBUG
                vCliPrintf(MAP_TASK_NAME, "CMD PARAM: %d-%d", pxMapChannelCfg->u8ParameterId, u8NewValue);
    #endif
                pxMapChannelCfg->u16Value = u16NewVoltage;
            }
            else
            {
                vCliPrintf(MAP_TASK_NAME, "CMD: Queue Error");
            }
        }
    }
}

static uint8_t u8GetParamValue(uint8_t u8ParameterId, uint16_t u16AdcValue)
{
    ERR_ASSERT(u8ParameterId < FM_VAR_SIZE_NUMBER);

    uint8_t u8ParamValue = 0U;

    switch (u8ParameterId)
    {
    case FM_VAR_LFO_ON:
        /* Map value like a gate */
        if (GATE_FROM_ADC(u16AdcValue))
        {
            u8ParamValue = 1U;
        }
        else
        {
            u8ParamValue = 0U;
        }
        break;

    case FM_VAR_LFO_FREQ:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_LFO_FREQ;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_VOICE_FEEDBACK:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_FEEDBACK;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_VOICE_ALGORITHM:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_ALGORITHM;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_VOICE_AUDIO_OUT:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_VOICE_OUT;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_VOICE_AMP_MOD_SENS:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_AMP_MOD_SENS;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_VOICE_PHA_MOD_SENS:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_PHA_MOD_SENS;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_DETUNE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_DETUNE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_MULTIPLE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_MULTIPLE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_TOTAL_LEVEL:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_TOTAL_LEVEL;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_KEY_SCALE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_KEY_SCALE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_ATTACK_RATE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_ATTACK_RATE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_AMP_MOD:
        /* Map value like a gate */
        if (GATE_FROM_ADC(u16AdcValue))
        {
            u8ParamValue = 1U;
        }
        else
        {
            u8ParamValue = 0U;
        }
        break;

    case FM_VAR_OPERATOR_DECAY_RATE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_DECAY_RATE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_RATE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_SUSTAIN_RATE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_SUSTAIN_LEVEL:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_SUSTAIN_LEVEL;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_RELEASE_RATE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_RELEASE_RATE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    case FM_VAR_OPERATOR_SSG_ENVELOPE:
        {
            uint32_t tmp_param_max_value = MAX_VALUE_SSG_ENVELOPE;
            uint32_t tmp_value = ((ADC_FULL_RANGE - (uint32_t)u16AdcValue) * tmp_param_max_value) / ADC_FULL_RANGE;
            u8ParamValue = (uint8_t)tmp_value;
        }
        break;

    default:
        break;
    }

    return u8ParamValue;
}

static void vMappingUpdateLoop(void)
{
    /* Loop for channels */
    for (uint32_t u8Index = 0U; u8Index < MAP_CH_NUM; u8Index++)
    {
        switch (pxMapElementList[u8Index].xMode)
        {
            case MAP_MODE_V_OCT:
                vMappingModeVoctHandler(u8Index, &pxMapElementList[u8Index]);
                break;

            case MAP_MODE_GATE:
                vMappingModeGateHandler(u8Index, &pxMapElementList[u8Index]);
                break;

            case MAP_MODE_PARAMETER:
                vMappingModeParameterHandler(u8Index, &pxMapElementList[u8Index]);
                break;

            default:
                /* Nothing to handle */
                break;
        }
    }
}

static void vMappingUpdateCallback(TimerHandle_t xTimer)
{
    xTaskNotify(map_task_handle, MAP_SIGNAL_MAPPING_UPDATE, eSetBits);
}

static void vMapMain(void *pvParameters)
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(MAP_TASK_INIT_DELAY));

    /* Init ADC peripheral */
    ADC_init(ADC_0, NULL);

    /* Init ADC conversion */
    ADC_start(ADC_0);

    /* Show init msg */
    vCliPrintf(MAP_TASK_NAME, "Init");

    /* Init update timer */
    xTimerStart(xMappingUpdateTimer, 0U);

    for(;;)
    {
        uint32_t u32TaskEvent = 0U;

        BaseType_t xEventWait = xTaskNotifyWait(0U, 
                                (
                                    MAP_SIGNAL_MAPPING_UPDATE
                                ), 
                                &u32TaskEvent, 
                                portMAX_DELAY);

        if (xEventWait == pdPASS)
        {
            if ( RTOS_CHECK_SIGNAL(u32TaskEvent, MAP_SIGNAL_MAPPING_UPDATE) )
            {
                vMappingUpdateLoop();
            }
            else
            {
                /* Nothing to do here */
            }
        }
    }
}

/* Public fuctions -----------------------------------------------------------*/

MapElement_t xMapGetCfg(uint8_t u8MapId)
{
    ERR_ASSERT(u8MapId < MAP_CH_NUM);

    return pxMapElementList[u8MapId];
}

void vMapSetCfg(uint8_t u8MapId, MapElement_t xMapValue)
{
    ERR_ASSERT(u8MapId < MAP_CH_NUM);

    if (xSemaphoreTake(xMappingCfgMutex, portMAX_DELAY) != pdTRUE)
    {
        ERR_ASSERT(0U);
    }

    pxMapElementList[u8MapId] = xMapValue;

    if (xSemaphoreGive(xMappingCfgMutex) != pdTRUE)
    {
        ERR_ASSERT(0U);
    }

}

void vMapTaskInit(void)
{
    /* Create task */
    xTaskCreate(vMapMain, MAP_TASK_NAME, MAP_TASK_STACK, NULL, MAP_TASK_PRIO, &map_task_handle);
    ERR_ASSERT( map_task_handle );

    /* Create mutex */
    xMappingCfgMutex = xSemaphoreCreateMutex();
    ERR_ASSERT( xMappingCfgMutex );

    /* Create timer resources */
    xMappingUpdateTimer = xTimerCreate("MappingUpdateTimer", 
                                pdMS_TO_TICKS(MAP_TASK_UPDATE_RATE_MS), 
                                pdTRUE, 
                                (void *)0U, 
                                vMappingUpdateCallback);
    ERR_ASSERT( xMappingUpdateTimer );
}

bool bMapTaskNotify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (map_task_handle != NULL)
    {
      xTaskNotify(map_task_handle, u32Event, eSetBits);
      bRetval = true;
    }
    return bRetval;
}

/* EOF */
