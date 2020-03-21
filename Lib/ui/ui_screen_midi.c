/**
  ******************************************************************************
  * @file           : ui_screen_midi.c
  * @brief          : UI definition for midi screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_midi.h"
#include "printf.h"
#include "ui_task.h"
#include "cli_task.h"

#include "ui_menu_main.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for Test screen */
typedef enum
{
    MIDI_SCREEN_ELEMENT_MODE = 0,
    MIDI_SCREEN_ELEMENT_CHANNEL,
    MIDI_SCREEN_ELEMENT_BANK,
    MIDI_SCREEN_ELEMENT_PROGRAM,
    MIDI_SCREEN_ELEMENT_RETURN,
    MIDI_SCREEN_LAST_ELEMENT,
} eMidiScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max num of elements */
#define UI_NUM_ELEMENT                  (8U)

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)

#define NAME_FORMAT_MODE                "MODE      %s"
#define NAME_FORMAT_CHANNEL             "CH           %01d"
#define NAME_FORMAT_BANK                "BANK         %01d"
#define NAME_FORMAT_PROGRAM             "PROGRAM      %01d"
#define NAME_FORMAT_RETURN              "RETURN"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenMidiName[MAX_LEN_NAME] = "MIDI";

ui_element_t xMidiScreenElementList[MIDI_SCREEN_LAST_ELEMENT];

char pcMidiModeName[MAX_LEN_NAME] = {0};
char pcMidiChannelName[MAX_LEN_NAME] = {0};
char pcMidiBankName[MAX_LEN_NAME] = {0};
char pcMidiProgramName[MAX_LEN_NAME] = {0};
char pcMidiReturn[MAX_LEN_NAME] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Eender function */
static void vScreenMidiRender(void * pvDisplay, void * pvScreen);
static void vElementModeRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementChannelRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementBankRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementProgramRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementReturnRender(void * pvDisplay, void * pvScreen, void * pvElement);

/* Action function */
static void vScreenMidiAction(void * pvMenu, void * pvEventData);
static void vElementModeAction(void * pvMenu, void * pvEventData);
static void vElementChannelAction(void * pvMenu, void * pvEventData);
static void vElementBankAction(void * pvMenu, void * pvEventData);
static void vElementProgramAction(void * pvMenu, void * pvEventData);
static void vElementReturnAction(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* Render functions ----------------------------------------------------------------------*/

static void vScreenMidiRender(void * pvDisplay, void * pvScreen)
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
        u8g2_SetFontMode(pxDisplayHandler, 1U);
        u8g2_SetDrawColor(pxDisplayHandler, 2U);
        u8g2_SetFont(pxDisplayHandler, u8g2_font_amstrad_cpc_extended_8r);

        /* Draw actions on screen */
        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxScreen->pcName);
        u8g2_DrawHLine(pxDisplayHandler, u8LineX, u8LineY, u8LineWith);
    }
}

static void vElementModeRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            /* Get data to print */

            /* Prepare data on buffer */
            sprintf(pxElement->pcName, NAME_FORMAT_MODE, (uint8_t)0U);

            /* Print selection ico */
            vUI_MISC_DrawSelectionBox(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementChannelRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_CHANNEL, 0U);

            /* Print selection ico */
            vUI_MISC_DrawSelectionBox(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementBankRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_BANK, 0U);

            /* Print selection ico */
            vUI_MISC_DrawSelectionBox(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementProgramRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_PROGRAM, 0U);

            /* Print selection ico */
            vUI_MISC_DrawSelectionBox(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
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
            vUI_MISC_DrawSelectionBox(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}


/* Actions functions ----------------------------------------------------------------------*/


static void vScreenMidiAction(void * pvMenu, void * pvEventData)
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

static void vElementModeAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
    }
}

static void vElementChannelAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
    }
}

static void vElementBankAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
    }
}

static void vElementProgramAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
    }
}

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

ui_status_t UI_screen_midi_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenMidiName;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xMidiScreenElementList;
        pxScreenHandler->u32ElementNumber = MIDI_SCREEN_LAST_ELEMENT;
        pxScreenHandler->render_cb = vScreenMidiRender;
        pxScreenHandler->action_cb = vScreenMidiAction;
        pxScreenHandler->bElementSelection = false;

        /* Init name var */
        sprintf(pcMidiModeName, NAME_FORMAT_MODE, "None");
        sprintf(pcMidiChannelName, NAME_FORMAT_CHANNEL, 0U);
        sprintf(pcMidiBankName, NAME_FORMAT_BANK, 0U);
        sprintf(pcMidiProgramName, NAME_FORMAT_PROGRAM, 0U);
        sprintf(pcMidiReturn, NAME_FORMAT_RETURN);

        /* Init elements */
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_MODE].pcName = pcMidiModeName;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_MODE].u32Index = MIDI_SCREEN_ELEMENT_MODE;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_MODE].render_cb = vElementModeRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_MODE].action_cb = vElementModeAction;

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_CHANNEL].pcName = pcMidiChannelName;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_CHANNEL].u32Index = MIDI_SCREEN_ELEMENT_CHANNEL;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_CHANNEL].render_cb = vElementChannelRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_CHANNEL].action_cb = vElementChannelAction;

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_BANK].pcName = pcMidiBankName;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_BANK].u32Index = MIDI_SCREEN_ELEMENT_BANK;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_BANK].render_cb = vElementBankRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_BANK].action_cb = vElementBankAction;

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_PROGRAM].pcName = pcMidiProgramName;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_PROGRAM].u32Index = MIDI_SCREEN_ELEMENT_PROGRAM;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_PROGRAM].render_cb = vElementProgramRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_PROGRAM].action_cb = vElementProgramAction;

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].pcName = pcMidiReturn;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].u32Index = MIDI_SCREEN_ELEMENT_RETURN;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].render_cb = vElementReturnRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].action_cb = vElementReturnAction;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
