/**
  ******************************************************************************
  * @file           : adc_driver.c
  * @brief          : Low level driver to manage ADC
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "adc_driver.h"
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* ADC 0 peripheral control variables */
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* Callback handler */
static adc_event_cb adc_0_event_cb = NULL;

/* ADC group regular conversion data (array of data) */
volatile uint16_t pu16AdcConvertedData[ADC_CH_NUM] = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Error handler
  * @retval None
*/
static void __adc_error_handler(void);

/**
  * @brief  Init hardware for ADC 0
  * @retval None
*/
static void __adc_0_low_level_init(void);

/**
  * @brief  Deinit hardware for ADC 0
  * @retval None
*/
static void __adc_0_low_level_deinit(void);

/* Private user code ---------------------------------------------------------*/

static void __adc_error_handler(void)
{
    ERR_ASSERT(0U);
}

static void __adc_0_low_level_init(void)
{
    /* Init ADC */
    ADC_ChannelConfTypeDef sConfig = {0};

    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV64;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.LowPowerAutoPowerOff = DISABLE;
    hadc1.Init.ContinuousConvMode = ENABLE;
    hadc1.Init.NbrOfConversion = ADC_CH_NUM;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_160CYCLES_5;
    hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_160CYCLES_5;
    hadc1.Init.OversamplingMode = DISABLE;
    hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        __adc_error_handler();
    }

    /** Configure Regular Channel 
     */
    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank = ADC_REGULAR_RANK_4;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        __adc_error_handler();
    }

    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = ADC_REGULAR_RANK_3;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        __adc_error_handler();
    }

    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = ADC_REGULAR_RANK_2;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        __adc_error_handler();
    }

    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        __adc_error_handler();
    }

    /* Run the ADC calibration */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        /* Calibration Error */
        __adc_error_handler();
    }
}

static void __adc_0_low_level_deinit(void)
{
    HAL_ADC_DeInit(&hadc1);
}

/* Callback ------------------------------------------------------------------*/

/**
  * @brief  Conversion complete callback in non blocking mode 
  * @param  hadc: ADC handle
  * @note   This example shows a simple way to report end of conversion
  *         and get conversion result. You can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        if (adc_0_event_cb != NULL)
        {
            adc_0_event_cb(ADC_EVENT_UPDATE);
        }
    }
}

/* Public user code ----------------------------------------------------------*/

adc_status_t ADC_init(adc_port_t dev, adc_event_cb event_cb)
{
    adc_status_t xRetval = ADC_STATUS_ERROR;
    if (dev == ADC_0)
    {
        if (event_cb != NULL)
        {
            adc_0_event_cb = event_cb;
        }
        __adc_0_low_level_init();
        xRetval = ADC_STATUS_OK;
    }
    return xRetval;
}

adc_status_t ADC_deinit(adc_port_t dev)
{
    adc_status_t xRetval = ADC_STATUS_ERROR;
    if (dev == ADC_0)
    {
        if (adc_0_event_cb != NULL)
        {
            adc_0_event_cb = NULL;
        }
        __adc_0_low_level_deinit();
        xRetval = ADC_STATUS_OK;
    }
    return xRetval;
}

adc_status_t ADC_start(adc_port_t dev)
{
    adc_status_t xRetval = ADC_STATUS_ERROR;

    if (dev == ADC_0)
    {
        /* Start ADC group regular conversion with DMA */
        if (HAL_ADC_Start_DMA(&hadc1, 
                            (uint32_t *)&pu16AdcConvertedData, 
                            ADC_CH_NUM
                            ) != HAL_OK)
        {
            xRetval = ADC_STATUS_BUSY;
        }
        else
        {
            xRetval = ADC_STATUS_OK;
        }
    }

    return xRetval;
}

adc_status_t ADC_stop(adc_port_t dev)
{
    adc_status_t xRetval = ADC_STATUS_ERROR;

    if (dev == ADC_0)
    {
        /* Start ADC group regular conversion with DMA */
        if (HAL_ADC_Stop_DMA(&hadc1) != HAL_OK)
        {
            __adc_error_handler();
        }

        xRetval = ADC_STATUS_OK;
    }

    return xRetval;
}

adc_status_t ADC_get_value(adc_port_t dev, adc_ch_id_t xChId, uint16_t * pu16AdcData)
{
    adc_status_t xRetval = ADC_STATUS_ERROR;

    if ((dev == ADC_0) && (xChId < ADC_CH_NUM) && (pu16AdcData != NULL))
    {
        uint16_t u16TmpDataValue = pu16AdcConvertedData[xChId];
        *pu16AdcData = u16TmpDataValue;
        xRetval = ADC_STATUS_OK;
    }

    return xRetval;
}

/*****END OF FILE****/

