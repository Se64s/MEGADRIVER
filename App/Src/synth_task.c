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
#define SYNTH_CMD_TIMEOUT   (1000U)

/* Size of cmd buff */
#define SYNTH_TMP_CMD_SIZE  (3U)

/* Maximun number of voices */
#define SYNTH_MAX_NUM_VOICE (YM2612_MAX_NUM_VOICE)

/* Private typedef -----------------------------------------------------------*/

/* Voice modes */
typedef enum 
{
    SYNTH_MODE_MONO = 0x00U,
    SYNTH_MODE_POLY,
    SYNTH_MODE_NOTDEF = 0xFFU
} synth_voice_mode_t;

/* Voice structure */
typedef struct
{
    uint8_t u8Note;
    uint8_t u8Velocity;
    bool bStatus;
} synth_voice_t;

/* Voice control structure */
typedef struct
{
    synth_voice_mode_t xMode;
    uint8_t u8NumVoices;
    uint8_t u8NumActVoices;
    synth_voice_t xVoiceList[SYNTH_MAX_NUM_VOICE];
    synth_voice_t xTmpVoice;
} synth_ctrl_voice_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/* Voice control structure */
static synth_ctrl_voice_t xVoiceCtrl = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Reset voice control structure
  * @param pxVoiceCtrl pointer to control structure.
  * @retval None
  */
static void vCtrlVoiceReset(synth_ctrl_voice_t * pxVoiceCtrl);

/**
  * @brief Activate voice.
  * @param pxVoiceCtrl pointer to control structure.
  * @param u8Note Note to assign.
  * @param u8Velocity Note velocity.
  * @retval None
  */
static void vCtrlVoiceOn(synth_ctrl_voice_t * pxVoiceCtrl, uint8_t u8Note, uint8_t u8Velocity);

/**
  * @brief Deactivate voice.
  * @param pxVoiceCtrl pointer to control structure.
  * @param u8Note Note to assign.
  * @param u8Velocity Note velocity.
  * @retval None
  */
static void vCtrlVoiceOff(synth_ctrl_voice_t * pxVoiceCtrl, uint8_t u8Note, uint8_t u8Velocity);

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

/**
  * @brief Handle midi cmd
  * @param pu8MidiCmd
  * @retval None
  */
static void vHandleMidiCmd(uint8_t * pu8MidiCmd);

/**
  * @brief Read SysEx message from midi lib
  * @param None
  * @retval None
  */
static void vHandleMidiSysEx(void);

/* Private fuctions ----------------------------------------------------------*/

static void vCtrlVoiceReset(synth_ctrl_voice_t * pxVoiceCtrl)
{
    pxVoiceCtrl->xMode = SYNTH_MODE_POLY;
    pxVoiceCtrl->u8NumVoices = SYNTH_MAX_NUM_VOICE;
    pxVoiceCtrl->u8NumActVoices = 0U;

    for (uint32_t u32Voice = 0; u32Voice < SYNTH_MAX_NUM_VOICE; u32Voice++)
    {
        pxVoiceCtrl->xVoiceList[u32Voice].u8Note = 0U;
        pxVoiceCtrl->xVoiceList[u32Voice].u8Velocity = 0U;
        pxVoiceCtrl->xVoiceList[u32Voice].bStatus = false;
    }

    pxVoiceCtrl->xTmpVoice.u8Note = 0U;
    pxVoiceCtrl->xTmpVoice.u8Velocity = 0U;
    pxVoiceCtrl->xTmpVoice.bStatus = false;
}

static void vCtrlVoiceOn(synth_ctrl_voice_t * pxVoiceCtrl, uint8_t u8Note, uint8_t u8Velocity)
{
    /* Check for num free voices */
    if (pxVoiceCtrl->u8NumActVoices < pxVoiceCtrl->u8NumVoices)
    {
        uint32_t u32VoiceIndex = 0xFFU;

        /* Search an active voice with the same note */
        for (uint32_t u32Voice = 0; u32Voice < pxVoiceCtrl->u8NumVoices; u32Voice++)
        {
            if ((pxVoiceCtrl->xVoiceList[u32Voice].u8Note == u8Note) && (pxVoiceCtrl->xVoiceList[u32Voice].bStatus))
            {
                u32VoiceIndex = u32Voice;
                break;
            }
        }

        /* If voice found overwrite it */
        if (u32VoiceIndex != 0xFFU)
        {
            if (bYM2612_set_note(u32VoiceIndex, u8Note))
            {
                vCliPrintf(SYNTH_TASK_NAME, "Key ON");
                vYM2612_key_on(u32VoiceIndex);
                bUiTaskNotify(UI_SIGNAL_SYNTH_ON);
            }
        }
        else
        {
            /* Search for a free slot */
            for (uint32_t u32Voice = 0; u32Voice < pxVoiceCtrl->u8NumVoices; u32Voice++)
            {
                if (!pxVoiceCtrl->xVoiceList[u32Voice].bStatus)
                {
                    u32VoiceIndex = u32Voice;
                    break;
                }
            }

            /* If free slot found, update ctrl struct */
            if (u32VoiceIndex != 0xFFU)
            {
                if (bYM2612_set_note(u32VoiceIndex, u8Note))
                {
                    /* Update struct */
                    pxVoiceCtrl->xVoiceList[u32VoiceIndex].u8Note = u8Note;
                    pxVoiceCtrl->xVoiceList[u32VoiceIndex].u8Velocity = u8Velocity;
                    pxVoiceCtrl->xVoiceList[u32VoiceIndex].bStatus = true;
                    pxVoiceCtrl->u8NumActVoices++;

                    /* Update synth */
                    vCliPrintf(SYNTH_TASK_NAME, "Key ON");
                    vYM2612_key_on(u32VoiceIndex);
                    bUiTaskNotify(UI_SIGNAL_SYNTH_ON);
                }
            }
        }
    }
    else
    {
        /* Save note in temporal slot */
        pxVoiceCtrl->xTmpVoice.u8Note = u8Note;
        pxVoiceCtrl->xTmpVoice.u8Velocity = u8Velocity;
        pxVoiceCtrl->xTmpVoice.bStatus = true;
    }
}

