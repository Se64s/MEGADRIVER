/**
  ******************************************************************************
  * @file           : ui_menu_test.c
  * @brief          : UI definition for test menu.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_menu_test.h"
#include "ui_screen_test.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Max num of screens */
#define UI_NUM_SCREEN       (1U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Ui screen variables */
ui_screen_t xMenuScreenListTest[UI_NUM_SCREEN] = {0};

char pcMenuTestName[] = "Test Menu";

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

ui_status_t UI_menu_test_init(ui_menu_t * pxMenuHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxMenuHandler != NULL)
    {
        /* Populate menu elements */
        pxMenuHandler->pcName = pcMenuTestName;
        pxMenuHandler->u32ScreenSelectionIndex = 0;
        pxMenuHandler->pxScreenList = xMenuScreenListTest;

        /* Populate screen elements */
        UI_screen_test_init(xMenuScreenListTest);

        retval = UI_STATUS_OK;
    }
    return retval;
}

/*****END OF FILE****/
