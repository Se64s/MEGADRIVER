/**
  ******************************************************************************
  * @file           : ui_screen_test.c
  * @brief          : UI definition for test screen.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "ui_screen_test.h"

#include "encoder_driver.h"
#include "ui_task.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Defined elements for Test screen */
typedef enum
{
    TEST_SCREEN_ELEMENT_ENCODER = 0,
    TEST_SCREEN_ELEMENT_SWITCH,
    TEST_SCREEN_ELEMENT_SYNTH,
    TEST_SCREEN_ELEMENT_MIDI,
    TEST_SCREEN_ELEMENT_CV1,
    TEST_SCREEN_ELEMENT_CV2,
    TEST_SCREEN_ELEMENT_CV3,
    TEST_SCREEN_ELEMENT_CV4,
    TEST_SCREEN_ELEMENT_LAST
} eTestScreenEment_t;

/* Private define ------------------------------------------------------------*/

#define UI_OUT_FROM_SCREEN              (0xFFFFFFFFU)
#define NUM_RENDER_ELEMENT              (5U)

/* Max num of elements */
#define UI_NUM_ELEMENT                  (8U)

/* Max len for element names */
#define MAX_LEN_NAME                    (16U)

#define NAME_FORMAT_SCREEN              "TEST UI"

#define NAME_FORMAT_ENCODER             "ENC    %d"
#define NAME_FORMAT_SWITCH              "SW     %01d"
#define NAME_FORMAT_SYNTH               "SYNTH  %s"
#define NAME_FORMAT_MIDI                "MIDI   %02X-%02X-%02X"
#define NAME_FORMAT_CV1                 "CV1    %05d"
#define NAME_FORMAT_CV2                 "CV2    %05d"
#define NAME_FORMAT_CV3                 "CV3    %05d"
#define NAME_FORMAT_CV4                 "CV4    %05d"

/* Render position defines */
#define UI_OFFSET_SCREEN_X              (0U)
#define UI_OFFSET_SCREEN_Y              (10U)
#define UI_OFFSET_LINE_SCREEN_Y         (2U)

#define UI_OFFSET_ELEMENT_X             (0U)
#define UI_OFFSET_ELEMENT_Y             (18U)
#define UI_OFFSET_INTRA_ELEMENT_Y       (2U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

char pcScreenTestName[] = "Test Screen Menu";

ui_element_t xElementTestScreenElement[TEST_SCREEN_ELEMENT_LAST];

char pcTestMainEncoderName[MAX_LEN_NAME];
char pcTestMainSwitchName[MAX_LEN_NAME];
char pcTestMainSynthName[MAX_LEN_NAME];
char pcTestMainMidiName[MAX_LEN_NAME];
char pcTestMainCv1Name[MAX_LEN_NAME];
char pcTestMainCv2Name[MAX_LEN_NAME];
char pcTestMainCv3Name[MAX_LEN_NAME];
char pcTestMainCv4Name[MAX_LEN_NAME];

/* Private function prototypes -----------------------------------------------*/

/* Handle selection print */
static void vDrawSelectBox(void * pxDisplayHandler, bool bSelect, uint8_t u8SelectBoxY);

/* Get draw index */
static uint32_t u32GetDrawIndexY(void * pxDisplayHandler, uint32_t u32RenderIndex, uint32_t u32ElementIndex);

/* Screen render function */
static void vScreenTestRender(void * pxDisplayHandler, void * pvScreen);

