/**
  ******************************************************************************
  * @file           : mapping_task.c
  * @brief          : Task to handle map interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mapping_task.h"
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

/** Number of elements to map */
#define MAP_MAPPING_SIZE_LIST               (ADC_CH_NUM)

/** Number of ADC steps by V_OCT note */
#define ADC_STEPS_BY_NOTE                   (34U)    // (1000mV / 12notes) / (adc_range_mV / adc_num_steps)

/** Minimal value to set gate */
#define ADC_STEPS_GATE_ON                   (900U)   // aprox 3V

/** ADC 0 V value */
#define ADC_ZERO_VOLT                       (2048U) // adc_num_steps / 2

/* Private macro -------------------------------------------------------------*/

/** Check signals */
#define MAP_CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

/** Note from ADC count */
#define NOTE_FROM_ADC(C_ADC)                ( (uint8_t) ( (C_ADC) / ADC_STEPS_BY_NOTE ) )

/** Get gate status ADC count */
#define GATE_FROM_ADC(C_ADC)                ( (bool) ( ( C_ADC ) < ADC_STEPS_GATE_ON ) )

/* Private variables ---------------------------------------------------------*/

/** List of mappig elements to handle */
MapElement_t pxMapElementList[MAP_MAPPING_SIZE_LIST] = {0U};

/** Task handler */
TaskHandle_t map_task_handle = NULL;

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
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vMapMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vMappingModeVoctHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    if ((pxMapChannelCfg != NULL) && (u8MapChannel < MAP_MAPPING_SIZE_LIST))
    {
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
}

static void vMappingModeGateHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    if ((pxMapChannelCfg != NULL) && (u8MapChannel < MAP_MAPPING_SIZE_LIST))
    {
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
}

static void vMappingModeParameterHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
    if ((pxMapChannelCfg != NULL) && (u8MapChannel < MAP_MAPPING_SIZE_LIST))
    {
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
                        vCliPrintf(MAP_TASK_NAME, "CMD: MOD PARAM");
                    }
#endif
                    pxMapChannelCfg->u16Value = u16NewVoltage;
                }
            }
        }
    }
}

static uint8_t u8GetParamValue(uint8_t u8ParameterId, uint16_t u16AdcValue)
{
    uint8_t u8ParamValue = 0U;
    return u8ParamValue;
}

static void vMapMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(MAP_TASK_INIT_DELAY));

    /* Init ADC peripheral */
    ADC_init(ADC_0, NULL);

    /* Init ADC conversion */
    ADC_start(ADC_0);

    /* Show init msg */
    vCliPrintf(MAP_TASK_NAME, "Init");

    /* Test init */
    pxMapElementList[0U].xMode = MAP_MODE_V_OCT;
    pxMapElementList[1U].xMode = MAP_MODE_GATE;
    pxMapElementList[2U].xMode = MAP_MODE_PARAMETER;
    pxMapElementList[3U].xMode = MAP_MODE_NONE;

    for(;;)
    {
        /* Loop for channels */
        for (uint32_t u8Index = 0U; u8Index < MAP_MAPPING_SIZE_LIST; u8Index++)
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

        /* Wait until next processing period */
        vTaskDelay(pdMS_TO_TICKS(MAP_TASK_UPDATE_RATE_MS));
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bMapTaskInit(void)
{
    bool bRetval = false;

    /* Create task */
    xTaskCreate(vMapMain, MAP_TASK_NAME, MAP_TASK_STACK, NULL, MAP_TASK_PRIO, &map_task_handle);

    /* Check resources */
    if (map_task_handle != NULL)
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
