/**
  ******************************************************************************
  * @file           : ui_sys.c
  * @brief          : UI engine to handle display menus.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_sys.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

ui_status_t UI_init(ui_menu_t * pxMenuHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (UI_menu_test_init(pxMenuHandler) == UI_STATUS_OK)
    {
        retval = UI_STATUS_OK;
    }

    return retval;
}

ui_status_t UI_render(u8g2_t * pxIniDisplayHandler, ui_menu_t * pxMenuHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if ((pxIniDisplayHandler != NULL) && (pxMenuHandler != NULL))
    {
        ui_screen_t * pxScreen;
        uint32_t u32ScreenIndex = pxMenuHandler->u32ScreenSelectionIndex;

        /* Get screen to render */
        pxScreen = &pxMenuHandler->pxScreenList[u32ScreenIndex];

        /* Update display data */
        u8g2_FirstPage(pxIniDisplayHandler);
        do {
            /* Render screen marquee*/
            pxScreen->render_cb(pxIniDisplayHandler, pxScreen);

            /* Render screen elements */
            for (uint32_t u32Index = 0; u32Index < pxScreen->u32ElementNumber; u32Index++)
            {
                ui_element_t * pxElement = &pxScreen->pxElementList[u32Index];
                pxElement->render_cb(pxIniDisplayHandler, pxScreen, pxElement);
            }
        } while (u8g2_NextPage(pxIniDisplayHandler));

        retval = UI_STATUS_OK;
    }

    return retval;
}

void UI_action(ui_menu_t * pxMenuHandler, void * pvEvent)
{

    if ((pxMenuHandler != NULL) && (pvEvent != NULL))
    {
        ui_screen_t * pxScreen;
        uint32_t u32ScreenIndex = pxMenuHandler->u32ScreenSelectionIndex;
        
        /* Get screen to render */
        pxScreen = &pxMenuHandler->pxScreenList[u32ScreenIndex];
        
        /* Send action to elements */
        for (uint32_t u32Index = 0; u32Index < pxScreen->u32ElementNumber; u32Index++)
        {
            ui_element_t * pxElement = &pxScreen->pxElementList[u32Index];

            if (pxElement->action_cb != NULL)
            {
                pxElement->action_cb(pxScreen, pxElement, pvEvent);
            }
        }
    }
}

/*****END OF FILE****/
