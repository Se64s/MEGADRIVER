/**
  ******************************************************************************
  * @file           : display_driver.h
  * @brief          : Driver to handle display peripheral
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DISPLAY_DRIVER_H
#define __DISPLAY_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/

/* Indicate use of retos */
#define DISPLAY_USE_RTOS

/* Exported types ------------------------------------------------------------*/

/* List of devices*/
typedef enum
{
    DISPLAY_0 = 0U,
    DISPLAY_NOTDEF = 0xFFU,
} display_port_t;

/* Operation status */
typedef enum
{
    DISPLAY_STATUS_ERROR = 0U,
    DISPLAY_STATUS_OK = 1U,
    DISPLAY_STATUS_BUSY = 2U,
    DISPLAY_STATUS_NOTDEF = 0xFFU,
} display_status_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init display.
  * @param  dev interface number to init.
  * @retval Operation status.
*/
display_status_t DISPLAY_init(display_port_t dev);

/**
  * @brief  De-init display.
  * @param  dev interface number to de-init.
  * @retval Operation status.
  */
display_status_t DISPLAY_deinit(display_port_t dev);

/**
  * @brief  Update display image.
  * @param  dev display number to update.
  * @retval Operation status.
  */
display_status_t DISPLAY_update(display_port_t dev);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_DRIVER_H */

/*****END OF FILE****/
