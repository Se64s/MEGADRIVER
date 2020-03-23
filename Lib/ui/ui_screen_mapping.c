/**
  ******************************************************************************
  * @file           : ui_screen_mapping.c
  * @brief          : UI definition for mapping screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_mapping.h"
#include "printf.h"
#include "ui_task.h"
#include "cli_task.h"

#include "ui_menu_main.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for FM screen */
typedef enum
{
    MAP_SCREEN_ELEMENT_RETURN = 0U,
    MAP_SCREEN_ELEMENT_LAST
} eMapScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max num of elements */
#define UI_NUM_ELEMENT                  (MAP_SCREEN_ELEMENT_LAST)

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)

#define NAME_FORMAT_RETURN              "RETURN"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenMapName[MAX_LEN_NAME] = "MAPPING";

ui_element_t xElementMapScreenElementList[UI_NUM_ELEMENT];

char pcMapReturnName[MAX_LEN_NAME] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Render functions */
static void vScreenMapRender(void * pvDisplay, void * pvScreen);
static void vElementReturnRender(void * pvDisplay, void * pvScreen, void * pvElement);

/* Actions functions */
static void vScreenMapAction(void * pvMenu, void * pvEventData);
static void vElementReturnAction(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* RENDER --------------------------------------------------------------------*/

static void vScreenMapRender(void * pvDisplay, void * pvScreen)
{
    if ((pvDisplay != NULL) && (pvScreen != NULL))
    {
        u8g2_t * pxDisplayHandler = pvDisplay;
        ui_screen_t * pxScreen = pvScreen;
        uint32_t u32IndX = 0U;
        uint32_t u32IndY = UI_OFFSET_SCREEN_Y;
        uint8_t u8LineX = 0U;
        uint8_t u8LineY = UI_OFFSET_SCREEN_Y + UI_OFFSET_LINE_SCREEN_Y;
        uint8_t u8LineWith = 0U;

        u8LineWith = u8g2_GetDisplayWidth(pxDisplayHandler);

        /* Set font */
        u8g2_SetFontMode(pxDisplayHandler, 1);
        u8g2_SetDrawColor(pxDisplayHandler, 2);
        u8g2_SetFont(pxDisplayHandler, u8g2_font_amstrad_cpc_extended_8r);

        /* Draw actions on screen */
        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxScreen->pcName);
        u8g2_DrawHLine(pxDisplayHandler, u8LineX, u8LineY, u8LineWith);
    }
}

static void vElementReturnRender(void * pvDisplay, void * pvScreen, void * pvElement)
{
    if ((pvDisplay != NULL) && (pvScreen != NULL) && (pvElement != NULL))
    {
        u8g2_t * pxDisplayHandler = pvDisplay;
        ui_screen_t * pxScreen = pvScreen;
        ui_element_t * pxElement = pvElement;
        uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
        uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

        /* Compute element offset */
        u32IndY += u32UI_MISC_GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

        if ((u32IndY < u8g2_GetDisplayHeight(pxDisplayHandler)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
        {

            /* Prepare data on buffer */
            sprintf(pxElement->pcName, NAME_FORMAT_RETURN);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

/* ACTION --------------------------------------------------------------------*/

static void vScreenMapAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        if (pxScreen != NULL)
        {
            /* Check if is a general event */
            uint32_t * pu32Event = pvEventData;
            ui_element_t * pxElement = &pxScreen->pxElementList[pxScreen->u32ElementSelectionIndex];

            /* Handle encoder events */
            if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
            {
                vUI_MISC_EncoderAction(pxMenu, pvEventData);
            }

            /* Handle action for selected element */
            if (pxElement->action_cb != NULL)
            {
                pxElement->action_cb(pxMenu, pvEventData);
            }
        }
    }
}

/* Element action functions */

static void vElementReturnAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        uint32_t * pu32EventData = pvEventData;

        if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Event Return");
            pxMenu->u32ScreenSelectionIndex = MENU_MAIN_SCREEN_POSITION;
        }
    }
}

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_map_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenMapName;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xElementMapScreenElementList;
        pxScreenHandler->u32ElementNumber = MAP_SCREEN_ELEMENT_LAST;
        pxScreenHandler->render_cb = vScreenMapRender;
        pxScreenHandler->action_cb = vScreenMapAction;
        pxScreenHandler->bElementSelection = false;

        /* Init name var */
        sprintf(pcMapReturnName, NAME_FORMAT_RETURN);

        /* Init elements */
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].pcName = pcMapReturnName;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].u32Index = MAP_SCREEN_ELEMENT_RETURN;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].render_cb = vElementReturnRender;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].action_cb = vElementReturnAction;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