/* Element render function */
static void vElementEncoderRender(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementSwitchRender(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementSynthRender(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementMidiRender(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementCv1Render(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementCv2Render(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementCv3Render(void * pxDisplayHandler, void * pvScreen, void * pvElement);
static void vElementCv4Render(void * pxDisplayHandler, void * pvScreen, void * pvElement);

/* Element action function */
static void vElementEncoderAction(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementSwitchAction(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementSynthAction(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementMidiAction(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementCv1Action(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementCv2Action(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementCv3Action(void * pvScreen, void * pvElement, void * pvEventData);
static void vElementCv4Action(void * pvScreen, void * pvElement, void * pvEventData);

/* Private user code ---------------------------------------------------------*/

/* Print help functions */
static void vDrawSelectBox(void * pxDisplayHandler, bool bSelect, uint8_t u8SelectBoxY)
{
    u8g2_t * pxDispHand = pxDisplayHandler;

    if (bSelect && (pxDispHand != NULL))
    {
        uint8_t u8BoxX = 0U;
        uint8_t u8BoxY = u8SelectBoxY - (u8g2_GetMaxCharHeight(pxDispHand) + UI_OFFSET_INTRA_ELEMENT_Y);
        uint8_t u8BoxW = u8g2_GetDisplayWidth(pxDispHand);
        uint8_t u8BoxH = u8g2_GetMaxCharWidth(pxDispHand) + UI_OFFSET_INTRA_ELEMENT_Y;

        u8g2_DrawBox(pxDispHand, u8BoxX, u8BoxY, u8BoxW, u8BoxH);
    }
}

static uint32_t u32GetDrawIndexY(void * pxDisplayHandler, uint32_t u32RenderIndex, uint32_t u32ElementIndex)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    uint32_t u32IndY = 0U;

    if (pxDisplayHandler != NULL)
    {
        /* Compute element offset */
        if (u32RenderIndex <= u32ElementIndex)
        {
            uint32_t u32PrintIndex = u32ElementIndex - u32RenderIndex;
            u32IndY += u8g2_GetMaxCharHeight(pxDispHand);
            u32IndY += u32PrintIndex * (u8g2_GetMaxCharHeight(pxDispHand) + UI_OFFSET_INTRA_ELEMENT_Y);
        }
    }

    return u32IndY;
}

/* Element render functions */

static void vScreenTestRender(void * pxDisplayHandler, void * pvScreen)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = 0U;
    uint32_t u32IndY = UI_OFFSET_SCREEN_Y;
    uint8_t u8LineX = 0U;
    uint8_t u8LineY = UI_OFFSET_SCREEN_Y + UI_OFFSET_LINE_SCREEN_Y;
    uint8_t u8LineWith = 0U;

    u8LineWith = u8g2_GetDisplayWidth(pxDispHand);

    /* Prepare data on buffer */
    sprintf(pxScreen->pcName, NAME_FORMAT_SCREEN);

    /* Set font */
    u8g2_SetFontMode(pxDispHand, 1);
    u8g2_SetDrawColor(pxDispHand, 2);
    u8g2_SetFont(pxDispHand, u8g2_font_amstrad_cpc_extended_8r);

    /* Draw actions on screen */
    u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxScreen->pcName);
    u8g2_DrawHLine(pxDispHand, u8LineX, u8LineY, u8LineWith);
}

static void vElementEncoderRender(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Get data to print */
        uint32_t u32EncData = 0U;
        ENCODER_getCount(ENCODER_ID_0, &u32EncData);

        /* Prepare data on buffer */
        sprintf(pxElement->pcName, NAME_FORMAT_ENCODER, (uint8_t)u32EncData);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementSwitchRender(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Prepare data on buffer */
        sprintf(pxElement->pcName, NAME_FORMAT_SWITCH, ENCODER_getSwState(ENCODER_ID_0));

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementSynthRender(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        uint32_t u32Arg = 0;
        if (u32Arg != 0)
        {
            sprintf(pxElement->pcName, NAME_FORMAT_SYNTH, "ON");
        }
        else
        {
            sprintf(pxElement->pcName, NAME_FORMAT_SYNTH, "OFF");
        }

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementMidiRender(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Prepare data on buffer */
        uint8_t u8MidiStatus = 0xAA;
        uint8_t u8MidiData1 = 0xBB;
        uint8_t u8MidiData2 = 0xCC;
        sprintf(pxElement->pcName, NAME_FORMAT_MIDI, u8MidiStatus, u8MidiData1, u8MidiData2);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementCv1Render(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Prepare data on buffer */
        uint16_t u16CvData = 1;
        sprintf(pxElement->pcName, NAME_FORMAT_CV1, u16CvData);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementCv2Render(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Prepare data on buffer */
        uint16_t u16CvData = 2;
        sprintf(pxElement->pcName, NAME_FORMAT_CV2, u16CvData);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementCv3Render(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = UI_OFFSET_ELEMENT_Y;

    /* Compute element offset */
    u32IndY += u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        /* Prepare data on buffer */
        uint16_t u16CvData = 3;
        sprintf(pxElement->pcName, NAME_FORMAT_CV3, u16CvData);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

static void vElementCv4Render(void * pxDisplayHandler, void * pvScreen, void * pvElement)
{
    u8g2_t * pxDispHand = pxDisplayHandler;
    ui_element_t * pxElement = pvElement;
    ui_screen_t * pxScreen = pvScreen;
    uint32_t u32IndX = UI_OFFSET_ELEMENT_X;
    uint32_t u32IndY = 0U;

    /* Compute element offset */
    u32IndY = u32GetDrawIndexY(pxDisplayHandler, pxScreen->u32ElementRenderIndex, pxElement->u32Index);

    if ((u32IndY < u8g2_GetDisplayHeight(pxDispHand)) && (u32IndY > UI_OFFSET_ELEMENT_Y))
    {
        u32IndY += UI_OFFSET_ELEMENT_Y;

        /* Prepare data on buffer */
        uint16_t u16CvData = 4;
        sprintf(pxElement->pcName, NAME_FORMAT_CV4, u16CvData);

        /* Print selection ico */
        vDrawSelectBox(pxDisplayHandler, pxElement->bSelected, (uint8_t)u32IndY);

        u8g2_DrawStr(pxDispHand, (uint8_t)u32IndX, (uint8_t)u32IndY, pxElement->pcName);
    }
}

/* Element action functions */

static void vElementEncoderAction(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;

    if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_CW))
    {
        uint32_t enc_count = 0;
        ENCODER_getCount(ENCODER_ID_0, &enc_count);
        cli_printf(UI_TASK_NAME, "Encoder CW event: %d", enc_count);

        uint32_t u32ElementIndex = pxScreenHandler->u32ElementSelectionIndex;
        uint32_t u32RenderIndex = pxScreenHandler->u32ElementRenderIndex;

        if (u32ElementIndex != (pxScreenHandler->u32ElementNumber - 1))
        {
            pxScreenHandler->pxElementList[u32ElementIndex++].bSelected = false;
            pxScreenHandler->pxElementList[u32ElementIndex].bSelected = true;
            pxScreenHandler->u32ElementSelectionIndex = u32ElementIndex;
        }

        /* Update render index - DOWN*/
        if ((u32ElementIndex - u32RenderIndex) >= NUM_RENDER_ELEMENT)
        {
            u32RenderIndex++;
            pxScreenHandler->u32ElementRenderIndex = u32RenderIndex;
        }
    }
    else if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_CCW))
    {
        uint32_t enc_count = 0;
        ENCODER_getCount(ENCODER_ID_0, &enc_count);
        cli_printf(UI_TASK_NAME, "Encoder CCW event: %d", enc_count);

        uint32_t u32ElementIndex = pxScreenHandler->u32ElementSelectionIndex;
        uint32_t u32RenderIndex = pxScreenHandler->u32ElementRenderIndex;

        if (u32ElementIndex != 0U)
        {
            pxScreenHandler->pxElementList[u32ElementIndex--].bSelected = false;
            pxScreenHandler->pxElementList[u32ElementIndex].bSelected = true;
            pxScreenHandler->u32ElementSelectionIndex = u32ElementIndex;
        }

        /* Update render index - UP*/
        if (u32ElementIndex < u32RenderIndex)
        {
            u32RenderIndex--;
            pxScreenHandler->u32ElementRenderIndex = u32RenderIndex;
        }
    }
}

static void vElementSwitchAction(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
    
    if (CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_SET) || CHECK_SIGNAL(*pu32EventData, UI_SIGNAL_ENC_UPDATE_SW_RESET))
    {
        cli_printf(UI_TASK_NAME, "Encoder SW state %d", ENCODER_getSwState(ENCODER_ID_0));
    }
}

static void vElementSynthAction(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

static void vElementMidiAction(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

static void vElementCv1Action(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

static void vElementCv2Action(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

static void vElementCv3Action(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

static void vElementCv4Action(void * pvScreen, void * pvElement, void * pvEventData)
{
    ui_screen_t * pxScreenHandler = pvScreen;
    ui_element_t * pxElementHandler = pvElement;
    uint32_t * pu32EventData = pvEventData;
}

/* Public user code ----------------------------------------------------------*/

ui_status_t UI_screen_test_init(ui_screen_t * pxScreenHandler)
{
    ui_status_t retval = UI_STATUS_ERROR;

    if (pxScreenHandler != NULL)
    {
        /* Populate screen elements */
        pxScreenHandler->pcName = pcScreenTestName;
        pxScreenHandler->u32ElementRenderIndex = 0;
        pxScreenHandler->u32ElementSelectionIndex = 0;
        pxScreenHandler->pxElementList = xElementTestScreenElement;
        pxScreenHandler->u32ElementNumber = TEST_SCREEN_ELEMENT_LAST;
        pxScreenHandler->render_cb = vScreenTestRender;

        /* Init elements */
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_ENCODER].pcName = pcTestMainEncoderName;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_ENCODER].bSelected = true;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_ENCODER].u32Index = TEST_SCREEN_ELEMENT_ENCODER;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_ENCODER].render_cb = vElementEncoderRender;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_ENCODER].action_cb = vElementEncoderAction;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SWITCH].pcName = pcTestMainSwitchName;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SWITCH].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SWITCH].u32Index = TEST_SCREEN_ELEMENT_SWITCH;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SWITCH].render_cb = vElementSwitchRender;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SWITCH].action_cb = vElementSwitchAction;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SYNTH].pcName = pcTestMainSynthName;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SYNTH].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SYNTH].u32Index = TEST_SCREEN_ELEMENT_SYNTH;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SYNTH].render_cb = vElementSynthRender;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_SYNTH].action_cb = vElementSynthAction;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_MIDI].pcName = pcTestMainMidiName;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_MIDI].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_MIDI].u32Index = TEST_SCREEN_ELEMENT_MIDI;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_MIDI].render_cb = vElementMidiRender;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_MIDI].action_cb = vElementMidiAction;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV1].pcName = pcTestMainCv1Name;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV1].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV1].u32Index = TEST_SCREEN_ELEMENT_CV1;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV1].render_cb = vElementCv1Render;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV1].action_cb = vElementCv1Action;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV2].pcName = pcTestMainCv2Name;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV2].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV2].u32Index = TEST_SCREEN_ELEMENT_CV2;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV2].render_cb = vElementCv2Render;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV2].action_cb = vElementCv2Action;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV3].pcName = pcTestMainCv3Name;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV3].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV3].u32Index = TEST_SCREEN_ELEMENT_CV3;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV3].render_cb = vElementCv3Render;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV3].action_cb = vElementCv3Action;

        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV4].pcName = pcTestMainCv4Name;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV4].bSelected = false;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV4].u32Index = TEST_SCREEN_ELEMENT_CV4;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV4].render_cb = vElementCv4Render;
        xElementTestScreenElement[TEST_SCREEN_ELEMENT_CV4].action_cb = vElementCv4Action;

        retval = UI_STATUS_OK;
    }

    return retval;
}

/*****END OF FILE****/
