/**
  ******************************************************************************
  * @file           : spi_driver.h
  * @brief          : Driver to handle spi peripheral
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_DRIVER_H
#define __SPI_DRIVER_H

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
    SPI_0 = 0U,
    SPI_NOTDEF = 0xFFU,
} spi_port_t;

/* Operation status */
typedef enum
{
    SPI_STATUS_ERROR = 0U,
    SPI_STATUS_OK = 1U,
    SPI_STATUS_BUSY = 2U,
    SPI_STATUS_NOTDEF = 0xFFU,
} spi_status_t;

/* spi event */
typedef enum
{
    SPI_EVENT_TX_DONE = 0U,
    SPI_EVENT_ERROR,
    SPI_EVENT_NOTDEF = 0xFF,
} spi_event_t;

/* Data reception callback */
typedef void (* spi_event_cb)(spi_event_t event);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialization of SPI interface
  * @param  dev interface number to init
  * @param  event_cb callback to indicate events on interface
  * @retval Operation status
*/
spi_status_t SPI_init(spi_port_t dev, spi_event_cb event_cb);

/**
  * @brief  De-initialization of spi interface
  * @param  dev spi interface number to deinit
  * @retval Operation status
  */
spi_status_t SPI_deinit(spi_port_t dev);

/**
  * @brief  Send data through defined interface
  * @param  dev spi interface number to use
  * @param  pdata pointer of data to send
  * @param  len number of bytes to send
  * @retval Operation status
  */
spi_status_t SPI_send(spi_port_t dev, uint8_t *pdata, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_DRIVER_H */

/*****END OF FILE****/
