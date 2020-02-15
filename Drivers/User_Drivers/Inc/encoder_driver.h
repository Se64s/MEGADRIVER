/**
  ******************************************************************************
  * @file           : encoder_driver.c
  * @brief          : Low level driver to manage encoders
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ENCODER_DRIVER_H
#define __ENCODER_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/

/* GPIO definitions */
#define ENCODER_0_ENC_GPIO_PIN      (GPIO_PIN_6 | GPIO_PIN_7)
#define ENCODER_0_ENC_GPIO_PORT     GPIOC
#define ENCODER_0_ENC_GPIO_CLK      __HAL_RCC_GPIOC_CLK_ENABLE

#define ENCODER_0_SW_GPIO_PIN       GPIO_PIN_11
#define ENCODER_0_SW_GPIO_PORT      GPIOA
#define ENCODER_0_SW_GPIO_CLK       __HAL_RCC_GPIOA_CLK_ENABLE

/* Range defines */
#define ENCODER_0_RANGE             255U
#define ENCODER_0_TH                (2U)
#define ENCODER_0_MAX_TH            (ENCODER_0_REF_VALUE + ENCODER_0_TH)
#define ENCODER_0_MIN_TH            (ENCODER_0_REF_VALUE - ENCODER_0_TH)
#define ENCODER_0_REF_VALUE         127U
#define ENCODER_0_VALUE_CW          1U
#define ENCODER_0_VALUE_CCW         0U

/* Exported types ------------------------------------------------------------*/

/* Operation status */
typedef enum
{
    ENCODER_STATUS_ERROR = 0U,
    ENCODER_STATUS_OK = 1U,
    ENCODER_STATUS_NOTDEF = 0xFFU,
} encoder_status_t;

/* Available encoders */
typedef enum
{
    ENCODER_ID_0 = 0U,
    ENCODER_DEV_NOTDEF = 0xFFU,
} encoder_id_t;

/* Switch state */
typedef enum
{
    ENCODER_SW_RESET = 0U,
    ENCODER_SW_SET,
    ENCODER_SW_NOTDEF = 0xFFU,
} encoder_sw_state_t;

/* Encoder defined events */
typedef enum
{
    ENCODER_EVENT_UPDATE = 0U,
    ENCODER_EVENT_SW,
    ENCODER_EVENT_NONE = 0xFFU,
} encoder_event_t;

/* Encoder event callback */
typedef void (* encoder_event_cb)(encoder_event_t event, uint32_t eventData);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialization of hardware resources interface
  * @param  xDevId id to initialise.
  * @retval Operation status
*/
encoder_status_t ENCODER_init(encoder_id_t xDevId, encoder_event_cb xEventCb);

/**
  * @brief  Deinitialization of hardware resources interface
  * @param  xDevId id to deinitialise.
  * @retval Operation status
*/
encoder_status_t ENCODER_deinit(encoder_id_t xDevId);

/**
  * @brief  Get encoder count
  * @param  xDevId id of encoder.
  * @param  pu32CountVal Pointer where store encoder count
  * @retval Operation status
*/
encoder_status_t ENCODER_getCount(encoder_id_t xDevId, uint32_t * pu32CountVal);

/**
  * @brief  Get switch state
  * @param  xDevId id of encoder.
  * @retval Pin status
*/
encoder_sw_state_t ENCODER_getSwState(encoder_id_t xDevId);

/**
  * @brief  Handle encoder count event.
  * @param  xDevId id of encoder.
  * @param  u32EncCount number of counts detected.
  * @retval None
*/
void ENCODER_irqEncHandler(encoder_id_t xDevId, uint32_t u32EncCount);

/**
  * @brief  Handle switch event.
  * @param  xDevId id of encoder.
  * @param  xSwitchState switch state
  * @retval None
*/
void ENCODER_irqSwHandler(encoder_id_t xDevId, encoder_sw_state_t xSwitchState);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_DRIVER_H */

/*****END OF FILE****/
