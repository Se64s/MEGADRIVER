
/**
 ******************************************************************************
 * @file           : ui_screen_midi.c
 * @brief          : UI definition for midi screen.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_preset.h"
#include "ui_task.h"
#include "ui_menu_main.h"
#include "midi_task.h"
#include "synth_task.h"
#include "cli_task.h"
#include "synth_app_data_const.h"
#include "app_lfs.h"
#include "printf.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for Test screen */
typedef enum
{
    PRESET_SCREEN_ELEMENT_BANK = 0,
    PRESET_SCREEN_ELEMENT_PROGRAM,
    PRESET_SCREEN_ELEMENT_NAME,
    PRESET_SCREEN_ELEMENT_SELECT,
    PRESET_SCREEN_ELEMENT_RETURN,
    PRESET_SCREEN_ELEMENT_LAST_ELEMENT
} ePresetScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)
#define MAX_LEN_NAME_SAVE_AUX           (4U)

/* Format for screen elements */
#define NAME_FORMAT_BANK                "BANK        %02d"
#define NAME_FORMAT_PROGRAM             "PROGRAM     %02d"
#define NAME_FORMAT_NAME                " %s"
#define NAME_FORMAT_SELECT              "SELECT      %s"
#define NAME_FORMAT_RETURN              "BACK"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/** Name for screen */
const char pcScreenPresetName[MAX_LEN_NAME] = "PRESET";

/** List with screen elements */
ui_element_t xPresetScreenElementList[PRESET_SCREEN_ELEMENT_LAST_ELEMENT];

/** String var with element messages */
char pcPresetBankName[MAX_LEN_NAME] = {0};
char pcPresetProgramName[MAX_LEN_NAME] = {0};
char pcPresetNameName[MAX_LEN_NAME] = {0};
char pcPresetSelectName[MAX_LEN_NAME] = {0};
char pcPresetReturnName[MAX_LEN_NAME] = {0};

/** Temporal string to save selection result */
char pcPresetSlectionAuxName[MAX_LEN_NAME] = {0};

/** Current bank selection */
uint8_t u8SelectionBank = 0;

/** Current program selection */
uint8_t u8SelectionProgram = 0;

/* Private function prototypes -----------------------------------------------*/

/* Render function */
static void vScreenPresetRender(void * pvDisplay, void * pvScreen);
static void vElementBankRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementProgramRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementNameRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementSelectRender(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementReturnRender(void * pvDisplay, void * pvScreen, void * pvElement);

/* Action function */
static void vScreenPresetAction(void * pvMenu, void * pvEventData);
static void vElementBankAction(void * pvMenu, void * pvEventData);
static void vElementProgramAction(void * pvMenu, void * pvEventData);
static void vElementNameAction(void * pvMenu, void * pvEventData);
static void vElementSelectAction(void * pvMenu, void * pvEventData);
static void vElementReturnAction(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* Render functions ----------------------------------------------------------------------*/

static void vScreenPresetRender(void * pvDisplay, void * pvScreen)
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
            sprintf(pxElement->pcName, NAME_FORMAT_BANK, u8SelectionBank);

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
            sprintf(pxElement->pcName, NAME_FORMAT_PROGRAM, u8SelectionProgram);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementNameRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            if (u8SelectionBank == MIDI_APP_BANK_USER)
            {
                lfs_ym_data_t xYmData = { 0U };
                LFS_read_ym_data(u8SelectionProgram, &xYmData);
                snprintf(pxElement->pcName, MAX_LEN_NAME - 1U, NAME_FORMAT_NAME, xYmData.pu8Name);
            }
            else
            {
                snprintf(pxElement->pcName, MAX_LEN_NAME - 1U, NAME_FORMAT_NAME, pxSYNTH_APP_DATA_CONST_get_name(u8SelectionProgram));
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementSelectRender(void * pvDisplay, void * pvScreen, void * pvElement)
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
            sprintf(pxElement->pcName, NAME_FORMAT_SELECT, pcPresetSlectionAuxName);

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

static void vScreenPresetAction(void * pvMenu, void * pvEventData)
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
                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (u8SelectionBank < (MIDI_APP_MAX_BANK - 1U))
                    {
                        u8SelectionBank++;
                        u8SelectionProgram = 0;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8SelectionBank != 0)
                    {
                        u8SelectionBank--;
                        u8SelectionProgram = 0;
                    }
                }
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
                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (u8SelectionBank == MIDI_APP_BANK_DEFAULT)
                    {
                        if (u8SelectionProgram < (SYNTH_APP_DATA_CONST_MAX_NUM_ELEMENTS - 1U))
                        {
                            u8SelectionProgram++;
                        }
                    }
                    else if (u8SelectionBank == MIDI_APP_BANK_USER)
                    {
                        if (u8SelectionProgram < (LFS_YM_SLOT_NUM - 1U))
                        {
                            u8SelectionProgram++;
                        }
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8SelectionProgram != 0U)
                    {
                        u8SelectionProgram--;
                    }
                }
            }
        }

        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementNameAction(void * pvMenu, void * pvEventData)
{
    (void)pvMenu;
    (void)pvEventData;
}

