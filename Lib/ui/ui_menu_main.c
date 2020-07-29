/**
  ******************************************************************************
  * @file           : ui_menu_main.c
  * @brief          : UI definition for synth app main menu.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_menu_main.h"
#include "ui_screen_main.h"
#include "ui_screen_midi.h"
#include "ui_screen_preset.h"
#include "ui_screen_fm.h"
#include "ui_screen_mapping.h"
#include "ui_screen_idle.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Max num of screens */
#define UI_NUM_SCREEN       (MENU_LAST_SCREEN_POSITION)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Ui screen variables */
ui_screen_t xMenuScreenListMenu[UI_NUM_SCREEN] = {0};

char pcMenuMainName[] = "Main Menu";

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

ui_status_t UI_menu_main_init(ui_menu_t * pxMenuHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxMenuHandler != NULL)
    {
        /* Populate menu elements */
        pxMenuHandler->pcName = pcMenuMainName;
        pxMenuHandler->u32ScreenSelectionIndex = 0U;
        pxMenuHandler->pxScreenList = xMenuScreenListMenu;

        /* Populate screen elements */
        UI_screen_main_init(&xMenuScreenListMenu[MENU_MAIN_SCREEN_POSITION]);
        UI_screen_midi_init(&xMenuScreenListMenu[MENU_MIDI_SCREEN_POSITION]);
        UI_screen_preset_init(&xMenuScreenListMenu[MENU_PRESET_SCREEN_POSITION]);
        UI_screen_fm_init(&xMenuScreenListMenu[MENU_FM_SCREEN_POSITION]);
        UI_screen_map_init(&xMenuScreenListMenu[MENU_MAPPING_SCREEN_POSITION]);
        UI_screen_idle_init(&xMenuScreenListMenu[MENU_IDLE_SCREEN_POSITION]);

        retval = UI_STATUS_OK;
    }
    return retval;
}

/*****END OF FILE****/
