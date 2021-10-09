/**
  ******************************************************************************
  * @file           : i2c_driver.h
  * @brief          : Driver to handle i2c peripheral
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_DRIVER_H
#define __I2C_DRIVER_H

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
    I2C_0 = 0U,
    I2C_NOTDEF = 0xFFU,
} i2c_port_t;

/* Operation status */
typedef enum
{
    I2C_STATUS_ERROR = 0U,
    I2C_STATUS_OK = 1U,
    I2C_STATUS_BUSY = 2U,
    I2C_STATUS_NOTDEF = 0xFFU,
} i2c_status_t;

/* I2c event */
typedef enum
{
    I2C_EVENT_MASTER_TX_DONE = 0U,
    I2C_EVENT_MASTER_RX_DONE,
    I2C_EVENT_MASTER_MEM_TX_DONE,
    I2C_EVENT_MASTER_MEM_RX_DONE,
    I2C_EVENT_ERROR,
    I2C_EVENT_NOTDEF = 0xFFU,
} i2c_event_t;

/* Driver event callback */
typedef void (* i2c_event_cb)(i2c_event_t event);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init I2C interface.
  * @param  dev interface number to init.
  * @param  event_cb callback to report events from interface.
  * @retval Operation status.
*/
i2c_status_t I2C_init(i2c_port_t dev, i2c_event_cb event_cb);

/**
  * @brief  De-init I2Cc interface.
  * @param  dev interface number to de-init.
  * @retval Operation status.
  */
i2c_status_t I2C_deinit(i2c_port_t dev);

/**
  * @brief  Send data through defined interface.
  * @param  dev i2c interface to use.
  * @param  i2c_addr slave address.
  * @param  pdata pointer of data to send.
  * @param  len number of bytes to send.
  * @retval Operation status.
  */
i2c_status_t I2C_master_send(i2c_port_t dev, uint16_t i2c_addr, uint8_t *pdata, uint16_t len);

/**
  * @brief  Read data through defined interface.
  * @param  dev i2c interface to use.
  * @param  i2c_addr slave address.
  * @param  pdata pointer where place data.
  * @param  len number of bytes to read.
  * @retval Operation status.
  */
i2c_status_t I2C_master_read(i2c_port_t dev, uint16_t i2c_addr, uint8_t *pdata, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __I2C_DRIVER_H */

/*****END OF FILE****/
