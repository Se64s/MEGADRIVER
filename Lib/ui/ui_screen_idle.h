/**
  ******************************************************************************
  * @file           : ui_screen_idle.h
  * @brief          : UI definition for idle screen.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SCREEN_IDLE_H
#define __UI_SCREEN_IDLE_H

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
  * @brief  Init resources for idle screen.
  * @param  pxScreenHandler handler to init screen.
  * @retval Operation status.
*/
ui_status_t UI_screen_idle_init(ui_screen_t * pxScreenHandler);

/**
 * @brief Set cc data to display
 * @param u8ccId 
 * @param u8CcData 
 */
void vUI_screen_idle_set_cc_data(uint8_t u8ccId, uint8_t u8CcData);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SCREEN_IDLE_H */

/*****END OF FILE****/
