/**
  ******************************************************************************
  * @file           : ui_screen_main.c
  * @brief          : UI definition for main screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_main.h"

#include "printf.h"

#include "ui_task.h"
#include "cli_task.h"

#include "ui_menu_main.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for Test screen */
typedef enum
{
    MAIN_SCREEN_ELEMENT_MIDI = 0,
    MAIN_SCREEN_ELEMENT_PRESET,
    MAIN_SCREEN_ELEMENT_FM,
    MAIN_SCREEN_ELEMENT_MAPPING,
    MAIN_SCREEN_LAST_ELEMENT,
} eMainScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max num of elements */
#define UI_NUM_ELEMENT                  (MAIN_SCREEN_LAST_ELEMENT)

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)

#define NAME_FORMAT_MIDI                "MIDI"
#define NAME_FORMAT_PRESET              "PRESET"
#define NAME_FORMAT_FM                  "FM"
#define NAME_FORMAT_MAPPING             "MAPPING"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenMainName[] = "MEGA DRIVER";

ui_element_t xMainScreenElementList[MAIN_SCREEN_LAST_ELEMENT];

char pcMainMidiName[MAX_LEN_NAME] = {0U};
char pcMainPresetName[MAX_LEN_NAME] = {0U};
char pcMainFmName[MAX_LEN_NAME] = {0U};
char pcMainMappingName[MAX_LEN_NAME] = {0U};

/* Private function prototypes -----------------------------------------------*/

/* Render function */
static void vScreenMainRender(void * pvDisplay, void * pvScreen);
static void vElementMidiRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementPresetRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementFmRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementMappingRender(void * pvDisplayHandler, void * pvScreen, void * pvElement);

/* Action function */
static void vScreenMainAction(void * pvMenu, void * pvEventData);
static void vElementMidiAction(void * pvMenu, void * pvEventData);
static void vElementPresetAction(void * pvMenu, void * pvEventData);
static void vElementFmAction(void * pvMenu, void * pvEventData);
// static void vElementMappingAction(void * pvMenu, void * pvEventData);

/* Render functions ----------------------------------------------------------*/

static void vScreenMainRender(void * pvDisplay, void * pvScreen)
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

static void vElementMidiRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_MIDI);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, (uint8_t)pxElement->u32Index, (uint8_t)u32IndY);

            /* Draw string */
            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementPresetRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_PRESET);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, (uint8_t)pxElement->u32Index, (uint8_t)u32IndY);

            /* Draw string */
            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementFmRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_FM);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, (uint8_t)pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementMappingRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_MAPPING);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, (uint8_t)pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}


/* Actions functions -----------------------------------------------------------------------*/


static void vScreenMainAction(void * pvMenu, void * pvEventData)
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
            if (RTOS_CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || RTOS_CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
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

static void vElementMidiAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        uint32_t * pu32EventData = pvEventData;

        if (RTOS_CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Enter in MIDI screen");
            pxMenu->u32ScreenSelectionIndex = MENU_MIDI_SCREEN_POSITION;
        }
    }
}

static void vElementPresetAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        uint32_t * pu32EventData = pvEventData;

        if (RTOS_CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Enter in PRESET screen");
            pxMenu->u32ScreenSelectionIndex = MENU_PRESET_SCREEN_POSITION;
        }
    }
}

static void vElementFmAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        uint32_t * pu32EventData = pvEventData;

        if (RTOS_CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Enter in FM screen");
            pxMenu->u32ScreenSelectionIndex = MENU_FM_SCREEN_POSITION;
        }
    }
}

static void vElementMappingAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        ui_menu_t * pxMenu = pvMenu;
        uint32_t * pu32EventData = pvEventData;

        if (RTOS_CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Enter in MAPPING screen");
            pxMenu->u32ScreenSelectionIndex = MENU_MAPPING_SCREEN_POSITION;
        }
    }
}

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_main_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenMainName;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xMainScreenElementList;
        pxScreenHandler->u32ElementNumber = MAIN_SCREEN_LAST_ELEMENT;
        pxScreenHandler->render_cb = vScreenMainRender;
        pxScreenHandler->action_cb = vScreenMainAction;
        pxScreenHandler->bElementSelection = false;

        /* Init name var */
        sprintf(pcMainMidiName, NAME_FORMAT_MIDI);
        sprintf(pcMainPresetName, NAME_FORMAT_PRESET);
        sprintf(pcMainFmName, NAME_FORMAT_FM);
        sprintf(pcMainMappingName, NAME_FORMAT_MAPPING);

        /* Init elements */
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MIDI].pcName = pcMainMidiName;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MIDI].u32Index = MAIN_SCREEN_ELEMENT_MIDI;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MIDI].render_cb = vElementMidiRender;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MIDI].action_cb = vElementMidiAction;

        xMainScreenElementList[MAIN_SCREEN_ELEMENT_PRESET].pcName = pcMainPresetName;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_PRESET].u32Index = MAIN_SCREEN_ELEMENT_PRESET;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_PRESET].render_cb = vElementPresetRender;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_PRESET].action_cb = vElementPresetAction;

        xMainScreenElementList[MAIN_SCREEN_ELEMENT_FM].pcName = pcMainFmName;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_FM].u32Index = MAIN_SCREEN_ELEMENT_FM;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_FM].render_cb = vElementFmRender;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_FM].action_cb = vElementFmAction;

        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MAPPING].pcName = pcMainMappingName;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MAPPING].u32Index = MAIN_SCREEN_ELEMENT_MAPPING;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MAPPING].render_cb = vElementMappingRender;
        xMainScreenElementList[MAIN_SCREEN_ELEMENT_MAPPING].action_cb = vElementMappingAction;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
