/**
  ******************************************************************************
  * @file           : ui_screen_idle.c
  * @brief          : UI definition for idle screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_idle.h"
#include "error.h"

#include "printf.h"
#include "ui_menu_main.h"

#include "ui_task.h"
#include "midi_task.h"
#include "synth_task.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for FM screen */
typedef enum
{
    IDLE_SCREEN_ELEMENT_DATA_CH_00 = 0U,
    IDLE_SCREEN_ELEMENT_DATA_CH_01,
    IDLE_SCREEN_ELEMENT_DATA_CH_02,
    IDLE_SCREEN_ELEMENT_DATA_CH_03,
    IDLE_SCREEN_ELEMENT_DATA_CH_04,
    IDLE_SCREEN_ELEMENT_DATA_CH_05,
    IDLE_SCREEN_ELEMENT_CMD_CC,
    IDLE_SCREEN_ELEMENT_LAST
} eIdleScreenElement_t;

/* Private define ------------------------------------------------------------*/

/** Max num of elements */
#define UI_NUM_ELEMENT              (IDLE_SCREEN_ELEMENT_LAST)

/** Max len for element names */
#define MAX_LEN_NAME                (16U)

/** Notes in an octave */
#define MAX_NOTES_OCTAVE            (12U)

/** Number of voice channels */
#define MAX_VOICE_CH                (IDLE_SCREEN_ELEMENT_DATA_CH_05 + 1U)

/** Return element string */
#define NAME_FORMAT_DATA_CH_ON      "CH%d %s%d" // voice - note

/** Msg note off */
#define NAME_FORMAT_DATA_CH_OFF     "CH%d ---"

/** Msg CC command */
#define NAME_FORMAT_CMD_CC_0        "CC %03d %03d"
#define NAME_FORMAT_CMD_CC_1        "%s %03d"

/** Base offset coord x */
#define BASE_OFFSET_X               (0U)

/** Base offset coord y */
#define BASE_OFFSET_Y               (30U)

/** Base offset coord y */
#define BASE_OFFSET_Y               (30U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenNameIdle[MAX_LEN_NAME] = "MIDI TRACK";

const char * pcOctaveSimbol[MAX_NOTES_OCTAVE] = {
    " C",
    "C#",
    " D",
    "D#",
    " E",
    " F",
    "F#",
    " G",
    "G#",
    " A",
    "A#",
    " B"
};

ui_element_t xElementIdleScreenElementList[UI_NUM_ELEMENT];

char pcNameDataCh0[MAX_LEN_NAME] = {0};
char pcNameDataCh1[MAX_LEN_NAME] = {0};
char pcNameDataCh2[MAX_LEN_NAME] = {0};
char pcNameDataCh3[MAX_LEN_NAME] = {0};
char pcNameDataCh4[MAX_LEN_NAME] = {0};
char pcNameDataCh5[MAX_LEN_NAME] = {0};
char pcNameCmdCc[MAX_LEN_NAME] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Render functions */
static void vScreenRenderIdle(void * pvDisplay, void * pvScreen);
static void vElementRenderDataCh(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderCmdCc(void * pvDisplay, void * pvScreen, void * pvElement);

/* Actions functions */
static void vScreenActionIdle(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* RENDER --------------------------------------------------------------------*/

static void vScreenRenderIdle(void * pvDisplay, void * pvScreen)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);

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
#ifdef MIDI_DBG_STATS
    static char pcTestStr[MAX_LEN_NAME] = {0};
    extern volatile uint32_t u32MidiCmdCount;
    sprintf(pcTestStr, "%s %d", pxScreen->pcName, u32MidiCmdCount);
    u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pcTestStr);
#else
    u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxScreen->pcName);
#endif /* MIDI_DBG_STATS */

    u8g2_DrawHLine(pxDisplayHandler, u8LineX, u8LineY, u8LineWith);
}

static void vElementRenderDataCh(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

    (void)pvScreen;

    u8g2_t * pxDisplayHandler = pvDisplay;
    ui_element_t * pxElement = pvElement;

    if (pxElement->u32Index < MAX_VOICE_CH)
    {
        /* Compute element offset */
        uint8_t u8OffsetX = BASE_OFFSET_X;
        uint8_t u8OffsetY = BASE_OFFSET_Y + ((pxElement->u32Index / 2U) * (u8g2_GetMaxCharHeight(pxDisplayHandler) + 8U));

        if (pxElement->u32Index % 2U)
        {
            u8OffsetX += (uint8_t)(u8g2_GetDisplayWidth(pxDisplayHandler) / 2U);
        }

        /* Get current note */
        uint8_t u8MidiNote = xMidiTaskGetNote(pxElement->u32Index);

        if (u8MidiNote != MIDI_DATA_NOT_VALID)
        {
            uint8_t u8Note = u8MidiNote % MAX_NOTES_OCTAVE;
            uint8_t u8Octave = u8MidiNote / MAX_NOTES_OCTAVE;

            sprintf(pxElement->pcName, NAME_FORMAT_DATA_CH_ON, pxElement->u32Index, pcOctaveSimbol[u8Note], u8Octave);
        }
        else
        {
            sprintf(pxElement->pcName, NAME_FORMAT_DATA_CH_OFF, pxElement->u32Index);
        }

        u8g2_DrawStr(pxDisplayHandler, u8OffsetX, u8OffsetY, pxElement->pcName);
    }
}

