/**
 * @file    synth_app_data_const.h
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Constant data definition.
 *
 */

#ifndef __SYNTH_APP_DATA_CONST_H
#define __SYNTH_APP_DATA_CONST_H

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include "YM2612_driver.h"

/* Exported defines ----------------------------------------------------------*/

/** Maximun number of elements to get */
#define SYNTH_APP_DATA_CONST_MAX_NUM_ELEMENTS   (4U)

/* Exported types  -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/**
  * @brief Get pointer to constant data.
  * @param u8PresetId preset id to get.
  * @retval pointer to constant data, NULL if data not available.
  */
xFmDevice_t * pxSYNTH_APP_DATA_CONST_get(uint8_t u8PresetId);

#ifdef  __cplusplus
}
#endif

#endif /* __SYNTH_APP_DATA_H */
