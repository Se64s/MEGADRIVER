/**
 * @file app_lfs.h
 * @author Sebastián Del Moral
 * @brief Application file system implementation.
 * @version 0.1
 * @date 2021-01-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_LFS_H
#define __APP_LFS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "YM2612_driver.h"
#include "synth_app_data_const.h"
#include "midi_lib.h"

/* Private defines -----------------------------------------------------------*/

/** Max size for register configuration name */
#define LFS_YM_CF_NAME_MAX_LEN              ( 16U )

/** Midi cfg default values */
#define LFS_MIDI_CFG_DEFAULT_MODE           ( MidiMode3 )
#define LFS_MIDI_CFG_DEFAULT_CH             ( 0U )
#define LFS_MIDI_CFG_DEFAULT_BANK           ( LFS_MIDI_BANK_ROM )
#define LFS_MIDI_CFG_DEFAULT_PROG           ( 0U )

#define LFS_MIDI_CFG_MAX_BANK               ( LFS_MIDI_BANK_MAX_NUM )
#define LFS_MIDI_CFG_MAX_PROG_BANK_FIX      ( SYNTH_APP_DATA_CONST_MAX_NUM_ELEMENTS )
#define LFS_MIDI_CFG_MAX_PROG_BANK_FLASH    ( LFS_YM_SLOT_NUM )
#define LFS_MIDI_CFG_MAX_PROG_BANK_SD       ( 255U )

/* Exported types ------------------------------------------------------------*/

/** Return codes */
typedef enum
{
    LFS_OK = 0U,
    LFS_ERROR = 1U,
    LFS_NOT_DEF = 255U,
} lfs_status_t;

/** Number of slots for user configs */
typedef enum
{
    LFS_MIDI_BANK_ROM = 0U,
    LFS_MIDI_BANK_FLASH,
    // LFS_MIDI_BANK_SD,
    LFS_MIDI_BANK_MAX_NUM,
} lfs_midi_bank_t;

/** Number of slots for user configs */
typedef enum
{
    LFS_YM_SLOT_0 = 0U,
    LFS_YM_SLOT_1,
    LFS_YM_SLOT_2,
    LFS_YM_SLOT_3,
    LFS_YM_SLOT_4,
    LFS_YM_SLOT_NUM,
} lfs_ym_cfg_slot_t;

/** Midi data structure */
typedef struct lfs_midi_data
{
    uint8_t u8Mode;
    uint8_t u8BaseChannel;
    uint8_t u8Bank;
    uint8_t u8Program;
} lfs_midi_data_t;

/** Defined program data */
typedef struct lfs_ym_cfg
{
    uint8_t pu8Name[LFS_YM_CF_NAME_MAX_LEN];
    xFmDevice_t xPresetData;
} lfs_ym_data_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Init app file system.
 * 
 * @return lfs_status_t 
 */
lfs_status_t LFS_init(void);

/**
 * @brief Read current midi config.
 * 
 * @param pxData pointer where store midi config.
 * @return lfs_status_t operation status.
 */
lfs_status_t LFS_read_midi_data(lfs_midi_data_t *pxData);

/**
 * @brief Write midi config
 * 
 * @param pxData pointer with data to store.
 * @return lfs_status_t operation status.
 */
lfs_status_t LFS_write_midi_data(lfs_midi_data_t *pxData);

/**
 * @brief Read YM config file
 * 
 * @param u8Slot config slot to read. Value must be lower than LFS_YM_MAX_NUM.
 * @param pxData pointer where store data.
 * @return lfs_status_t operation status.
 */
lfs_status_t LFS_read_ym_data(uint8_t u8Slot, lfs_ym_data_t *pxData);

/**
 * @brief Save YM config file
 * 
 * @param u8Slot config slot to write. Value must be lower than LFS_YM_MAX_NUM.
 * @param pxData pointer with data to store.
 * @return lfs_status_t operation status.
 */
lfs_status_t LFS_write_ym_data(uint8_t u8Slot, lfs_ym_data_t *pxData);

#ifdef __cplusplus
}
#endif

#endif /* __APP_LFS_H */

/* EOF */
