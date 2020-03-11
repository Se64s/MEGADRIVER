/**
  ******************************************************************************
  * @file           : synth_task.c
  * @brief          : Task to handle synth chip interaction
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "synth_task.h"
#include "cli_task.h"
#include "ui_task.h"
#include "midi_task.h"

/* Private includes ----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for rx a cmd */
#define SYNTH_CMD_TIMEOUT       (1000U)

/* Size of cmd buff */
#define SYNTH_TMP_CMD_SIZE      (3U)

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Activate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOn(SynthMsg_t * pxCmdMsg);

/**
  * @brief Deactivate voice.
  * @param pxCmdMsg pointer to synth cmd.
  * @retval None
  */
static void vCmdVoiceOff(SynthMsg_t * pxCmdMsg);

/**
  * @brief Deactivate all voices.
  * @retval None
  */
static void vCmdVoiceOffAll(void);

/**
  * @brief Read SysEx message from midi lib
  * @param None
  * @retval None
  */
static void vCmdSysEx(void);

/**
  * @brief Initial YM2612 setup
  * @retval None
  */
static void _init_setup(void);

/**
  * @brief Main task loop
  * @param pvParameters function paramters
  * @retval None
  */
static void vSynthTaskMain(void *pvParameters);

/* Private fuctions ----------------------------------------------------------*/

static void vCmdVoiceOn(SynthMsg_t * pxCmdMsg)
{
    if (pxCmdMsg->xType == SYNTH_CMD_NOTE_ON)
    {
        uint8_t u8VoiceChannel = pxCmdMsg->u8Data[0U];
        uint8_t u8Note = pxCmdMsg->u8Data[1U];

        /* Check voice range */
        if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
        {
            if (bYM2612_set_note(u8VoiceChannel, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "Key ON: %d", u8VoiceChannel);
                vYM2612_key_on(u8VoiceChannel);
                bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
            }
        }
    }
}

static void vCmdVoiceOff(SynthMsg_t * pxCmdMsg)
{
    if (pxCmdMsg->xType == SYNTH_CMD_NOTE_OFF)
    {
        uint8_t u8VoiceChannel = pxCmdMsg->u8Data[0U];
        uint8_t u8Note = pxCmdMsg->u8Data[1U];

        /* Check voice range */
        if (u8VoiceChannel < SYNTH_MAX_NUM_VOICE)
        {
            if (bYM2612_set_note(u8VoiceChannel, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "Key OFF: %d", u8VoiceChannel);
                vYM2612_key_off(u8VoiceChannel);
                bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
            }
        }
    }
}

static void vCmdVoiceOffAll(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Clear ALL voices");
    for (uint8_t u8VoiceIndex = 0U; u8VoiceIndex < SYNTH_MAX_NUM_VOICE; u8VoiceIndex++)
    {
        vYM2612_key_off(u8VoiceIndex);
    }
    bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
}

static void vCmdMidiSysEx(void)
{
    uint32_t u32SysExLenData = 0U;
    uint8_t * pu8SysExData;

    if (midi_get_sysex_data(&pu8SysExData, &u32SysExLenData) == midiOk)
    {
        if (u32SysExLenData >= SYNTH_LEN_MIN_SYSEX_CMD)
        {
            SynthSysExCmd_t * pxSysExCmd = (SynthSysExCmd_t *)pu8SysExData;

            if ((pxSysExCmd->xSysExCmd) == SYNTH_SYSEX_CMD_SET_PRESET && (u32SysExLenData == SYNTH_LEN_SET_REG_CMD))
            {
                xFmDevice_t * xPresetData = &pxSysExCmd->pu8CmdData;
                vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD SET PRESET");
                vYM2612_set_reg_preset(xPresetData);
            }
        }
        else
        {
            vCliPrintf(SYNTH_TASK_NAME, "SysEx CMD too short");
        }
    }
}

