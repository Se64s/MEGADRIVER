/**
  ******************************************************************************
  * @file           : ui_sys_misc.c
  * @brief          : Common helper functions.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_sys_misc.h"
#include "ui_task.h"
#include "cli_task.h"
#include "encoder_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

void vUI_MISC_DrawSelection(u8g2_t * pxDisplayHandler, ui_screen_t * pxScreen, uint8_t u8ElementIndex, uint8_t u8SelectBoxY)
{
    if ((pxDisplayHandler != NULL) && (pxScreen != NULL))
    {
        if (pxScreen->u32ElementSelectionIndex == u8ElementIndex)
        {
            uint8_t u8CharPosY = u8SelectBoxY;

            /* If box is selected, mark option */
            if (pxScreen->bElementSelection)
            {
                
                uint8_t u8CharPosX = u8g2_GetDisplayWidth(pxDisplayHandler) - (2U * u8g2_GetMaxCharWidth(pxDisplayHandler));
                u8g2_DrawStr(pxDisplayHandler, u8CharPosX, u8CharPosY, "<<");
            }
            else
            {
                uint8_t u8CharPosX = u8g2_GetDisplayWidth(pxDisplayHandler) - u8g2_GetMaxCharWidth(pxDisplayHandler);
                u8g2_DrawStr(pxDisplayHandler, u8CharPosX, u8CharPosY, "<");
            }
        }
    }
}

uint32_t u32UI_MISC_GetDrawIndexY(u8g2_t * pxDisplayHandler, uint32_t u32RenderIndex, uint32_t u32ElementIndex)
{
    uint32_t u32IndY = 0U;

    if (pxDisplayHandler != NULL)
    {
        /* Compute element offset */
        if (u32RenderIndex <= u32ElementIndex)
        {
            uint32_t u32PrintIndex = u32ElementIndex - u32RenderIndex;
            u32IndY += u8g2_GetMaxCharHeight(pxDisplayHandler);
            u32IndY += u32PrintIndex * (u8g2_GetMaxCharHeight(pxDisplayHandler) + UI_OFFSET_INTRA_ELEMENT_Y);
        }
    }

    return u32IndY;
}

void vUI_MISC_EncoderAction(ui_menu_t * pxMenu, void * pvEventData)
{
    if ((pxMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * p32Event = pvEventData;
        uint32_t u32ScreenIndex = pxMenu->u32ScreenSelectionIndex;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[u32ScreenIndex];

        if (pxScreen != NULL)
        {
            /* Check no element is selected on screen */
            if (!pxScreen->bElementSelection)
            {
                if (RTOS_CHECK_SIGNAL(*p32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    uint32_t enc_count = 0;
                    ENCODER_getCount(ENCODER_ID_0, &enc_count);
#ifdef UI_DBG_VERBOSE
                    vCliPrintf(UI_TASK_NAME, "Encoder  CW event: %d", enc_count);
#endif

                    uint32_t u32ElementIndex = pxScreen->u32ElementSelectionIndex;
                    uint32_t u32RenderIndex = pxScreen->u32ElementRenderIndex;

                    if (u32ElementIndex != (pxScreen->u32ElementNumber - 1))
                    {
                        u32ElementIndex++;
                        pxScreen->u32ElementSelectionIndex = u32ElementIndex;
                    }

                    /* Update render index - DOWN*/
                    if ((u32ElementIndex - u32RenderIndex) >= NUM_RENDER_ELEMENT)
                    {
                        u32RenderIndex++;
                        pxScreen->u32ElementRenderIndex = u32RenderIndex;
                    }
                }
                else if (RTOS_CHECK_SIGNAL(*p32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    uint32_t enc_count = 0;
                    ENCODER_getCount(ENCODER_ID_0, &enc_count);
#ifdef UI_DBG_VERBOSE
                    vCliPrintf(UI_TASK_NAME, "Encoder CCW event: %d", enc_count);
#endif // UI_DBG_VERBOSE

                    uint32_t u32ElementIndex = pxScreen->u32ElementSelectionIndex;
                    uint32_t u32RenderIndex = pxScreen->u32ElementRenderIndex;

                    if (u32ElementIndex != 0U)
                    {
                        u32ElementIndex--;
                        pxScreen->u32ElementSelectionIndex = u32ElementIndex;
                    }

                    /* Update render index - UP*/
                    if (u32ElementIndex < u32RenderIndex)
                    {
                        u32RenderIndex--;
                        pxScreen->u32ElementRenderIndex = u32RenderIndex;
                    }
                }
            }
        }
    }
}

/*****END OF FILE****/
