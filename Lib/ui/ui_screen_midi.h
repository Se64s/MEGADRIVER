/**
  ******************************************************************************
  * @file           : ui_screen_midi.h
  * @brief          : UI definition for midi screen.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SCREEN_MIDI_H
#define __UI_SCREEN_MIDI_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "ui_sys.h"
#include "ui_sys_misc.h"

/* Private defines -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init resources for ui menu.
  * @param  pxScreenHandler handler to init screen.
  * @retval Operation status.
*/
ui_status_t UI_screen_midi_init(ui_screen_t * pxScreenHandler);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SCREEN_MIDI_H */

/*****END OF FILE****/
