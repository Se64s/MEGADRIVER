/**
  ******************************************************************************
  * @file           : ui_sys.h
  * @brief          : UI system to handle display menus
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SYS_H
#define __UI_SYS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

#include "u8g2.h"

/* Private defines -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Operation status */
typedef enum 
{
    UI_STATUS_ERROR = 0U,
    UI_STATUS_OK = 1U,
    UI_STATUS_BUSY = 2U,
    UI_STATUS_NOTDEF = 0xFFU,
} ui_status_t;

/* Operation status */
typedef enum 
{
    UI_ELEMENT_TYPE_TEXT = 0U,
    UI_ELEMENT_TYPE_BAR,
    UI_ELEMENT_TYPE_NOTDEF = 0xFFU,
} ui_element_type_t;

/* Callback definitions */
typedef void (* ui_screen_render_cb)(void * pvDisplay, void * pvScreen);
typedef void (* ui_element_render_cb)(void * pvDisplay, void * pvScreen, void * pvElement);
typedef void (* ui_screen_action_cb)(void * pvMenu, void * pvEventData);
typedef void (* ui_element_action_cb)(void * pvMenu, void * pvEventData);

/* Element screen definition */
typedef struct
{
    char * pcName;
    uint32_t u32Index;
    ui_element_render_cb render_cb;
    ui_element_action_cb action_cb;
} ui_element_t;

/* Screen structure definition */
typedef struct
{
    const char * pcName;
    uint32_t u32ElementSelectionIndex;
    uint32_t u32ElementRenderIndex;
    uint32_t u32ElementNumber;
    ui_element_t * pxElementList;
    ui_screen_render_cb render_cb;
    ui_screen_action_cb action_cb;
    bool bElementSelection;
} ui_screen_t;

/* Screen menu definition */
typedef struct
{
    char * pcName;
    uint32_t u32ScreenSelectionIndex;
    ui_screen_t * pxScreenList;
} ui_menu_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Init resources for ui menu.
  * @param  pxMenuHandler menu handler to use.
  * @retval Operation status.
*/
ui_status_t UI_init(ui_menu_t * pxMenuHandler);

/**
  * @brief  Render menu image.
  * @param  pxIniDisplayHandler.
  * @param  pxMenuHandler.
  * @retval Operation status.
  */
ui_status_t UI_render(u8g2_t * pxIniDisplayHandler, ui_menu_t * pxMenuHandler);

/**
  * @brief  Handle actions on menu.
  * @param  pxMenuHandler menu handler to use.
  * @param  pvEvent pointer to event to be handled by ui.
  * @retval Operation status.
  */
void UI_action(ui_menu_t * pxMenuHandler, void * pvEvent);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SYS_H */

/*****END OF FILE****/
