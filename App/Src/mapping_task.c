/**
  ******************************************************************************
  * @file           : mapping_task.c
  * @brief          : Task to handle map interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "mapping_task.h"
#include "cli_task.h"

#include "adc_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** Update rate for mapping actions in miliseconds */
#define MAP_TASK_UPDATE_RATE_MS             (500U)

/** Number of elements to map */
#define MAP_MAPPING_SIZE_LIST               (ADC_CH_NUM)

/* Private macro -------------------------------------------------------------*/

/* Check signals */
#define MAP_CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

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
            vCliPrintf(MAP_TASK_NAME, "ADC-%02d: %d", u8MapChannel, u16NewVoltage);
        }
    }
}

static void vMappingModeGateHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
}

static void vMappingModeParameterHandler(uint8_t u8MapChannel, MapElement_t * pxMapChannelCfg)
{
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
