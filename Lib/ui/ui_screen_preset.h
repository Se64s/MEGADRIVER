/**
  ******************************************************************************
  * @file           : ui_screen_preset.h
  * @brief          : UI definition for preset screen.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SCREEN_PRESET_H
#define __UI_SCREEN_PRESET_H

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
 * @brief Init resources for PRESET menu.
 * 
 * @param pxScreenHandler handler to init screen.
 * @return ui_status_t Operation status.
 */
ui_status_t UI_screen_preset_init(ui_screen_t * pxScreenHandler);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SCREEN_PRESET_H */

/*****END OF FILE****/
