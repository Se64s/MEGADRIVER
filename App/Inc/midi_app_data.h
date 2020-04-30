/**
 * @file midi_app_data.h
 * @author Sebastian Del Moral Gallardo.
 * @brief Basic library to handle midi app data layout.
 *
 */

#ifndef __MIDI_FLASH_DATA_H
#define __MIDI_FLASH_DATA_H

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "app_data_handler.h"
#include "midi_task.h"

/* Exported defines ----------------------------------------------------------*/

/** Padding bytes for 8B alligment */
#define MIDI_APP_DATA_SPARE_SIZE    (12U)

/** Init page for midi data  */
/* WARNING: Change linker file to limit the user data area */
#define MIDI_APP_DATA_INIT_PAGE     (54U)

/** Init page for midi data  */
#define MIDI_APP_DATA_NUM_PAGES     (1U)

/** Default values */
#define MIDI_APP_DATA_DEFAULT_MODE   (MidiMode3)
#define MIDI_APP_DATA_DEFAULT_CH     (0U)
#define MIDI_APP_DATA_DEFAULT_BANK   (0U)
#define MIDI_APP_DATA_DEFAULT_PROG   (0U)

/* Exported types  -----------------------------------------------------------*/

/** Defined program data */
typedef struct __packed midi_app_data
{
    uint8_t u8Mode;
    uint8_t u8BaseChannel;
    uint8_t u8Bank;
    uint8_t u8Program;
    uint8_t pu8Spare[MIDI_APP_DATA_SPARE_SIZE];
} midi_app_data_t;
/** Final size: 1 + 1 + 1 + 1 + 12 = 16B, 8B alligned */

/* Exported variables --------------------------------------------------------*/

/**
  * @brief Init midi app data.
  * @retval true app data initiated, false app data not initiated.
  */
bool bMIDI_APP_DATA_init(void);

/**
  * @brief Write element into flash.
  * @param pxMidiData pointer with data to save.
  * @retval true data saved, false ioc.
  */
bool bMIDI_APP_DATA_write(midi_app_data_t * pxMidiData);

/**
  * @brief Get element from memory layout.
  * @retval Pointer to midi persistent data, null if peristent data not init.
  */
const midi_app_data_t * pxMIDI_APP_DATA_read(void);

/* Exported functions --------------------------------------------------------*/

#ifdef  __cplusplus
}
#endif

#endif /* __MIDI_FLASH_DATA_H */
