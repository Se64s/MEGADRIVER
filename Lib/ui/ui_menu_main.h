/**
  ******************************************************************************
  * @file           : ui_menu_main.h
  * @brief          : UI definition for main menu.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_MENU_MAIN_H
#define __UI_MENU_MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "ui_sys.h"

/* Private defines -----------------------------------------------------------*/

/* Defined screen order */
typedef enum
{
    MENU_MAIN_SCREEN_POSITION = 0U,
    MENU_MIDI_SCREEN_POSITION,
    MENU_FM_SCREEN_POSITION,
    MENU_MAPPING_SCREEN_POSITION,
    MENU_LAST_SCREEN_POSITION,
} eMenuScreenPosition_t;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init resources for ui menu.
  * @param  pxMenuHandler handler to use on render.
  * @retval Operation status.
*/
ui_status_t UI_menu_main_init(ui_menu_t * pxMenuHandler);

#ifdef __cplusplus
}
#endif

#endif /* __UI_MENU_MAIN_H */

/*****END OF FILE****/
