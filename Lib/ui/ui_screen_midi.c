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
#include "midi_task.h"
#include "ui_menu_main.h"
#include "encoder_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for Test screen */
typedef enum
{
    MIDI_SCREEN_ELEMENT_MODE = 0,
    MIDI_SCREEN_ELEMENT_CHANNEL,
    MIDI_SCREEN_ELEMENT_BANK,
    MIDI_SCREEN_ELEMENT_PROGRAM,
    MIDI_SCREEN_ELEMENT_SAVE,
    MIDI_SCREEN_ELEMENT_RETURN,
    MIDI_SCREEN_LAST_ELEMENT,
} eMidiScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)
#define MAX_LEN_NAME_SAVE_AUX           (4U)

#define NAME_FORMAT_MODE                "MODE      %s"
#define NAME_FORMAT_CHANNEL             "CH          %02d"
#define NAME_FORMAT_BANK                "BANK        %02d"
#define NAME_FORMAT_PROGRAM             "PROGRAM     %02d"
#define NAME_FORMAT_SAVE                "SAVE        %s"
#define NAME_FORMAT_RETURN              "BACK"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenMidiName[MAX_LEN_NAME] = "MIDI";

ui_element_t xMidiScreenElementList[MIDI_SCREEN_LAST_ELEMENT];

char pcMidiModeName[MAX_LEN_NAME] = {0};
char pcMidiChannelName[MAX_LEN_NAME] = {0};
char pcMidiBankName[MAX_LEN_NAME] = {0};
char pcMidiProgramName[MAX_LEN_NAME] = {0};
char pcMidiSaveName[MAX_LEN_NAME] = {0};
char pcMidiReturn[MAX_LEN_NAME] = {0};

char pcMidiSaveAuxName[MAX_LEN_NAME_SAVE_AUX] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Eender function */
static void vScreenMidiRender(void * pvDisplay, void * pvScreen);
static void vElementModeRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementChannelRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementBankRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementProgramRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementSaveRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementReturnRender(void * pvDisplay, void * pvScreen, void * pvElement);

/* Action function */
static void vScreenMidiAction(void * pvMenu, void * pvEventData);
static void vElementModeAction(void * pvMenu, void * pvEventData);
static void vElementChannelAction(void * pvMenu, void * pvEventData);
static void vElementBankAction(void * pvMenu, void * pvEventData);
static void vElementProgramAction(void * pvMenu, void * pvEventData);
static void vElementSaveAction(void * pvMenu, void * pvEventData);
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
            if (xMidiTaskGetMode() == MidiMode3)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_MODE, "POLY");
            }
            else if (xMidiTaskGetMode() == MidiMode4)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_MODE, "MONO");
            }
            else
            {
                sprintf(pxElement->pcName, NAME_FORMAT_MODE, "NONE");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

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
            sprintf(pxElement->pcName, NAME_FORMAT_CHANNEL, u8MidiTaskGetChannel());

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

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
            sprintf(pxElement->pcName, NAME_FORMAT_BANK, u8MidiTaskGetBank());

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

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
            sprintf(pxElement->pcName, NAME_FORMAT_PROGRAM, u8MidiTaskGetProgram());

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementSaveRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            /* Clear aux string in case of not empty and not selection */
            if (pxScreen->u32ElementSelectionIndex != pxElement->u32Index)
            {
                if (pcMidiSaveAuxName[0U] != 0U)
                {
                    sprintf(pcMidiSaveAuxName, "");
                }
            }

            /* Prepare data on buffer */
            sprintf(pxElement->pcName, NAME_FORMAT_SAVE, pcMidiSaveAuxName);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

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
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

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
        uint32_t * pu32Event = pvEventData;
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        /* Handle encoder events */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (pxScreen->bElementSelection)
            {
                midiMode_t xTmpMode = xMidiTaskGetMode();

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    xTmpMode++;
                    
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    xTmpMode--;
                }

                (void)bMidiTaskSetMode(xTmpMode);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementChannelAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * pu32Event = pvEventData;
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        /* Handle encoder events */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (pxScreen->bElementSelection)
            {
                uint8_t u8Channel = u8MidiTaskGetChannel();

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8Channel != 0U)
                    {
                        u8Channel--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    u8Channel++;
                }

                (void)bMidiTaskSetChannel(u8Channel);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementBankAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * pu32Event = pvEventData;
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        /* Handle encoder events */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (pxScreen->bElementSelection)
            {
                uint8_t u8Bank = u8MidiTaskGetBank();

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8Bank != 0U)
                    {
                        u8Bank--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    u8Bank++;
                }

                (void)bMidiTaskSetBank(u8Bank);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementProgramAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * pu32Event = pvEventData;
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        /* Handle encoder events */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (pxScreen->bElementSelection)
            {
                uint8_t u8Program = u8MidiTaskGetProgram();

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8Program != 0U)
                    {
                        u8Program--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    u8Program++;
                }

                (void)bMidiTaskSetProgram(u8Program);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementSaveAction(void * pvMenu, void * pvEventData)
{
    (void)pvMenu;

    if (pvEventData != NULL)
    {
        uint32_t * pu32EventData = pvEventData;

        if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            /* Set midi screen */
            vCliPrintf(UI_TASK_NAME, "Event Save");

            /* Save current CFG midi screen */
            if (bMidiTaskSaveCfg())
            {
                sprintf(pcMidiSaveAuxName, "OK");
            }
            else
            {
                sprintf(pcMidiSaveAuxName, "ERR");
            }
        }
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

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_SAVE].pcName = pcMidiSaveName;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_SAVE].u32Index = MIDI_SCREEN_ELEMENT_SAVE;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_SAVE].render_cb = vElementSaveRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_SAVE].action_cb = vElementSaveAction;

        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].pcName = pcMidiReturn;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].u32Index = MIDI_SCREEN_ELEMENT_RETURN;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].render_cb = vElementReturnRender;
        xMidiScreenElementList[MIDI_SCREEN_ELEMENT_RETURN].action_cb = vElementReturnAction;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
