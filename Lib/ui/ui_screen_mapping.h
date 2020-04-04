/**
  ******************************************************************************
  * @file           : ui_screen_mapping.h
  * @brief          : UI definition for mapping screen.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SCREEN_MAPPING_H
#define __UI_SCREEN_MAPPING_H

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
ui_status_t UI_screen_map_init(ui_screen_t * pxScreenHandler);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SCREEN_MAPPING_H */

/*****END OF FILE****/
