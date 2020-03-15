/**
 * @file    flash_driver.h
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Basic funtionality for use flash write/read.
 *
 */

#ifndef __FLASH_DRIVER_H
#define __FLASH_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"

/* Exported types ------------------------------------------------------------*/

/** Status retval definition */
typedef enum
{
    FLASH_DRIVER_ERROR = 0U,
    FLASH_DRIVER_OK,
    FLASH_DRIVER_BUSY,
    FLASH_DRIVER_NOT_DEF = 255U
} flash_status_t;

/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Init flash interface.
  * @retval Operation status.
  */
flash_status_t xFLASH_init(void);

/**
  * @brief  Deinit flash interface.
  * @retval Operation status.
  */
flash_status_t xFLASH_deinit(void);

/**
  * @brief  Erase page number.
  * @param  u32PageNumber Number of flash page to erase.
  * @retval Operation status.
  */
flash_status_t xFLASH_ErasePage(uint32_t u32PageNumber);

/**
  * @brief  Write data on address.
  * @param  u32Addr Address where store data.
  * @param  u64Data Data to store.
  * @retval Operation status.
  */
flash_status_t xFLASH_WriteDoubleWord(uint32_t u32Addr, uint64_t u64Data);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_DRIVER_H */