static void _init_setup(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Initial register setup");

    vYM2612_write_reg(0x22, 0x00, 0); // LFO off

    vYM2612_write_reg(0x27, 0x00, 0); // CH3 normal

    vYM2612_write_reg(0x28, 0x00, 0); // Key Off CH0
    vYM2612_write_reg(0x28, 0x01, 0); // Key Off CH1
    vYM2612_write_reg(0x28, 0x02, 0); // Key Off CH2
    vYM2612_write_reg(0x28, 0x04, 0); // Key Off CH3
    vYM2612_write_reg(0x28, 0x05, 0); // Key Off CH4
    vYM2612_write_reg(0x28, 0x06, 0); // Key Off CH5

    vYM2612_write_reg(0x2B, 0x00, 0); // DAC off

    /* Setup each voice */
    for (uint32_t u32VoiceIndex = 0U; u32VoiceIndex < SYNTH_MAX_NUM_VOICE; u32VoiceIndex++)
    {
        uint8_t u8RegSelection = u32VoiceIndex / 3U;
        uint8_t u8RegOffset = u32VoiceIndex % 3U;

        // Operator 1
        vYM2612_write_reg(0x30 + u8RegOffset, 0x71, u8RegSelection); //DT1/Mul
        vYM2612_write_reg(0x40 + u8RegOffset, 0x23, u8RegSelection); //Total Level
        vYM2612_write_reg(0x50 + u8RegOffset, 0x5F, u8RegSelection); //RS/AR
        vYM2612_write_reg(0x60 + u8RegOffset, 0x05, u8RegSelection); //AM/D1R
        vYM2612_write_reg(0x70 + u8RegOffset, 0x02, u8RegSelection); //D2R
        vYM2612_write_reg(0x80 + u8RegOffset, 0x11, u8RegSelection); //D1L/RR
        vYM2612_write_reg(0x90 + u8RegOffset, 0x00, u8RegSelection); //SSG EG

        //Operator 2
        vYM2612_write_reg(0x34 + u8RegOffset, 0x0D, u8RegSelection); //DT1/Mul
        vYM2612_write_reg(0x44 + u8RegOffset, 0x2D, u8RegSelection); //Total Level
        vYM2612_write_reg(0x54 + u8RegOffset, 0x99, u8RegSelection); //RS/AR
        vYM2612_write_reg(0x64 + u8RegOffset, 0x05, u8RegSelection); //AM/D1R
        vYM2612_write_reg(0x74 + u8RegOffset, 0x02, u8RegSelection); //D2R
        vYM2612_write_reg(0x84 + u8RegOffset, 0x11, u8RegSelection); //D1L/RR
        vYM2612_write_reg(0x94 + u8RegOffset, 0x00, u8RegSelection); //SSG EG

        //Operator 3
        vYM2612_write_reg(0x38 + u8RegOffset, 0x33, u8RegSelection); //DT1/Mul
        vYM2612_write_reg(0x48 + u8RegOffset, 0x26, u8RegSelection); //Total Level
        vYM2612_write_reg(0x58 + u8RegOffset, 0x5F, u8RegSelection); //RS/AR
        vYM2612_write_reg(0x68 + u8RegOffset, 0x05, u8RegSelection); //AM/D1R
        vYM2612_write_reg(0x78 + u8RegOffset, 0x02, u8RegSelection); //D2R
        vYM2612_write_reg(0x88 + u8RegOffset, 0x11, u8RegSelection); //D1L/RR
        vYM2612_write_reg(0x98 + u8RegOffset, 0x00, u8RegSelection); //SSG EG

        //Operator 4
        vYM2612_write_reg(0x3C + u8RegOffset, 0x01, u8RegSelection); //DT1/Mul
        vYM2612_write_reg(0x4C + u8RegOffset, 0x00, u8RegSelection); //Total Level
        vYM2612_write_reg(0x5C + u8RegOffset, 0x94, u8RegSelection); //RS/AR
        vYM2612_write_reg(0x6C + u8RegOffset, 0x07, u8RegSelection); //AM/D1R
        vYM2612_write_reg(0x7C + u8RegOffset, 0x02, u8RegSelection); //D2R
        vYM2612_write_reg(0x8C + u8RegOffset, 0xA6, u8RegSelection); //D1L/RR
        vYM2612_write_reg(0x9C + u8RegOffset, 0x00, u8RegSelection); //SSG EG

        vYM2612_write_reg(0xB0 + u8RegOffset, 0x32, u8RegSelection); // Ch FB/Algo
        vYM2612_write_reg(0xB4 + u8RegOffset, 0xC0, u8RegSelection); // Enable L-R output
        vYM2612_write_reg(0xA4 + u8RegOffset, 0x22, u8RegSelection); // Set Freq MSB
        vYM2612_write_reg(0xA0 + u8RegOffset, 0x69, u8RegSelection); // Freq LSB
    }
}

static void vSynthTaskMain( void *pvParameters )
{
    /* Init delay to for pow stabilization */
    vTaskDelay(pdMS_TO_TICKS(500U));

    /* Init YM2612 resources */
    (void)xYM2612_init();

    /* Show init msg */
    vCliPrintf(SYNTH_TASK_NAME, "Init");

    /* Basic register init */
    _init_setup();

    for(;;)
    {
      MessageBufferHandle_t MsgBuff = xMidiGetMessageBuffer();

      if (MsgBuff != NULL)
      {
        size_t xReceivedBytes;
        SynthMsg_t xSynthCmd = {0};

        /* Get cmd from buffer */
        xReceivedBytes = xMessageBufferReceive(MsgBuff, (void *) &xSynthCmd, sizeof(SynthMsg_t), pdMS_TO_TICKS(SYNTH_CMD_TIMEOUT));

        /* Handle cmd if not empty */
        if (xReceivedBytes == sizeof(SynthMsg_t))
        {
            if (xSynthCmd.xType == SYNTH_CMD_NOTE_ON)
            {
                vCmdVoiceOn(&xSynthCmd);
            }
            else if (xSynthCmd.xType == SYNTH_CMD_NOTE_OFF)
            {
                vCmdVoiceOff(&xSynthCmd);
            }
            else if (xSynthCmd.xType == SYNTH_CMD_NOTE_OFF_ALL)
            {
                vCmdVoiceOffAll();
            }
            else if (xSynthCmd.xType == SYNTH_CMD_SYSEX)
            {
                vCmdMidiSysEx();
            }
        }
      }
    }
}

/* Public fuctions -----------------------------------------------------------*/

bool bSynthTaskInit(void)
{
    bool retval = false;

    /* Create task */
    xTaskCreate(vSynthTaskMain, SYNTH_TASK_NAME, SYNTH_TASK_STACK, NULL, SYNTH_TASK_PRIO, &xSynthTaskHandle);

    /* Check resources */
    if (xSynthTaskHandle != NULL)
    {
        retval = true;
    }
    return(retval);
}

bool bSynthTaskNotify(uint32_t u32Event)
{
    bool bRetval = false;
    /* Check if task has been init */
    if (xSynthTaskHandle != NULL)
    {
      xTaskNotify(xSynthTaskHandle, u32Event, eSetBits);
      bRetval = true;
    }
    return bRetval;
}

/*****END OF FILE****/