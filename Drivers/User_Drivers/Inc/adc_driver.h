/**
  ******************************************************************************
  * @file           : adc_driver.h
  * @brief          : Driver to handle ADC peripheral
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_DRIVER_H
#define __ADC_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* List of devices*/
typedef enum
{
    ADC_0 = 0U,
    ADC_NOTDEF = 0xFFU,
} adc_port_t;

/* List of adc channels */
typedef enum
{
    ADC_CH0 = 0U,
    ADC_CH1,
    ADC_CH2,
    ADC_CH3,
    ADC_CH_NUM
} adc_ch_id_t;

/* Operation status */
typedef enum
{
    ADC_STATUS_ERROR = 0U,
    ADC_STATUS_OK = 1U,
    ADC_STATUS_BUSY = 2U,
    ADC_STATUS_NOTDEF = 0xFFU,
} adc_status_t;

/* adc events */
typedef enum
{
    ADC_EVENT_UPDATE = 0U,
    ADC_EVENT_NOTDEF = 0xFFU,
} adc_event_t;

/* Driver event callback */
typedef void (* adc_event_cb)(adc_event_t event);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init ADC interface.
  * @param  dev interface number to init.
  * @param  event_cb callback to report events from interface.
  * @retval Operation status.
*/
adc_status_t ADC_init(adc_port_t dev, adc_event_cb event_cb);

/**
  * @brief  De-init ADC interface.
  * @param  dev interface number to de-init.
  * @retval Operation status.
  */
adc_status_t ADC_deinit(adc_port_t dev);

/**
  * @brief  Start adc continuous conversion.
  * @param  dev interface number to init.
  * @retval Operation status.
*/
adc_status_t ADC_start(adc_port_t dev);

/**
  * @brief  Stop adc continuous conversion.
  * @param  dev interface number to init.
  * @retval Operation status.
*/
adc_status_t ADC_stop(adc_port_t dev);

/**
  * @brief  Init ADC interface.
  * @param  dev interface number to init.
  * @param  xChId callback to report events from interface.
  * @param  pu16AdcData pointer where store ADC read.
  * @retval Operation status.
*/
adc_status_t ADC_get_value(adc_port_t dev, adc_ch_id_t xChId, uint16_t * pu16AdcData);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_DRIVER_H */

/*****END OF FILE****/