static void vElementRenderCmdCc(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

    u8g2_t * pxDisplayHandler = pvDisplay;
    ui_screen_t * pxScreen = pvScreen;
    ui_element_t * pxElement = pvElement;

    if (pxScreen->bElementSelection)
    {
        uint8_t u8OffsetX = BASE_OFFSET_X;
        uint8_t u8OffsetY = UI_OFFSET_SCREEN_Y + UI_OFFSET_LINE_SCREEN_Y + 1U;
        uint8_t u8BoxWidth = u8g2_GetDisplayWidth(pxDisplayHandler);
        uint8_t u8BoxHeight = u8g2_GetDisplayHeight(pxDisplayHandler);

        u8g2_SetDrawColor(pxDisplayHandler, 0U);
        u8g2_DrawBox(pxDisplayHandler, u8OffsetX, u8OffsetY, u8BoxWidth, u8BoxHeight);
        u8g2_SetDrawColor(pxDisplayHandler, 1U);

        /* Get CC data here */
        SynthCcMap_t xCcData = xSynthGetLastCc();

        u8OffsetY += u8g2_GetMaxCharHeight(pxDisplayHandler) + 10U;
        snprintf(pxElement->pcName, MAX_LEN_NAME, NAME_FORMAT_CMD_CC_0, xCcData.u8Cmd, xCcData.u8CcData);
        u8g2_DrawStr(pxDisplayHandler, u8OffsetX, u8OffsetY, pxElement->pcName);

        u8OffsetY += u8g2_GetMaxCharHeight(pxDisplayHandler) + 10U;
        snprintf(pxElement->pcName, MAX_LEN_NAME, NAME_FORMAT_CMD_CC_1, xCcData.pcParamName, xCcData.u8Data);
        u8g2_DrawStr(pxDisplayHandler, u8OffsetX, u8OffsetY, pxElement->pcName);
    }
}

/* ACTION --------------------------------------------------------------------*/

static void vScreenActionIdle(void * pvMenu, void * pvEventData)
{
    ERR_ASSERT(pvMenu != NULL);
    ERR_ASSERT(pvEventData != NULL);

    ui_menu_t * pxMenu = pvMenu;
    ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

    if (pxScreen != NULL)
    {
        /* Check if is a general event */
        uint32_t * pu32Event = pvEventData;
        ui_element_t * pxElement = &pxScreen->pxElementList[pxScreen->u32ElementSelectionIndex];

        /* Handle encoder events */
        if ( CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || 
                CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW) )
        {
            bUiTaskNotify(UI_SIGNAL_RESTORE_IDLE);
        }
        else if ( CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET) )
        {
            vMidiPanic();
        }
        else if ( CHECK_SIGNAL(*pu32Event, UI_SIGNAL_MIDI_CC) )
        {
            pxScreen->bElementSelection = true;
        }
        else if ( CHECK_SIGNAL(*pu32Event, UI_SIGNAL_RESTORE_CC) )
        {
            pxScreen->bElementSelection = false;
        }

        /* Handle action for selected element */
        if (pxElement->action_cb != NULL)
        {
            pxElement->action_cb(pxMenu, pvEventData);
        }
    }
}

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_idle_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenNameIdle;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xElementIdleScreenElementList;
        pxScreenHandler->u32ElementNumber = IDLE_SCREEN_ELEMENT_LAST;
        pxScreenHandler->render_cb = vScreenRenderIdle;
        pxScreenHandler->action_cb = vScreenActionIdle;
        pxScreenHandler->bElementSelection = false;

        /* Init elements */
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_00].pcName = pcNameDataCh0;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_00].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_00;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_00].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_00].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_01].pcName = pcNameDataCh1;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_01].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_01;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_01].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_01].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_02].pcName = pcNameDataCh2;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_02].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_02;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_02].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_02].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_03].pcName = pcNameDataCh3;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_03].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_03;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_03].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_03].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_04].pcName = pcNameDataCh4;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_04].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_04;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_04].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_04].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_05].pcName = pcNameDataCh5;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_05].u32Index = IDLE_SCREEN_ELEMENT_DATA_CH_05;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_05].render_cb = vElementRenderDataCh;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_DATA_CH_05].action_cb = NULL;

        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_CMD_CC].pcName = pcNameCmdCc;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_CMD_CC].u32Index = IDLE_SCREEN_ELEMENT_CMD_CC;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_CMD_CC].render_cb = vElementRenderCmdCc;
        xElementIdleScreenElementList[IDLE_SCREEN_ELEMENT_CMD_CC].action_cb = NULL;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
