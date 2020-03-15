/**
 * @file    app_data_handler.h
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Handler to managey flash operations with wear leveling.
 *
 */

#ifndef __APP_DATA_HAND_H
#define __APP_DATA_HAND_H

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "flash_driver.h"
#include <stdbool.h>

/* Exported defines ----------------------------------------------------------*/

/** Define use of RTOS */
#define APP_DATA_USE_RTOS

/** App data number of pages defined */
#define APP_DATA_NUM_PAGES          (10U)

/** New flash layout */
#define APP_DATA_8_NO_INIT          (0xFF)
#define APP_DATA_16_NO_INIT         (0xFFFF)
#define APP_DATA_32_NO_INIT         (0xFFFFFFFF)
#define APP_DATA_64_NO_INIT         (0xFFFFFFFFFFFFFFFF)

/* Exported types ------------------------------------------------------------*/

/** Status retval definition */
typedef enum
{
    APP_DATA_ERROR = 0U,
    APP_DATA_OK = 1U,
    APP_DATA_NOT_INIT = 2U,
    APP_DATA_NOT_DEF = 255U
} app_data_status_t;

/** Layout control structure */
typedef struct
{
    uint32_t u32InitPage;
    uint32_t u32NumPages;
    uint32_t u32ElementSize;
    uint32_t u32ElementIndex;
    uint32_t u32PageIndex;
} app_data_layout_t;

/* Exported defines ----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief Initiate flash zone to contain a flash layout.
  * @param pxLayout Control structure with layout parameters.
  * @retval Result.
  */
app_data_status_t xAPP_DATA_init_layout(app_data_layout_t * pxLayout);

/**
  * @brief Clear flash layout.
  * @param pxLayout Control structure with layout parameters.
  * @retval Result.
  */
app_data_status_t xAPP_DATA_clear_layout(app_data_layout_t * pxLayout);

/**
  * @brief Save element into memory layout.
  * @param pxLayout Control structure with layout parameters.
  * @param pvData Pointer of data to handle.
  * @retval Result.
  */
app_data_status_t xAPP_DATA_save_element(app_data_layout_t * pxLayout, void * pvData);

/**
  * @brief Get element from memory layout.
  * @param pxLayout Control structure with layout parameters.
  * @retval pointer to data.
  */
const void * pvAPP_DATA_get_element(app_data_layout_t * pxLayout);

#ifdef  __cplusplus
}
#endif

#endif /* __APP_DATA_HAND_H */