static void vElementSelectAction(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * pu32Event = pvEventData;
        (void)pvMenu;

        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            vCliPrintf(UI_TASK_NAME, "Preset selection");
            bool bRetVal = false;

            if (u8SelectionBank < MIDI_APP_MAX_BANK)
            {
                if (u8SelectionBank == MIDI_APP_BANK_DEFAULT)
                {
                    if (u8SelectionProgram < SYNTH_APP_DATA_CONST_MAX_NUM_ELEMENTS)
                    {
                        bRetVal = true;
                    }
                }
                else if (u8SelectionBank == MIDI_APP_BANK_USER)
                {
                    if (u8SelectionProgram < LFS_YM_SLOT_NUM)
                    {
                        bRetVal = true;
                    }
                }
                else
                {
                    /* Nothing to do */
                }
            }

            if (bRetVal)
            {
                sprintf(pcPresetSlectionAuxName, "OK");
            }
            else
            {
                /* Restore valid values */
                u8SelectionBank = 0U;
                u8SelectionProgram = 0U;

                sprintf(pcPresetSlectionAuxName, "ERR");
            }

            (void)bMidiTaskSetBank(u8SelectionBank);
            (void)bMidiTaskSetProgram(u8SelectionProgram);
        }

        /* Handle encoder events */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            /* Clear aux string */
            sprintf(pcPresetSlectionAuxName, "");
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

ui_status_t UI_screen_preset_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenPresetName;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xPresetScreenElementList;
        pxScreenHandler->u32ElementNumber = PRESET_SCREEN_ELEMENT_LAST_ELEMENT;
        pxScreenHandler->render_cb = vScreenPresetRender;
        pxScreenHandler->action_cb = vScreenPresetAction;
        pxScreenHandler->bElementSelection = false;

        /* Init name var */
        sprintf(pcPresetBankName, NAME_FORMAT_BANK, u8SelectionBank);
        sprintf(pcPresetProgramName, NAME_FORMAT_PROGRAM, u8SelectionProgram);
        sprintf(pcPresetProgramName, NAME_FORMAT_NAME, "");
        sprintf(pcPresetSelectName, NAME_FORMAT_SELECT, pcPresetSlectionAuxName);
        sprintf(pcPresetReturnName, NAME_FORMAT_RETURN);

        /* Init elements */
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_BANK].pcName = pcPresetBankName;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_BANK].u32Index = PRESET_SCREEN_ELEMENT_BANK;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_BANK].render_cb = vElementBankRender;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_BANK].action_cb = vElementBankAction;

        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_PROGRAM].pcName = pcPresetProgramName;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_PROGRAM].u32Index = PRESET_SCREEN_ELEMENT_PROGRAM;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_PROGRAM].render_cb = vElementProgramRender;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_PROGRAM].action_cb = vElementProgramAction;

        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_NAME].pcName = pcPresetNameName;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_NAME].u32Index = PRESET_SCREEN_ELEMENT_NAME;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_NAME].render_cb = vElementNameRender;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_NAME].action_cb = vElementNameAction;

        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_SELECT].pcName = pcPresetSelectName;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_SELECT].u32Index = PRESET_SCREEN_ELEMENT_SELECT;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_SELECT].render_cb = vElementSelectRender;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_SELECT].action_cb = vElementSelectAction;

        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_RETURN].pcName = pcPresetReturnName;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_RETURN].u32Index = PRESET_SCREEN_ELEMENT_RETURN;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_RETURN].render_cb = vElementReturnRender;
        xPresetScreenElementList[PRESET_SCREEN_ELEMENT_RETURN].action_cb = vElementReturnAction;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
