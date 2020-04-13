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
#include "synth_task.h"
#include "cli_task.h"
#include "ui_menu_main.h"

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
    FM_SCREEN_ELEMENT_OPERATOR,
    FM_SCREEN_ELEMENT_OP_DETUNE,
    FM_SCREEN_ELEMENT_OP_MULTIPLE,
    FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL,
    FM_SCREEN_ELEMENT_OP_KEY_SCALE,
    FM_SCREEN_ELEMENT_OP_ATTACK_RATE,
    FM_SCREEN_ELEMENT_OP_AMP_MOD_EN,
    FM_SCREEN_ELEMENT_OP_DECAY_RATE,
    FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE,
    FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL,
    FM_SCREEN_ELEMENT_OP_RELEASE_RATE,
    FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE,
    FM_SCREEN_ELEMENT_SAVE,
    FM_SCREEN_ELEMENT_RETURN,
    FM_SCREEN_ELEMENT_LAST
} eFmScreenElement_t;

/* Private define ------------------------------------------------------------*/

/* Max num of elements */
#define UI_NUM_ELEMENT                          (FM_SCREEN_ELEMENT_LAST)

/* Max len for element names */
#define MAX_LEN_NAME                            (16U)
#define MAX_LEN_NAME_SAVE_AUX                   (4U)

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
#define NAME_FORMAT_OPERATOR_ALL                "OPERATOR   ALL"
#define NAME_FORMAT_OP_DETUNE                   " DETUNE    %d"
#define NAME_FORMAT_OP_MULTIPLE                 " MULT      %d"
#define NAME_FORMAT_OP_TOTAL_LEVEL              " TOT LVL   %d"
#define NAME_FORMAT_OP_KEY_SCALE                " KY SCALE  %d"
#define NAME_FORMAT_OP_ATTACK_RATE              " ATT RATE  %d"
#define NAME_FORMAT_OP_AMP_MOD_EN               " AMP MOD   %s"
#define NAME_FORMAT_OP_DECAY_RATE               " DEC RATE  %d"
#define NAME_FORMAT_OP_SUSTAIN_RATE             " SUST RATE %d"
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
uint8_t u8SavePresetSelector = 0U;

