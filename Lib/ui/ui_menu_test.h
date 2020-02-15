/**
  ******************************************************************************
  * @file           : ui_menu_test.h
  * @brief          : UI definition for test screen.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_MENU_TEST_H
#define __UI_MENU_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "ui_sys.h"

/* Private defines -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init resources for ui menu.
  * @param  pxMenuHandler handler to use on render.
  * @retval Operation status.
*/
ui_status_t UI_menu_test_init(ui_menu_t * pxMenuHandler);

#ifdef __cplusplus
}
#endif

#endif /* __UI_MENU_TEST_H */

/*****END OF FILE****/
