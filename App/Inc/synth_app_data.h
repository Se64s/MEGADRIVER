/**
 * @file    synth_app_data.h
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Basic library to handle synth app data layout.
 *
 */

#ifndef __SYNTH_APP_DATA_H
#define __SYNTH_APP_DATA_H

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "app_data_handler.h"
#include "YM2612_driver.h"

/* Exported defines ----------------------------------------------------------*/

/** Init page for midi data  */
/* WARNING: Change linker file to limit the user data area */
#define SYNTH_APP_DATA_PRESET_INIT_PAGE     (55U)

/** Init page for synth data  */
#define SYNTH_APP_PAGE_PER_PRESET           (1U)

/** Init page for midi data  */
#define SYNTH_APP_DATA_NUM_PRESETS          (8U)

/** Init Len name preset data */
#define SYNTH_APP_DATA_LEN_PRESET_NAME      (16U)

/* Exported types  -----------------------------------------------------------*/

/** Defined program data */
typedef struct __packed synth_app_data
{
    uint8_t pu8Name[SYNTH_APP_DATA_LEN_PRESET_NAME];
    xFmDevice_t xPresetData;
} synth_app_data_t;
/** Final size: 16 + 296 = 312B, 8B alligned */

/* Exported variables --------------------------------------------------------*/

/**
  * @brief Init app data.
  * @retval true app data initiated, false app data not initiated.
  */
bool bSYNTH_APP_DATA_init(void);

/**
  * @brief Save preset data.
  * @param u8PresetId Preset position.
  * @param pxPresetData pointer with data to save.
  * @retval true data saved, false ioc.
  */
bool bSYNTH_APP_DATA_write(uint8_t u8PresetId, synth_app_data_t * pxPresetData);

/**
  * @brief  Get preset data from flash.
  * @param  u8PresetId Preset position.
  * @retval Pointer to const data with app data, NULL if data not found.
  */
const synth_app_data_t * pxSYNTH_APP_DATA_read(uint8_t u8PresetId);

#ifdef  __cplusplus
}
#endif

#endif /* __SYNTH_APP_DATA_H */