char pcElementLabelReturn[MAX_LEN_NAME] = {0U};
char pcElementLabelLfoFreq[MAX_LEN_NAME] = {0U};
char pcElementLabelLfoEn[MAX_LEN_NAME] = {0U};
char pcElementLabelVoice[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceFeedback[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceAlgorithm[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceOut[MAX_LEN_NAME] = {0U};
char pcElementLabelVoiceAmpModSens[MAX_LEN_NAME] = {0U};
char pcElementLabelVoicePhaModSens[MAX_LEN_NAME] = {0U};
char pcElementLabelOperator[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorDetune[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorMultiple[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorTotalLevel[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorKeyScale[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorAttackRate[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorAmpModEn[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorDecayRate[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorSustainRate[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorSustainLevel[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorReleaseRate[MAX_LEN_NAME] = {0U};
char pcElementLabelOperatorSsgEnvelope[MAX_LEN_NAME] = {0U};
char pcElementLabelSave[MAX_LEN_NAME] = {0U};

char pcFmSaveAuxName[MAX_LEN_NAME_SAVE_AUX] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Auc functions */
static uint8_t u8GetVoiceVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, eFmParameter_t eVarType);
static void vSetVoiceVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, eFmParameter_t eVarType, uint8_t u8Value);
static void vActionVoiceElement(xFmDevice_t * pxDeviceCfg, eFmParameter_t eVarType, uint32_t u32Event, uint8_t u8MaxValue);

static uint8_t u8GetOperatorVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, uint8_t u8Operator, eFmParameter_t eVarType);
static void vSetOperatorVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, uint8_t u8Operator, eFmParameter_t eVarType, uint8_t u8Value);
static void vActionOperatorElement(xFmDevice_t * pxDeviceCfg, eFmParameter_t eVarType, uint32_t u32Event, uint8_t u8MaxValue);

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
static void vElementRenderOperator(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorDetune(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorMultiple(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorTotalLevel(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorKeyScale(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorAttackRate(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorAmpModEn(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorDecayRate(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorSustainRate(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorSustainLevel(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorReleaseRate(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperatorSsgEnvelope(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderSave(void * pvDisplay, void * pvScreen, void * pvElement);

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
static void vElementActionOperator(void * pvMenu, void * pvEventData);
static void vElementActionOperatorDetune(void * pvMenu, void * pvEventData);
static void vElementActionOperatorMultiple(void * pvMenu, void * pvEventData);
static void vElementActionOperatorTotalLevel(void * pvMenu, void * pvEventData);
static void vElementActionOperatorKeyScale(void * pvMenu, void * pvEventData);
static void vElementActionOperatorAttackRate(void * pvMenu, void * pvEventData);
static void vElementActionOperatorAmpModEn(void * pvMenu, void * pvEventData);
static void vElementActionOperatorDecayRate(void * pvMenu, void * pvEventData);
static void vElementActionOperatorSustainRate(void * pvMenu, void * pvEventData);
static void vElementActionOperatorSustainLevel(void * pvMenu, void * pvEventData);
static void vElementActionOperatorReleaseRate(void * pvMenu, void * pvEventData);
static void vElementActionOperatorSsgEnvelope(void * pvMenu, void * pvEventData);
static void vElementActionSave(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

static uint8_t u8GetVoiceVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, eFmParameter_t eVarType)
{
    uint8_t u8RetVal = 0U;

    if ((pxDeviceCfg != NULL) && (u8Voice < YM2612_NUM_CHANNEL))
    {
        switch (eVarType)
        {
            case FM_VAR_VOICE_FEEDBACK:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].u8Feedback;
                break;
            case FM_VAR_VOICE_ALGORITHM:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].u8Algorithm;
                break;
            case FM_VAR_VOICE_AUDIO_OUT:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].u8AudioOut;
                break;
            case FM_VAR_VOICE_AMP_MOD_SENS:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].u8AmpModSens;
                break;
            case FM_VAR_VOICE_PHA_MOD_SENS:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].u8PhaseModSens;
                break;
            default:
                break;
        }
    }

    return u8RetVal;
}

static void vSetVoiceVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, eFmParameter_t eVarType, uint8_t u8Value)
{
    if ((pxDeviceCfg != NULL) && (u8Voice < YM2612_NUM_CHANNEL))
    {
        switch (eVarType)
        {
            case FM_VAR_VOICE_FEEDBACK:
                pxDeviceCfg->xChannel[u8Voice].u8Feedback = u8Value;
                break;
            case FM_VAR_VOICE_ALGORITHM:
                pxDeviceCfg->xChannel[u8Voice].u8Algorithm = u8Value;
                break;
            case FM_VAR_VOICE_AUDIO_OUT:
                pxDeviceCfg->xChannel[u8Voice].u8AudioOut = u8Value;
                break;
            case FM_VAR_VOICE_AMP_MOD_SENS:
                pxDeviceCfg->xChannel[u8Voice].u8AmpModSens = u8Value;
                break;
            case FM_VAR_VOICE_PHA_MOD_SENS:
                pxDeviceCfg->xChannel[u8Voice].u8PhaseModSens = u8Value;
                break;
            default:
                break;
        }
    }
}

static uint8_t u8GetOperatorVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, uint8_t u8Operator, eFmParameter_t eVarType)
{
    uint8_t u8RetVal = 0U;

    if ((pxDeviceCfg != NULL) && (u8Voice < YM2612_NUM_CHANNEL) && (u8Operator < YM2612_NUM_OP_CHANNEL))
    {
        switch (eVarType)
        {
            case FM_VAR_OPERATOR_DETUNE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8Detune;
                break;
            case FM_VAR_OPERATOR_MULTIPLE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8Multiple;
                break;
            case FM_VAR_OPERATOR_TOTAL_LEVEL:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8TotalLevel;
                break;
            case FM_VAR_OPERATOR_KEY_SCALE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8KeyScale;
                break;
            case FM_VAR_OPERATOR_ATTACK_RATE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8AttackRate;
                break;
            case FM_VAR_OPERATOR_AMP_MOD:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8AmpMod;
                break;
            case FM_VAR_OPERATOR_DECAY_RATE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8DecayRate;
                break;
            case FM_VAR_OPERATOR_SUSTAIN_RATE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SustainRate;
                break;
            case FM_VAR_OPERATOR_SUSTAIN_LEVEL:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SustainLevel;
                break;
            case FM_VAR_OPERATOR_RELEASE_RATE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8ReleaseRate;
                break;
            case FM_VAR_OPERATOR_SSG_ENVELOPE:
                u8RetVal = pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SsgEg;
                break;
            default:
                break;
        }
    }

    return u8RetVal;
}

static void vSetOperatorVariable(xFmDevice_t * pxDeviceCfg , uint8_t u8Voice, uint8_t u8Operator, eFmParameter_t eVarType, uint8_t u8Value)
{
    if ((pxDeviceCfg != NULL) && (u8Voice < YM2612_NUM_CHANNEL) && (u8Operator < YM2612_NUM_OP_CHANNEL))
    {
        switch (eVarType)
        {
            case FM_VAR_OPERATOR_DETUNE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8Detune = u8Value;
                break;
            case FM_VAR_OPERATOR_MULTIPLE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8Multiple = u8Value;
                break;
            case FM_VAR_OPERATOR_TOTAL_LEVEL:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8TotalLevel = u8Value;
                break;
            case FM_VAR_OPERATOR_KEY_SCALE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8KeyScale = u8Value;
                break;
            case FM_VAR_OPERATOR_ATTACK_RATE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8AttackRate = u8Value;
                break;
            case FM_VAR_OPERATOR_AMP_MOD:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8AmpMod = u8Value;
                break;
            case FM_VAR_OPERATOR_DECAY_RATE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8DecayRate = u8Value;
                break;
            case FM_VAR_OPERATOR_SUSTAIN_RATE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SustainRate = u8Value;
                break;
            case FM_VAR_OPERATOR_SUSTAIN_LEVEL:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SustainLevel = u8Value;
                break;
            case FM_VAR_OPERATOR_RELEASE_RATE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8ReleaseRate = u8Value;
                break;
            case FM_VAR_OPERATOR_SSG_ENVELOPE:
                pxDeviceCfg->xChannel[u8Voice].xOperator[u8Operator].u8SsgEg = u8Value;
                break;
            default:
                break;
        }
    }
}

static void vActionVoiceElement(xFmDevice_t * pxDeviceCfg, eFmParameter_t eVarType, uint32_t u32Event, uint8_t u8MaxValue)
{
    if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (pxDeviceCfg != NULL))
    {
        /* Get channel id to use */
        uint8_t u8TmpChannel = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
        uint8_t u8ValueInit = u8GetVoiceVariable(pxDeviceCfg, u8TmpChannel, eVarType);
        uint8_t u8ValueTmp = u8ValueInit;

        if (CHECK_SIGNAL(u32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (u8ValueTmp > 0U)
            {
                u8ValueTmp--;
            }
        }
        else if (CHECK_SIGNAL(u32Event, UI_SIGNAL_ENC_UPDATE_CW))
        {
            if (u8ValueTmp < (u8MaxValue - 1))
            {
                u8ValueTmp++;
            }
        }

        if (u8ValueInit != u8ValueTmp)
        {
            if (u8VoiceIndex == YM2612_NUM_CHANNEL)
            {
                for (uint32_t u32VoiceIndex = 0; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                {
                    vSetVoiceVariable(pxDeviceCfg, u32VoiceIndex, eVarType, u8ValueTmp);
                }
            }
            else
            {
                vSetVoiceVariable(pxDeviceCfg, u8TmpChannel, eVarType, u8ValueTmp);
            }

            vYM2612_set_reg_preset(pxDeviceCfg);
        }
    }
}

static void vActionOperatorElement(xFmDevice_t * pxDeviceCfg, eFmParameter_t eVarType, uint32_t u32Event, uint8_t u8MaxValue)
{
    if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL) && (pxDeviceCfg != NULL))
    {
        uint8_t u8TmpVoice = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
        uint8_t u8TmpOperator = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;
        uint8_t u8TmpValue = u8GetOperatorVariable(pxDeviceCfg , u8TmpVoice, u8TmpOperator, eVarType);
        uint8_t u8TmpValueInit = u8TmpValue;

        if (CHECK_SIGNAL(u32Event, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (u8TmpValue > 0U)
            {
                u8TmpValue--;
            }
        }
        else if (CHECK_SIGNAL(u32Event, UI_SIGNAL_ENC_UPDATE_CW))
        {
            if (u8TmpValue < (u8MaxValue - 1))
            {
                u8TmpValue++;
            }
        }

        if (u8TmpValueInit != u8TmpValue)
        {
            /* Check and copy if ALL operator selected */
            if (u8OperatorIndex == YM2612_NUM_OP_CHANNEL)
            {
                for (uint8_t u8Index = 0U; u8Index < YM2612_NUM_OP_CHANNEL; u8Index++)
                {
                    vSetOperatorVariable(pxDeviceCfg , u8TmpVoice, u8Index, eVarType, u8TmpValue);
                }
            }
            else
            {
                vSetOperatorVariable(pxDeviceCfg , u8TmpVoice, u8TmpOperator, eVarType, u8TmpValue);
            }

            /* Check and copy if ALL voices selected */
            if (u8VoiceIndex == YM2612_NUM_CHANNEL)
            {
                for (uint32_t u32Index = 1U; u32Index < YM2612_NUM_OP_CHANNEL; u32Index++)
                {
                    pxDeviceCfg->xChannel[u32Index] = pxDeviceCfg->xChannel[0U];
                }
            }

            vYM2612_set_reg_preset(pxDeviceCfg);
        }
    }
}

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            if (u8VoiceIndex == YM2612_NUM_CHANNEL)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

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

static void vElementRenderOperator(void * pvDisplay, void * pvScreen, void * pvElement)
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
            if (u8OperatorIndex == YM2612_NUM_OP_CHANNEL)
            {
                sprintf(pxElement->pcName, NAME_FORMAT_OPERATOR_ALL);
            }
            else
            {
                sprintf(pxElement->pcName, NAME_FORMAT_OPERATOR, u8OperatorIndex);
            }

            /* Print selection ico */
            vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

            u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
        }
    }
}

static void vElementRenderOperatorDetune(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_DETUNE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8Detune);
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

static void vElementRenderOperatorMultiple(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_MULTIPLE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8Multiple);
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

static void vElementRenderOperatorTotalLevel(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_TOTAL_LEVEL, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8TotalLevel);
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

static void vElementRenderOperatorKeyScale(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_KEY_SCALE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8KeyScale);
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

static void vElementRenderOperatorAttackRate(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_ATTACK_RATE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8AttackRate);
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

static void vElementRenderOperatorAmpModEn(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                if (pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8AmpMod)
                {
                    sprintf(pxElement->pcName, NAME_FORMAT_OP_AMP_MOD_EN, "ON");
                }
                else
                {
                    sprintf(pxElement->pcName, NAME_FORMAT_OP_AMP_MOD_EN, "OFF");
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

static void vElementRenderOperatorDecayRate(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_DECAY_RATE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8DecayRate);
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

static void vElementRenderOperatorSustainRate(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_SUSTAIN_RATE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8SustainRate);
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

static void vElementRenderOperatorSustainLevel(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_SUSTAIN_LEVEL, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8SustainLevel);
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

static void vElementRenderOperatorReleaseRate(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_RELEASE_RATE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8ReleaseRate);
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

static void vElementRenderOperatorSsgEnvelope(void * pvDisplay, void * pvScreen, void * pvElement)
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
            xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

            /* Prepare data on buffer */
            if ((u8VoiceIndex <= YM2612_NUM_CHANNEL) && (u8OperatorIndex <= YM2612_NUM_OP_CHANNEL))
            {
                uint8_t u8TmpVoiceIndex = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                uint8_t u8TmpOperatorIndex = (u8OperatorIndex == YM2612_NUM_OP_CHANNEL) ? 0U : u8OperatorIndex;

                sprintf(pxElement->pcName, NAME_FORMAT_OP_SSG_ENVELOPE, pxDeviceCfg->xChannel[u8TmpVoiceIndex].xOperator[u8TmpOperatorIndex].u8SsgEg);
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

static void vElementRenderSave(void * pvDisplay, void * pvScreen, void * pvElement)
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
                if (pcFmSaveAuxName[0U] != 0U)
                {
                    sprintf(pcFmSaveAuxName, "");
                    u8SavePresetSelector = 0U;
                }
            }

            /* Prepare data on buffer */
            sprintf(pxElement->pcName, NAME_FORMAT_SAVE, pcFmSaveAuxName);

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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();
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
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
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
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
                {
                    if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                    {
                        xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                        /* Copy all values */
                        for (uint32_t u32VoiceIndex = 1U; u32VoiceIndex < YM2612_NUM_CHANNEL; u32VoiceIndex++)
                        {
                            pxDeviceCfg->xChannel[u32VoiceIndex] = pxDeviceCfg->xChannel[0U];
                        }

                        /* Apply changes to register */
                        vYM2612_set_reg_preset(pxDeviceCfg);
                    }
                }
            }
        }
        /* Element selection action */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionVoiceElement(pxDeviceCfg, FM_VAR_VOICE_FEEDBACK, *pu32Event, MAX_VALUE_FEEDBACK);
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionVoiceElement(pxDeviceCfg, FM_VAR_VOICE_ALGORITHM, *pu32Event, MAX_VALUE_ALGORITHM);
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionVoiceElement(pxDeviceCfg, FM_VAR_VOICE_AUDIO_OUT, *pu32Event, MAX_VALUE_VOICE_OUT);
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionVoiceElement(pxDeviceCfg, FM_VAR_VOICE_AMP_MOD_SENS, *pu32Event, MAX_VALUE_AMP_MOD_SENS);
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionVoiceElement(pxDeviceCfg, FM_VAR_VOICE_PHA_MOD_SENS, *pu32Event, MAX_VALUE_PHA_MOD_SENS);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperator(void * pvMenu, void * pvEventData)
{
    if ((pvMenu != NULL) && (pvEventData != NULL))
    {
        uint32_t * pu32Event = pvEventData;
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];

        /* Handle encoder events */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW) || CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            if (pxScreen->bElementSelection)
            {
                if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CCW))
                {
                    if (u8OperatorIndex > 0U)
                    {
                        u8OperatorIndex--;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_CW))
                {
                    if (u8OperatorIndex < YM2612_NUM_OP_CHANNEL)
                    {
                        u8OperatorIndex++;
                    }
                }
                else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
                {
                    if (u8OperatorIndex == YM2612_NUM_OP_CHANNEL)
                    {
                        uint8_t u8TmpVoice = (u8VoiceIndex == YM2612_NUM_CHANNEL) ? 0U : u8VoiceIndex;
                        xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                        /* Copy all values op */
                        for (uint32_t u32Index = 1U; u32Index < YM2612_NUM_OP_CHANNEL; u32Index++)
                        {
                            pxDeviceCfg->xChannel[u8TmpVoice].xOperator[u32Index] = pxDeviceCfg->xChannel[u8TmpVoice].xOperator[0U];
                        }

                        /* Check and copy all channels if ALL selected */
                        if (u8VoiceIndex == YM2612_NUM_CHANNEL)
                        {
                            /* Copy all values op */
                            for (uint32_t u32Index = 1U; u32Index < YM2612_NUM_CHANNEL; u32Index++)
                            {
                                pxDeviceCfg->xChannel[u32Index] = pxDeviceCfg->xChannel[0U];
                            }
                        }

                        /* Apply changes to register */
                        vYM2612_set_reg_preset(pxDeviceCfg);
                    }
                }
            }
        }
        /* Element selection action */
        if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorDetune(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_DETUNE, *pu32Event, MAX_VALUE_DETUNE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorMultiple(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_MULTIPLE, *pu32Event, MAX_VALUE_MULTIPLE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorTotalLevel(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_TOTAL_LEVEL, *pu32Event, MAX_VALUE_TOTAL_LEVEL);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorKeyScale(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_KEY_SCALE, *pu32Event, MAX_VALUE_KEY_SCALE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorAttackRate(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_ATTACK_RATE, *pu32Event, MAX_VALUE_ATTACK_RATE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorAmpModEn(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_AMP_MOD, *pu32Event, MAX_VALUE_AMP_MOD_EN);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorDecayRate(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_DECAY_RATE, *pu32Event, MAX_VALUE_DECAY_RATE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorSustainRate(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_SUSTAIN_RATE, *pu32Event, MAX_VALUE_SUSTAIN_RATE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorSustainLevel(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_SUSTAIN_LEVEL, *pu32Event, MAX_VALUE_SUSTAIN_LEVEL);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorReleaseRate(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_RELEASE_RATE, *pu32Event, MAX_VALUE_RELEASE_RATE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionOperatorSsgEnvelope(void * pvMenu, void * pvEventData)
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
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vActionOperatorElement(pxDeviceCfg, FM_VAR_OPERATOR_SSG_ENVELOPE, *pu32Event, MAX_VALUE_SSG_ENVELOPE);
            }
        }
        /* Element selection action */
        else if (CHECK_SIGNAL(*pu32Event, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
    }
}

static void vElementActionSave(void * pvMenu, void * pvEventData)
{
    if (pvEventData != NULL)
    {
        ui_menu_t * pxMenu = pvMenu;
        ui_screen_t * pxScreen = &pxMenu->pxScreenList[pxMenu->u32ScreenSelectionIndex];
        uint32_t * pu32EventData = pvEventData;

        if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET))
        {
            if (pxScreen->bElementSelection)
            {
                xFmDevice_t * pxDeviceCfg = pxYM2612_get_reg_preset();

                vCliPrintf(UI_TASK_NAME, "FM Save User Preset");

                if (bSynthSaveUserPreset(pxDeviceCfg, u8SavePresetSelector))
                {
                    sprintf(pcFmSaveAuxName, "OK");
                }
                else
                {
                    sprintf(pcFmSaveAuxName, "ERR");
                }
            }
            else
            {
                sprintf(pcFmSaveAuxName, "%02d", u8SavePresetSelector);
            }

            pxScreen->bElementSelection = !pxScreen->bElementSelection;
        }
        else if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_CW))
        {
            if (u8SavePresetSelector != 0U)
            {
                u8SavePresetSelector--;
            }

            sprintf(pcFmSaveAuxName, "%02d", u8SavePresetSelector);
        }
        else if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_CCW))
        {
            if (u8SavePresetSelector < (SYNTH_MAX_NUM_USER_PRESET - 1))
            {
                u8SavePresetSelector++;
            }

            sprintf(pcFmSaveAuxName, "%02d", u8SavePresetSelector);
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

        xScreenElementList[FM_SCREEN_ELEMENT_OPERATOR].pcName = pcElementLabelOperator;
        xScreenElementList[FM_SCREEN_ELEMENT_OPERATOR].u32Index = FM_SCREEN_ELEMENT_OPERATOR;
        xScreenElementList[FM_SCREEN_ELEMENT_OPERATOR].render_cb = vElementRenderOperator;
        xScreenElementList[FM_SCREEN_ELEMENT_OPERATOR].action_cb = vElementActionOperator;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_DETUNE].pcName = pcElementLabelOperatorDetune;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DETUNE].u32Index = FM_SCREEN_ELEMENT_OP_DETUNE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DETUNE].render_cb = vElementRenderOperatorDetune;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DETUNE].action_cb = vElementActionOperatorDetune;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_MULTIPLE].pcName = pcElementLabelOperatorMultiple;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_MULTIPLE].u32Index = FM_SCREEN_ELEMENT_OP_MULTIPLE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_MULTIPLE].render_cb = vElementRenderOperatorMultiple;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_MULTIPLE].action_cb = vElementActionOperatorMultiple;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL].pcName = pcElementLabelOperatorTotalLevel;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL].u32Index = FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL].render_cb = vElementRenderOperatorTotalLevel;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_TOTAL_LEVEL].action_cb = vElementActionOperatorTotalLevel;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_KEY_SCALE].pcName = pcElementLabelOperatorKeyScale;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_KEY_SCALE].u32Index = FM_SCREEN_ELEMENT_OP_KEY_SCALE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_KEY_SCALE].render_cb = vElementRenderOperatorKeyScale;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_KEY_SCALE].action_cb = vElementActionOperatorKeyScale;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_ATTACK_RATE].pcName = pcElementLabelOperatorAttackRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_ATTACK_RATE].u32Index = FM_SCREEN_ELEMENT_OP_ATTACK_RATE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_ATTACK_RATE].render_cb = vElementRenderOperatorAttackRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_ATTACK_RATE].action_cb = vElementActionOperatorAttackRate;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_AMP_MOD_EN].pcName = pcElementLabelOperatorAmpModEn;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_AMP_MOD_EN].u32Index = FM_SCREEN_ELEMENT_OP_AMP_MOD_EN;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_AMP_MOD_EN].render_cb = vElementRenderOperatorAmpModEn;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_AMP_MOD_EN].action_cb = vElementActionOperatorAmpModEn;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_DECAY_RATE].pcName = pcElementLabelOperatorDecayRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DECAY_RATE].u32Index = FM_SCREEN_ELEMENT_OP_DECAY_RATE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DECAY_RATE].render_cb = vElementRenderOperatorDecayRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_DECAY_RATE].action_cb = vElementActionOperatorDecayRate;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE].pcName = pcElementLabelOperatorSustainRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE].u32Index = FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE].render_cb = vElementRenderOperatorSustainRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_RATE].action_cb = vElementActionOperatorSustainRate;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL].pcName = pcElementLabelOperatorSustainLevel;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL].u32Index = FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL].render_cb = vElementRenderOperatorSustainLevel;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SUSTAIN_LEVEL].action_cb = vElementActionOperatorSustainLevel;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_RELEASE_RATE].pcName = pcElementLabelOperatorReleaseRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_RELEASE_RATE].u32Index = FM_SCREEN_ELEMENT_OP_RELEASE_RATE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_RELEASE_RATE].render_cb = vElementRenderOperatorReleaseRate;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_RELEASE_RATE].action_cb = vElementActionOperatorReleaseRate;

        xScreenElementList[FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE].pcName = pcElementLabelOperatorSsgEnvelope;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE].u32Index = FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE].render_cb = vElementRenderOperatorSsgEnvelope;
        xScreenElementList[FM_SCREEN_ELEMENT_OP_SSG_ENVELOPE].action_cb = vElementActionOperatorSsgEnvelope;

        xScreenElementList[FM_SCREEN_ELEMENT_SAVE].pcName = pcElementLabelSave;
        xScreenElementList[FM_SCREEN_ELEMENT_SAVE].u32Index = FM_SCREEN_ELEMENT_SAVE;
        xScreenElementList[FM_SCREEN_ELEMENT_SAVE].render_cb = vElementRenderSave;
        xScreenElementList[FM_SCREEN_ELEMENT_SAVE].action_cb = vElementActionSave;

        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].pcName = pcElementLabelReturn;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].u32Index = FM_SCREEN_ELEMENT_RETURN;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].render_cb = vElementRenderReturn;
        xScreenElementList[FM_SCREEN_ELEMENT_RETURN].action_cb = vElementActionReturn;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
