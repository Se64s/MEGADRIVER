/**
  ******************************************************************************
  * @file           : serail_driver.h
  * @brief          : Header for serial_driver.c file.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SERIAL_DRIVER_H
#define __SERIAL_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/

/* Data rx buffer size */
#define SERIAL_0_CBUF_SIZE  (310U)
#define SERIAL_0_RX_SIZE    (8U)

#define SERIAL_1_CBUF_SIZE  (32U)
#define SERIAL_1_RX_SIZE    (8U)

/* Exported types ------------------------------------------------------------*/

/* List of serial devices*/
typedef enum
{
    SERIAL_0 = 0U,
    SERIAL_1,
    SERIAL_NODEF = 0xFFU,
} serial_port_t;

/* Operation status */
typedef enum
{
    SERIAL_STATUS_ERROR = 0U,
    SERIAL_STATUS_OK = 1U,
    SERIAL_STATUS_BUSY = 2U,
    SERIAL_STATUS_NODEF = 0xFFU,
} serial_status_t;

/* Serial event */
typedef enum
{
    SERIAL_EVENT_RX_IDLE = 0U,
    SERIAL_EVENT_TX_DONE,
    SERIAL_EVENT_RX_BUF_FULL,
    SERIAL_EVENT_ERROR,
    SERIAL_EVENT_NOTDEF = 0xFF,
} serial_event_t;

/* Data reception callback */
typedef void (* serial_event_cb)(serial_event_t event);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialization of serial interface
  * @param  dev serial interface number to init
  * @param  event_cb callback to indicate data has rrive to the serial interface
  * @retval Operation status
*/
serial_status_t SERIAL_init(serial_port_t dev, serial_event_cb event_cb);

/**
  * @brief  De-initialization of serial interface
  * @param  dev serial interface number to deinit
  * @retval Operation status
  */
serial_status_t SERIAL_deinit(serial_port_t dev);

/**
  * @brief  Send serial data through defined interface
  * @param  dev serial interface number to use
  * @param  pdata pointer of data to send
  * @param  len number of bytes to send
  * @retval Operation status
  */
serial_status_t SERIAL_send(serial_port_t dev, uint8_t *pdata, uint16_t len);

/**
  * @brief  Read data stored on serial buffer
  * @param  dev serial interface number to use
  * @param  pdata pointer where store read data
  * @param  max_len maximun number of bytes to read
  * @retval number of bytes read
  */
uint16_t SERIAL_read(serial_port_t dev, uint8_t *pdata, uint16_t max_len);

/**
  * @brief  Send serial data through defined interface
  * @param  dev serial interface number to use
  * @param  pdata pointer of data to send
  * @param  len number of bytes to send
  * @retval number of bytes in buffer
  */
uint16_t SERIAL_get_read_count(serial_port_t dev);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_DRIVER_H */

/*****END OF FILE****/
