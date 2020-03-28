/**
  ******************************************************************************
  * @file           : ui_screen_fm.c
  * @brief          : UI definition for fm screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_fm.h"
#include "printf.h"
#include "ui_task.h"
#include "cli_task.h"
#include "ui_menu_main.h"
#include "YM2612_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for FM screen */
typedef enum
{
    FM_SCREEN_ELEMENT_LFO_FREQ = 0U,
    FM_SCREEN_ELEMENT_LFO_EN,
    FM_SCREEN_ELEMENT_VOICE,
    FM_SCREEN_ELEMENT_VOICE_FEEDBACK,
    FM_SCREEN_ELEMENT_VOICE_ALGORITHM,
    FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT,
    FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS,
    FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE,
    // FM_SCREEN_ELEMENT_OPERATOR,
    // FM_SCREEN_ELEMENT_OP_DETUNE,
    // FM_SCREEN_ELEMENT_OP_MULTIPLE,
    // FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL,
    // FM_SCREEN_ELEMENT_OP_KEY_SCALE,
    // FM_SCREEN_ELEMENT_OP_ATTACK_RATE,
    // FM_SCREEN_ELEMENT_OP_AMP_MOD_EN,
    // FM_SCREEN_ELEMENT_OP_DECAY_RATE,
    // FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL,
    // FM_SCREEN_ELEMENT_OP_RELEASE_RATE,
    // FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE,
    // FM_SCREEN_ELEMENT_SAVE,
    FM_SCREEN_ELEMENT_RETURN,
    FM_SCREEN_ELEMENT_LAST
} eFmScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max num of elements */
#define UI_NUM_ELEMENT                          (FM_SCREEN_ELEMENT_LAST)

/* Max len for element names */
#define MAX_LEN_NAME                            (16U)

#define NAME_FORMAT_LFO_FREQ                    "LFO FREQ   %d"
#define NAME_FORMAT_LFO_EN                      "LFO EN     %s"
#define NAME_FORMAT_VOICE                       "VOICE      %d"
#define NAME_FORMAT_VOICE_ALL                   "VOICE      ALL"
#define NAME_FORMAT_VOICE_FEEDBACK              " FEEDBACK  %d"
#define NAME_FORMAT_VOICE_ALGORITHM             " ALGORTHM  %d"
#define NAME_FORMAT_VOICE_AUDIO_OUT             " OUT       %s"
#define NAME_FORMAT_VOICE_VOICE_AMP_MOD_SENS    " AMS       %d"
#define NAME_FORMAT_VOICE_VOICE_AMP_MOD_PHAS    " PMS       %d"
#define NAME_FORMAT_OPERATOR                    "OPERATOR   %d"
#define NAME_FORMAT_OP_DETUNE                   " DETUNE    %d"
#define NAME_FORMAT_OP_MULTIPLE                 " MULT      %d"
#define NAME_FORMAT_OP_TOTAL_LEVEL              " TOT LVL   %d"
#define NAME_FORMAT_OP_KEY_SCALE                " KY SCALE  %d"
#define NAME_FORMAT_OP_ATTACK_RATE              " ATT RATE  %d"
#define NAME_FORMAT_OP_AMP_MOD_EN               " AMP MOD   %s"
#define NAME_FORMAT_OP_DECAY_RATE               " DEC RATE  %d"
#define NAME_FORMAT_OP_SUSTAIN_LEVEL            " SUST LVL  %d"
#define NAME_FORMAT_OP_RELEASE_RATE             " REL RATE  %d"
#define NAME_FORMAT_OP_SSG_ENVELOPE             " SSG ENV   %d"
#define NAME_FORMAT_SAVE                        "SAVE       %s"
#define NAME_FORMAT_RETURN                      "BACK"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenName[MAX_LEN_NAME] = "FM";

ui_element_t xScreenElementList[UI_NUM_ELEMENT];

uint8_t u8VoiceIndex = 0U;
uint8_t u8OperatorIndex = 0U;

