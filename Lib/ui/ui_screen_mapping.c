/**
  ******************************************************************************
  * @file           : ui_screen_mapping.c
  * @brief          : UI definition for mapping screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_mapping.h"
#include "error.h"

#include "printf.h"
#include "ui_menu_main.h"

#include "ui_task.h"
#include "cli_task.h"
#include "mapping_task.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for FM screen */
typedef enum
{
    MAP_SCREEN_ELEMENT_CFG_ID = 0U,
    MAP_SCREEN_ELEMENT_MODE,
    MAP_SCREEN_ELEMENT_VOICE,
    MAP_SCREEN_ELEMENT_OPERATOR,
    MAP_SCREEN_ELEMENT_PARAMETER,
    MAP_SCREEN_ELEMENT_RETURN,
    MAP_SCREEN_ELEMENT_LAST
} eMapScreenElement_t;

/* Private define ------------------------------------------------------------*/

/** Max num of elements */
#define UI_NUM_ELEMENT              (MAP_SCREEN_ELEMENT_LAST)

/** Max len for element names */
#define MAX_LEN_NAME                (16U)

/** Return element string */
#define NAME_FORMAT_CFG_ID          "MAPPING      %d"
#define NAME_FORMAT_MODE            "MODE      %s"
#define NAME_FORMAT_VOICE           "VOICE CH     %d"
#define NAME_FORMAT_OPERATOR        "OPERATOR     %d"
#define NAME_FORMAT_PARAMETER       "PARAM     %s"
#define NAME_FORMAT_RETURN          "RETURN"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

const char pcScreenNameMap[MAX_LEN_NAME] = "MAPPING";

ui_element_t xElementMapScreenElementList[UI_NUM_ELEMENT];

char pcMapNameCfgId[MAX_LEN_NAME] = {0};
char pcMapNameMode[MAX_LEN_NAME] = {0};
char pcMapNameVoice[MAX_LEN_NAME] = {0};
char pcMapNameOperator[MAX_LEN_NAME] = {0};
char pcMapNameParameter[MAX_LEN_NAME] = {0};
char pcMapNameReturn[MAX_LEN_NAME] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Render functions */
static void vScreenRenderMap(void * pvDisplay, void * pvScreen);
static void vElementRenderCfgId(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderMode(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderVoice(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderOperator(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderParameter(void * pvDisplay, void * pvScreen, void * pvElement);
static void vElementRenderReturn(void * pvDisplay, void * pvScreen, void * pvElement);

/* Actions functions */
static void vScreenActionMap(void * pvMenu, void * pvEventData);
static void vElementActionCfgId(void * pvMenu, void * pvEventData);
static void vElementActionMode(void * pvMenu, void * pvEventData);
static void vElementActionVoice(void * pvMenu, void * pvEventData);
static void vElementActionOperator(void * pvMenu, void * pvEventData);
static void vElementActionParameter(void * pvMenu, void * pvEventData);
static void vElementActionReturn(void * pvMenu, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* RENDER --------------------------------------------------------------------*/

static void vScreenRenderMap(void * pvDisplay, void * pvScreen)
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
    u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxScreen->pcName);
    u8g2_DrawHLine(pxDisplayHandler, u8LineX, u8LineY, u8LineWith);
}

static void vElementRenderCfgId(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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
        sprintf(pxElement->pcName, NAME_FORMAT_CFG_ID, 0U);

        /* Print selection ico */
        vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementRenderMode(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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
        sprintf(pxElement->pcName, NAME_FORMAT_MODE, "NONE");

        /* Print selection ico */
        vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementRenderVoice(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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
        sprintf(pxElement->pcName, NAME_FORMAT_VOICE, 0U);

        /* Print selection ico */
        vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementRenderOperator(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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
        sprintf(pxElement->pcName, NAME_FORMAT_OPERATOR, 0U);

        /* Print selection ico */
        vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementRenderParameter(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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
        sprintf(pxElement->pcName, NAME_FORMAT_PARAMETER, "NONE");

        /* Print selection ico */
        vUI_MISC_DrawSelection(pxDisplayHandler, pxScreen, pxElement->u32Index, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDisplayHandler, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementRenderReturn(void * pvDisplay, void * pvScreen, void * pvElement)
{
    ERR_ASSERT(pvDisplay != NULL);
    ERR_ASSERT(pvScreen != NULL);
    ERR_ASSERT(pvElement != NULL);

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

/* ACTION --------------------------------------------------------------------*/

static void vScreenActionMap(void * pvMenu, void * pvEventData)
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

static void vElementActionCfgId(void * pvMenu, void * pvEventData)
{

}

static void vElementActionMode(void * pvMenu, void * pvEventData)
{

}

static void vElementActionVoice(void * pvMenu, void * pvEventData)
{

}

static void vElementActionOperator(void * pvMenu, void * pvEventData)
{

}

static void vElementActionParameter(void * pvMenu, void * pvEventData)
{

}

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

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_map_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenNameMap;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xElementMapScreenElementList;
        pxScreenHandler->u32ElementNumber = MAP_SCREEN_ELEMENT_LAST;
        pxScreenHandler->render_cb = vScreenRenderMap;
        pxScreenHandler->action_cb = vScreenActionMap;
        pxScreenHandler->bElementSelection = false;

        /* Init elements */
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_CFG_ID].pcName = pcMapNameCfgId;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_CFG_ID].u32Index = MAP_SCREEN_ELEMENT_CFG_ID;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_CFG_ID].render_cb = vElementRenderCfgId;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_CFG_ID].action_cb = vElementActionCfgId;

        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_MODE].pcName = pcMapNameMode;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_MODE].u32Index = MAP_SCREEN_ELEMENT_MODE;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_MODE].render_cb = vElementRenderMode;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_MODE].action_cb = vElementActionMode;

        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_VOICE].pcName = pcMapNameVoice;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_VOICE].u32Index = MAP_SCREEN_ELEMENT_VOICE;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_VOICE].render_cb = vElementRenderVoice;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_VOICE].action_cb = vElementActionVoice;

        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_OPERATOR].pcName = pcMapNameOperator;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_OPERATOR].u32Index = MAP_SCREEN_ELEMENT_OPERATOR;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_OPERATOR].render_cb = vElementRenderOperator;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_OPERATOR].action_cb = vElementActionOperator;

        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_PARAMETER].pcName = pcMapNameParameter;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_PARAMETER].u32Index = MAP_SCREEN_ELEMENT_PARAMETER;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_PARAMETER].render_cb = vElementRenderParameter;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_PARAMETER].action_cb = vElementActionParameter;

        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].pcName = pcMapNameReturn;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].u32Index = MAP_SCREEN_ELEMENT_RETURN;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].render_cb = vElementRenderReturn;
        xElementMapScreenElementList[MAP_SCREEN_ELEMENT_RETURN].action_cb = vElementActionReturn;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
