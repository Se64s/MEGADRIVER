/**
  ******************************************************************************
  * @file           : mapping_task.c
  * @brief          : Task to handle map interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "mapping_task.h"
#include "error.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "synth_task.h"
#include "cli_task.h"

#include "adc_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** Update rate for mapping actions in miliseconds */
#define MAP_TASK_UPDATE_RATE_MS             (25U)

/** Send event timeout */
#define MAP_SEND_EVENT_TIMEOUT              (100U)

/** Number of ADC steps by V_OCT note */
#define ADC_STEPS_BY_NOTE                   (34U)    // (1000mV / 12notes) / (adc_range_mV / adc_num_steps)

/** Minimal value to set gate */
#define ADC_STEPS_GATE_ON                   (900U)   // aprox 3V

/** ADC full range */
#define ADC_FULL_RANGE                      (4096U)

/** ADC 0mV value */
#define ADC_ZERO_VOLT                       (2048U) // ADC_FULL_RANGE / 2

/* Private macro -------------------------------------------------------------*/

/** Check signals */
#define MAP_CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

/** Note from ADC count */
#define NOTE_FROM_ADC(C_ADC)                ( (uint8_t) ( (C_ADC) / ADC_STEPS_BY_NOTE ) )

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

/* Private function prototypes -----------------------------------------------*/

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

static void vMappingModeVoctHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if (ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK)
    {
        if (u16NewVoltage < ADC_ZERO_VOLT)
        {
            uint8_t u8NewNote = NOTE_FROM_ADC(ADC_ZERO_VOLT - u16NewVoltage);
            uint8_t u8OldNote = NOTE_FROM_ADC(ADC_ZERO_VOLT - pxMapChannelCfg->u16Value);

            // Update new note value to synth task
            if (u8NewNote != u8OldNote)
            {
                QueueHandle_t xSynthQueue = pxSynthTaskGetQueue();

                if (xSynthQueue != NULL)
                {
                    SynthEvent_t xSynthEvent = {0U};

                    xSynthEvent.eType = SYNTH_EVENT_CHANGE_NOTE;
                    xSynthEvent.uPayload.xChangeNote.u8Note = u8NewNote;
                    xSynthEvent.uPayload.xChangeNote.u8VoiceId = pxMapChannelCfg->u8Voice;

                    if (xQueueSend(xSynthQueue, &xSynthEvent, pdMS_TO_TICKS(MAP_SEND_EVENT_TIMEOUT)) != pdPASS)
                    {
                        vCliPrintf(MAP_TASK_NAME, "CMD: Queue Error");
                    }
#ifdef MAP_DEBUG
                    else
                    {
                        vCliPrintf(MAP_TASK_NAME, "CMD: V_OCT");
                    }
#endif

                    pxMapChannelCfg->u16Value = u16NewVoltage;
                }
            }
        }
    }
}

static void vMappingModeGateHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if (ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK)
    {
        bool bNewGateStatus = GATE_FROM_ADC(u16NewVoltage);
        bool bOldGateStatus = GATE_FROM_ADC(pxMapChannelCfg->u16Value);

        // Update new note value to synth task
        if (bNewGateStatus != bOldGateStatus)
        {
            QueueHandle_t xSynthQueue = pxSynthTaskGetQueue();

            if (xSynthQueue != NULL)
            {
                SynthEvent_t xSynthEvent = {0U};

                xSynthEvent.eType = SYNTH_EVENT_NOTE_ON_OFF;
                xSynthEvent.uPayload.xNoteOnOff.bGateState = bNewGateStatus;
                xSynthEvent.uPayload.xNoteOnOff.u8VoiceId = pxMapChannelCfg->u8Voice;

                if (xQueueSend(xSynthQueue, &xSynthEvent, pdMS_TO_TICKS(MAP_SEND_EVENT_TIMEOUT)) != pdPASS)
                {
                    vCliPrintf(MAP_TASK_NAME, "CMD: Queue Error");
                }
#ifdef MAP_DEBUG
                else
                {
                    vCliPrintf(MAP_TASK_NAME, "CMD: GATE");
                }
#endif

                pxMapChannelCfg->u16Value = u16NewVoltage;
            }
        }
    }
}

static void vMappingModeParameterHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    ERR_ASSERT(pxMapChannelCfg != NULL);
    ERR_ASSERT(u8MapChannel < MAP_CH_NUM);

    uint16_t u16NewVoltage = 0U;

    if (ADC_get_value(ADC_0, u8MapChannel, &u16NewVoltage) == ADC_STATUS_OK)
    {
        /* Check if new value is different from old one */
        uint8_t u8NewValue = u8GetParamValue(pxMapChannelCfg->u8ParameterId, u16NewVoltage);
        uint8_t u8OldValue = u8GetParamValue(pxMapChannelCfg->u8ParameterId, pxMapChannelCfg->u16Value);

        /* Update new note value to synth task */
        if (u8NewValue != u8OldValue)
        {
            QueueHandle_t xSynthQueue = pxSynthTaskGetQueue();

            if (xSynthQueue != NULL)
            {
                SynthEvent_t xSynthEvent = {0U};

                xSynthEvent.eType = SYNTH_EVENT_MOD_PARAM;
                xSynthEvent.uPayload.xChangeParameter.u8VoiceId = pxMapChannelCfg->u8Voice;
                xSynthEvent.uPayload.xChangeParameter.u8operatorId = pxMapChannelCfg->u8Operator;
                xSynthEvent.uPayload.xChangeParameter.u8ParameterId = pxMapChannelCfg->u8ParameterId;
                xSynthEvent.uPayload.xChangeParameter.u8Value = u8NewValue;

                if (xQueueSend(xSynthQueue, &xSynthEvent, pdMS_TO_TICKS(MAP_SEND_EVENT_TIMEOUT)) != pdPASS)
                {
                    vCliPrintf(MAP_TASK_NAME, "CMD: Queue Error");
                }
#ifdef MAP_DEBUG
                else
                {
                    vCliPrintf(MAP_TASK_NAME, "CMD PARAM: %d-%d", pxMapChannelCfg->u8ParameterId, u8NewValue);
                }
#endif
                pxMapChannelCfg->u16Value = u16NewVoltage;
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
            if (MAP_CHECK_SIGNAL(u32TaskEvent, MAP_SIGNAL_MAPPING_UPDATE))
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

bool bMapTaskInit(void)
{
    bool bRetval = false;

    /* Create task */
    xTaskCreate(vMapMain, MAP_TASK_NAME, MAP_TASK_STACK, NULL, MAP_TASK_PRIO, &map_task_handle);

    /* Create mutex */
    xMappingCfgMutex = xSemaphoreCreateMutex();

    /* Create timer resources */
    xMappingUpdateTimer = xTimerCreate("MappingUpdateTimer", 
                                pdMS_TO_TICKS(MAP_TASK_UPDATE_RATE_MS), 
                                pdTRUE, 
                                (void *)0U, 
                                vMappingUpdateCallback);

    /* Check resources */
    if ((map_task_handle != NULL) && (xMappingCfgMutex != NULL) && (xMappingUpdateTimer != NULL))
    {
        bRetval = true;
    }

    return bRetval;
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

/*****END OF FILE****/
