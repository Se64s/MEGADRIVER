/**
  ******************************************************************************
  * @file           : ui_sys_misc.h
  * @brief          : Common helper functions.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_SYS_MISC_H
#define __UI_SYS_MISC_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "ui_sys.h"

/* Private defines -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define UI_OUT_FROM_SCREEN              (0xFFFFFFFFU)
#define NUM_RENDER_ELEMENT              (5U)

#define UI_OFFSET_SCREEN_X              (0U)
#define UI_OFFSET_SCREEN_Y              (10U)
#define UI_OFFSET_LINE_SCREEN_Y         (2U)

#define UI_OFFSET_ELEMENT_X             (0U)
#define UI_OFFSET_ELEMENT_Y             (18U)
#define UI_OFFSET_INTRA_ELEMENT_Y       (2U)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Check and draw a selection indicator in given index and position.
  * @param  pxDisplayHandler ui display hamdler.
  * @param  pxScreen screen control element.
  * @param  u8ElementIndex index of screen to draw.
  * @param  u8SelectBoxY Y coordinate to draw selection box.
  * @retval None.
*/
void vUI_MISC_DrawSelection(u8g2_t * pxDisplayHandler, ui_screen_t * pxScreen, uint8_t u8ElementIndex, uint8_t u8SelectBoxY);

/**
  * @brief  Get Y coordinate from given element.
  * @param  pxDisplayHandler ui display hamdler.
  * @param  u32RenderIndex Index of current render position.
  * @param  u32ElementIndex index of element to get Y coordinate.
  * @retval Y coordinate.
*/
uint32_t u32UI_MISC_GetDrawIndexY(u8g2_t * pxDisplayHandler, uint32_t u32RenderIndex, uint32_t u32ElementIndex);

/**
  * @brief  Handle encoder action.
  * @param  pxMenu ui menu handler.
  * @param  pvEventData pointer to event data.
  * @retval None.
*/
void vUI_MISC_EncoderAction(ui_menu_t * pxMenu, void * pvEventData);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SYS_MISC_H */

/*****END OF FILE****/