static void vCtrlVoiceOff(synth_ctrl_voice_t * pxVoiceCtrl, uint8_t u8Note, uint8_t u8Velocity)
{
    uint32_t u32VoiceIndex = 0xFFU;

    /* Search for active note and deactivate it */
    for (uint32_t u32Voice = 0; u32Voice < pxVoiceCtrl->u8NumVoices; u32Voice++)
    {
        if ((pxVoiceCtrl->xVoiceList[u32Voice].u8Note == u8Note) && (pxVoiceCtrl->xVoiceList[u32Voice].bStatus))
        {
            /* Update struct */
            pxVoiceCtrl->xVoiceList[u32Voice].u8Note = 0U;
            pxVoiceCtrl->xVoiceList[u32Voice].u8Velocity = 0U;
            pxVoiceCtrl->xVoiceList[u32Voice].bStatus = false;

            /* Update synth */
            vCliPrintf(SYNTH_TASK_NAME, "Key OFF");
            vYM2612_key_off(u32Voice);
            bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);

            u32VoiceIndex = u32Voice;
        }
    }

    /* Update voice number */
    if (u32VoiceIndex != 0xFFU)
    {
        pxVoiceCtrl->u8NumActVoices--;
    }

    /* Check tmp voice */
    if (pxVoiceCtrl->xTmpVoice.u8Note == u8Note)
    {
        pxVoiceCtrl->xTmpVoice.u8Note = 0U;
        pxVoiceCtrl->xTmpVoice.u8Velocity = 0U;
        pxVoiceCtrl->xTmpVoice.bStatus = false;
    }

    /* Check and restore tmp voice */
    if ((pxVoiceCtrl->u8NumActVoices < pxVoiceCtrl->u8NumVoices) && pxVoiceCtrl->xTmpVoice.bStatus)
    {
        uint8_t u8TmpNote = pxVoiceCtrl->xTmpVoice.u8Note;
        uint8_t u8TmpVelocity = pxVoiceCtrl->xTmpVoice.u8Velocity;

        pxVoiceCtrl->xTmpVoice.u8Note = 0U;
        pxVoiceCtrl->xTmpVoice.u8Velocity = 0U;
        pxVoiceCtrl->xTmpVoice.bStatus = false;

        vCtrlVoiceOn(pxVoiceCtrl, u8TmpNote, u8TmpVelocity);
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

static void vHandleMidiCmd(uint8_t * pu8MidiCmd)
{
    if (pu8MidiCmd != NULL)
    {
        uint8_t u8MidiCmd = *pu8MidiCmd++;

        if ((u8MidiCmd & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON)
        {
            uint8_t u8MidiData0 = *pu8MidiCmd++;
            uint8_t u8MidiData1 = *pu8MidiCmd++;
            vCtrlVoiceOn(&xVoiceCtrl, u8MidiData0, u8MidiData1);
        }
        else if ((u8MidiCmd & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF)
        {
            uint8_t u8MidiData0 = *pu8MidiCmd++;
            uint8_t u8MidiData1 = *pu8MidiCmd++;
            vCtrlVoiceOff(&xVoiceCtrl, u8MidiData0, u8MidiData1);
        }
    }
}

static void vHandleMidiSysEx(void)
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

    /* Init Voice control */
    vCtrlVoiceReset(&xVoiceCtrl);

    for(;;)
    {
      MessageBufferHandle_t MsgBuff = xMidiGetMessageBuffer();

      if (MsgBuff != NULL)
      {
        size_t xReceivedBytes;
        MidiMsg_t xMidiCmd = {0};

        /* Get cmd from buffer */
        xReceivedBytes = xMessageBufferReceive(MsgBuff, (void *) &xMidiCmd, sizeof(MidiMsg_t), pdMS_TO_TICKS(SYNTH_CMD_TIMEOUT));

        /* Handle cmd if not empty */
        if (xReceivedBytes == sizeof(MidiMsg_t))
        {
            if (xMidiCmd.xType == MIDI_TYPE_CMD)
            {
                vHandleMidiCmd(xMidiCmd.u8Data);
            }
            else if (xMidiCmd.xType == MIDI_TYPE_RT)
            {
            }
            else if (xMidiCmd.xType == MIDI_TYPE_SYSEX)
            {
                vHandleMidiSysEx();
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