char pcElementLabelReturn[MAX_LEN_NAME] = {0U};
char pcElementLabelLfoFreq[MAX_LEN_NAME] = {0U};
char pcElementLabelLfoEn[MAX_LEN_NAME] = {0U};
char pcElementLabelVoice[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceFeedback[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceAlgorithm[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceOut[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceAmpModSens[MAX_LEN_NAME] = {0U};
char pcElementLabelVoicePhaModSens[MAX_LEN_NAME] = {0U};

/* Private function prototypes -----------------------------------------------*/

/* Render functions */
static void vScreenRender(void * pvDisplay, void * pvScreen);
static void vElementRenderReturn(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderLfoFreq(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderLfoEn(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoice(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoiceFeedback(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoiceAlgorithm(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoiceOut(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoiceAmpModSens(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoicePhaModSens(void * pvDisplay, void * pvScreen, void * pvElement);

/* Actions functions */
static void vScreenAction(void * pvMenu, void * pvEventData);
static void vElementActionReturn(void * pvMenu, void * pvEventData);
static void vElementActionLfoFreq(void * pvMenu, void * pvEventData);
static void vElementActionLfoEn(void * pvMenu, void * pvEventData);
static void vElementActionVoice(void * pvMenu, void * pvEventData);
static void vElementActionVoiceFeedback(void * pvMenu, void * pvEventData);
static void vElementActionVoiceAlgorithm(void * pvMenu, void * pvEventData);
static void vElementActionVoiceOut(void * pvMenu, void * pvEventData);
static void vElementActionVoiceAmpModSens(void * pvMenu, void * pvEventData);
static void vElementActionVoicePhaModSens(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* RENDER --------------------------------------------------------------------*/

static void vScreenRender(void * pvDisplay, void * pvScreen)
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

static void vElementRenderReturn(void * pvDisplay, void * pvScreen, void * pvElement)
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

static void vElementRenderLfoFreq(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            sprintf(pxElement->pcName, NAME_FORMAT_LFO_FREQ, pxDeviceCfg->u8LfoFreq);

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderLfoEn(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            if (pxDeviceCfg->u8LfoOn == 0U)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_LFO_EN, "OFF");
            }
            else if (pxDeviceCfg->u8LfoOn == 1U)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_LFO_EN, "ON");
            }
            else
            {
                sprintf(pxElement->pcName, NAME_FORMAT_LFO_EN, "ERR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoice(void * pvDisplay, void * pvScreen, void * pvElement)
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
            if (u8VoiceIndex == YM2612_MAX_NUM_VOICE)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_ALL);
            }
            else
            {
                sprintf(pxElement->pcName, NAME_FORMAT_VOICE, u8VoiceIndex);
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoiceFeedback(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            if (u8VoiceIndex < YM2612_NUM_CHANNEL)
            {
                /* Prepare data on buffer */
                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_FEEDBACK, pxDeviceCfg->xChannel[u8VoiceIndex].u8Feedback);
            }
            else if (u8VoiceIndex == YM2612_NUM_CHANNEL)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_FEEDBACK, pxDeviceCfg->xChannel[0U].u8Feedback);
            }
            else
            {
                sprintf(pxElement->pcName, "ERROR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoiceAlgorithm(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
            {
                uint8_t u8TmpIndex = u8VoiceIndex;

                if (u8TmpIndex == YM2612_NUM_CHANNEL)
                {
                    u8TmpIndex = 0U;
                }

                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_ALGORITHM, pxDeviceCfg->xChannel[u8TmpIndex].u8Algorithm);
            }
            else
            {
                sprintf(pxElement->pcName, "ERROR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoiceOut(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
            {
                uint8_t u8TmpIndex = u8VoiceIndex;

                if (u8TmpIndex == YM2612_NUM_CHANNEL)
                {
                    u8TmpIndex = 0U;
                }

                switch (pxDeviceCfg->xChannel[u8TmpIndex].u8AudioOut)
                {
                case 0U:
                    sprintf(pxElement->pcName, NAME_FORMAT_VOICE_AUDIO_OUT, "OFF");
                    break;

                case 1U:
                    sprintf(pxElement->pcName, NAME_FORMAT_VOICE_AUDIO_OUT, "R");
                    break;

                case 2U:
                    sprintf(pxElement->pcName, NAME_FORMAT_VOICE_AUDIO_OUT, "L");
                    break;
                
                case 3U:
                    sprintf(pxElement->pcName, NAME_FORMAT_VOICE_AUDIO_OUT, "LR");
                    break;

                default:
                    sprintf(pxElement->pcName, NAME_FORMAT_VOICE_AUDIO_OUT, "ERR");
                    break;
                }
            }
            else
            {
                sprintf(pxElement->pcName, "ERROR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoiceAmpModSens(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
            {
                uint8_t u8TmpIndex = u8VoiceIndex;

                if (u8TmpIndex == YM2612_NUM_CHANNEL)
                {
                    u8TmpIndex = 0U;
                }

                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_VOICE_AMP_MOD_SENS, pxDeviceCfg->xChannel[u8TmpIndex].u8AmpModSens);
            }
            else
            {
                sprintf(pxElement->pcName, "ERROR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderVoicePhaModSens(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

            /* Prepare data on buffer */
            if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
            {
                uint8_t u8TmpIndex = u8VoiceIndex;

                if (u8TmpIndex == YM2612_NUM_CHANNEL)
                {
                    u8TmpIndex = 0U;
                }

                sprintf(pxElement->pcName, NAME_FORMAT_VOICE_VOICE_AMP_MOD_PHAS, pxDeviceCfg->xChannel[u8TmpIndex].u8PhaseModSens);
            }
            else
            {
                sprintf(pxElement->pcName, "ERROR");
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

/* ACTION --------------------------------------------------------------------*/

static void vScreenAction(void * pvMenu, void * pvEventData)
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

static void vElementActionReturn(void * pvMenu, void * pvEventData)
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

static void vElementActionLfoFreq(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();
                uint8_t u8TmpValue = pxDeviceCfg->u8LfoFreq;

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (pxDeviceCfg->u8LfoFreq > 0U)
                    {
                        pxDeviceCfg->u8LfoFreq--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (pxDeviceCfg->u8LfoFreq < (MAX_VALUE_LFO_FREQ - 1))
                    {
                        pxDeviceCfg->u8LfoFreq++;
                    }
                }

                if (pxDeviceCfg->u8LfoFreq != u8TmpValue)
                {
                    vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionLfoEn(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();
                uint8_t u8TmpValue = pxDeviceCfg->u8LfoOn;

                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (pxDeviceCfg->u8LfoOn == 0U)
                    {
                        pxDeviceCfg->u8LfoOn = 1U;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (pxDeviceCfg->u8LfoOn == 1U)
                    {
                        pxDeviceCfg->u8LfoOn = 0U;
                    }
                }

                if (pxDeviceCfg->u8LfoOn != u8TmpValue)
                {
                    vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionVoice(void * pvMenu, void * pvEventData)
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
                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8VoiceIndex > 0U)
                    {
                        u8VoiceIndex--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (u8VoiceIndex < YM2612_NUM_CHANNEL)
                    {
                        u8VoiceIndex++;
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

static void vElementActionVoiceFeedback(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

                if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
                {
                    uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                    uint8_t u8TmpValue = pxDeviceCfg->xChannel[u8TmpChannel].u8Feedback;

                    if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                    {
                        if (u8TmpValue > 0U)
                        {
                            u8TmpValue--;
                        }
                    }
                    else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                    {
                        if (u8TmpValue < (MAX_VALUE_FEEDBACK - 1))
                        {
                            u8TmpValue++;
                        }
                    }

                    if (pxDeviceCfg->xChannel[u8TmpChannel].u8Feedback != u8TmpValue)
                    {
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                            {
                                pxDeviceCfg->xChannel[u32VoiceIndex].u8Feedback = u8TmpValue;
                            }
                        }
                        else
                        {
                            pxDeviceCfg->xChannel[u8TmpChannel].u8Feedback = u8TmpValue;
                        }

                        vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionVoiceAlgorithm(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

                if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
                {
                    uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                    uint8_t u8TmpValue = pxDeviceCfg->xChannel[u8TmpChannel].u8Algorithm;

                    if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                    {
                        if (u8TmpValue > 0U)
                        {
                            u8TmpValue--;
                        }
                    }
                    else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                    {
                        if (u8TmpValue < (MAX_VALUE_ALGORITHM - 1))
                        {
                            u8TmpValue++;
                        }
                    }

                    if (pxDeviceCfg->xChannel[u8TmpChannel].u8Algorithm != u8TmpValue)
                    {
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                            {
                                pxDeviceCfg->xChannel[u32VoiceIndex].u8Algorithm = u8TmpValue;
                            }
                        }
                        else
                        {
                            pxDeviceCfg->xChannel[u8TmpChannel].u8Algorithm = u8TmpValue;
                        }

                        vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionVoiceOut(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

                if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
                {
                    uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                    uint8_t u8TmpValue = pxDeviceCfg->xChannel[u8TmpChannel].u8AudioOut;

                    if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                    {
                        if (u8TmpValue > 0U)
                        {
                            u8TmpValue--;
                        }
                    }
                    else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                    {
                        if (u8TmpValue < (MAX_VALUE_VOICE_OUT - 1))
                        {
                            u8TmpValue++;
                        }
                    }

                    if (pxDeviceCfg->xChannel[u8TmpChannel].u8AudioOut != u8TmpValue)
                    {
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                            {
                                pxDeviceCfg->xChannel[u32VoiceIndex].u8AudioOut = u8TmpValue;
                            }
                        }
                        else
                        {
                            pxDeviceCfg->xChannel[u8TmpChannel].u8AudioOut = u8TmpValue;
                        }

                        vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionVoiceAmpModSens(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

                if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
                {
                    uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                    uint8_t u8TmpValue = pxDeviceCfg->xChannel[u8TmpChannel].u8AmpModSens;

                    if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                    {
                        if (u8TmpValue > 0U)
                        {
                            u8TmpValue--;
                        }
                    }
                    else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                    {
                        if (u8TmpValue < (MAX_VALUE_AMP_MOD_SENS - 1))
                        {
                            u8TmpValue++;
                        }
                    }

                    if (pxDeviceCfg->xChannel[u8TmpChannel].u8AmpModSens != u8TmpValue)
                    {
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                            {
                                pxDeviceCfg->xChannel[u32VoiceIndex].u8AmpModSens = u8TmpValue;
                            }
                        }
                        else
                        {
                            pxDeviceCfg->xChannel[u8TmpChannel].u8AmpModSens = u8TmpValue;
                        }

                        vYM2612_set_reg_preset(pxDeviceCfg);
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

static void vElementActionVoicePhaModSens(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_set_reg_preset();

                if (u8VoiceIndex <= YM2612_NUM_CHANNEL)
                {
                    uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                    uint8_t u8TmpValue = pxDeviceCfg->xChannel[u8TmpChannel].u8PhaseModSens;

                    if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                    {
                        if (u8TmpValue > 0U)
                        {
                            u8TmpValue--;
                        }
                    }
                    else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                    {
                        if (u8TmpValue < (MAX_VALUE_PHA_MOD_SENS - 1))
                        {
                            u8TmpValue++;
                        }
                    }

                    if (pxDeviceCfg->xChannel[u8TmpChannel].u8PhaseModSens != u8TmpValue)
                    {
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                            {
                                pxDeviceCfg->xChannel[u32VoiceIndex].u8PhaseModSens = u8TmpValue;
                            }
                        }
                        else
                        {
                            pxDeviceCfg->xChannel[u8TmpChannel].u8PhaseModSens = u8TmpValue;
                        }

                        vYM2612_set_reg_preset(pxDeviceCfg);
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

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_fm_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenName;
        pxScreenHandler->u32ElementRenderIndex = 0U;
        pxScreenHandler->u32ElementSelectionIndex = 0U;
        pxScreenHandler->pxElementList = xScreenElementList;
        pxScreenHandler->u32ElementNumber = FM_SCREEN_ELEMENT_LAST;
        pxScreenHandler->render_cb = vScreenRender;
        pxScreenHandler->action_cb = vScreenAction;
        pxScreenHandler->bElementSelection = false;

        /* Init name var */
        sprintf(pcElementLabelLfoFreq, NAME_FORMAT_LFO_FREQ, 0U);
        sprintf(pcElementLabelReturn, NAME_FORMAT_RETURN);

        /* Init elements */
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_FREQ].pcName = pcElementLabelLfoFreq;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_FREQ].u32Index = FM_SCREEN_ELEMENT_LFO_FREQ;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_FREQ].render_cb = vElementRenderLfoFreq;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_FREQ].action_cb = vElementActionLfoFreq;

        xScreenElementList[FM_SCREEN_ELEMENT_LFO_EN].pcName = pcElementLabelLfoEn;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_EN].u32Index = FM_SCREEN_ELEMENT_LFO_EN;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_EN].render_cb = vElementRenderLfoEn;
        xScreenElementList[FM_SCREEN_ELEMENT_LFO_EN].action_cb = vElementActionLfoEn;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE].pcName = pcElementLabelVoice;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE].u32Index = FM_SCREEN_ELEMENT_VOICE;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE].render_cb = vElementRenderVoice;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE].action_cb = vElementActionVoice;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_FEEDBACK].pcName = pcElementLabelVoiceFeedback;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_FEEDBACK].u32Index = FM_SCREEN_ELEMENT_VOICE_FEEDBACK;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_FEEDBACK].render_cb = vElementRenderVoiceFeedback;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_FEEDBACK].action_cb = vElementActionVoiceFeedback;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_ALGORITHM].pcName = pcElementLabelVoiceAlgorithm;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_ALGORITHM].u32Index = FM_SCREEN_ELEMENT_VOICE_ALGORITHM;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_ALGORITHM].render_cb = vElementRenderVoiceAlgorithm;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_ALGORITHM].action_cb = vElementActionVoiceAlgorithm;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT].pcName = pcElementLabelVoiceOut;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT].u32Index = FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT].render_cb = vElementRenderVoiceOut;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AUDIO_OUT].action_cb = vElementActionVoiceOut;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS].pcName = pcElementLabelVoiceAmpModSens;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS].u32Index = FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS].render_cb = vElementRenderVoiceAmpModSens;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_SENS].action_cb = vElementActionVoiceAmpModSens;

        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE].pcName = pcElementLabelVoicePhaModSens;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE].u32Index = FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE].render_cb = vElementRenderVoicePhaModSens;
        xScreenElementList[FM_SCREEN_ELEMENT_VOICE_AMP_MOD_PHASE].action_cb = vElementActionVoicePhaModSens;

        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].pcName = pcElementLabelReturn;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].u32Index = FM_SCREEN_ELEMENT_RETURN;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].render_cb = vElementRenderReturn;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].action_cb = vElementActionReturn;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
