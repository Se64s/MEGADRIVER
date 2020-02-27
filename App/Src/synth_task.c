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

#include "YM2612_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Timeout for rx a cmd */
#define SYNTH_CMD_TIMEOUT   (1000U)

/* Size of cmd buff */
#define SYNTH_TMP_CMD_SIZE  (3U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Task handler */
TaskHandle_t xSynthTaskHandle = NULL;

/* Private function prototypes -----------------------------------------------*/

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
  * @param u8CmdSize
  * @retval None
  */
static void vHandleMidiCmd(uint8_t * pu8MidiCmd, uint8_t u8CmdSize);

/* Private fuctions ----------------------------------------------------------*/

static void _init_setup(void)
{
    vCliPrintf(SYNTH_TASK_NAME, "Initial register setup");

    vYM2612_write_reg(0x22, 0x00, 0); // LFO off
    vYM2612_write_reg(0x27, 0x00, 0); // CH3 normal
    vYM2612_write_reg(0x28, 0x00, 0); //All ch off
    vYM2612_write_reg(0x28, 0x01, 0);
    vYM2612_write_reg(0x28, 0x02, 0);
    vYM2612_write_reg(0x28, 0x03, 0);
    vYM2612_write_reg(0x28, 0x04, 0);
    vYM2612_write_reg(0x28, 0x05, 0);
    vYM2612_write_reg(0x28, 0x06, 0);
    vYM2612_write_reg(0x2B, 0x00, 0); // DAC off
    vYM2612_write_reg(0x30, 0x71, 0); // DT1/MUL
    vYM2612_write_reg(0x34, 0x0d, 0);
    vYM2612_write_reg(0x38, 0x33, 0);
    vYM2612_write_reg(0x3C, 0x01, 0);
    vYM2612_write_reg(0x40, 0x23, 0); // Total level
    vYM2612_write_reg(0x44, 0x2d, 0);
    vYM2612_write_reg(0x48, 0x26, 0);
    vYM2612_write_reg(0x4C, 0x00, 0);
    vYM2612_write_reg(0x50, 0x5f, 0); // RS/AR
    vYM2612_write_reg(0x54, 0x99, 0);
    vYM2612_write_reg(0x58, 0x5f, 0);
    vYM2612_write_reg(0x5C, 0x94, 0);
    vYM2612_write_reg(0x60, 0x05, 0); // AM/D1R
    vYM2612_write_reg(0x64, 0x05, 0);
    vYM2612_write_reg(0x68, 0x05, 0);
    vYM2612_write_reg(0x6C, 0x05, 0);
    vYM2612_write_reg(0x70, 0x02, 0); // D2R
    vYM2612_write_reg(0x74, 0x02, 0);
    vYM2612_write_reg(0x78, 0x02, 0);
    vYM2612_write_reg(0x7C, 0x02, 0);
    vYM2612_write_reg(0x80, 0x11, 0); // D1L/RR
    vYM2612_write_reg(0x84, 0x11, 0);
    vYM2612_write_reg(0x88, 0x11, 0);
    vYM2612_write_reg(0x8C, 0xA6, 0);
    vYM2612_write_reg(0x90, 0x00, 0); // Propietary
    vYM2612_write_reg(0x94, 0x00, 0);
    vYM2612_write_reg(0x98, 0x00, 0);
    vYM2612_write_reg(0x9C, 0x00, 0);
    vYM2612_write_reg(0xB0, 0x32, 0); // FB algorithm
    vYM2612_write_reg(0xB4, 0xC0, 0); // Enable L-R output
    vYM2612_write_reg(0x28, 0x00, 0); // Key off
    vYM2612_write_reg(0xA4, 0x22, 0); // Set freq
    vYM2612_write_reg(0xA0, 0x69, 0);
}

static void vHandleMidiCmd(uint8_t * pu8MidiCmd, uint8_t u8CmdSize)
{
  if (u8CmdSize == 3U)
  {
    uint8_t u8MidiCmd = pu8MidiCmd[0U] & MIDI_STATUS_CMD_MASK;
    uint8_t u8MidiData0 = pu8MidiCmd[1U];
    uint8_t u8MidiData1 = pu8MidiCmd[2U];

    if (u8MidiCmd == MIDI_STATUS_NOTE_ON)
    {
        if (bYM2612_set_note(YM2612_CH_1, u8MidiData0))
        {
            vCliPrintf(SYNTH_TASK_NAME, "Key ON");
            vYM2612_key_on(YM2612_CH_1);
            bUiTaskNotify(UI_SIGNAL_SYNTH_ON);
        }
    }
    else if (u8MidiCmd == MIDI_STATUS_NOTE_OFF)
    {
        vCliPrintf(SYNTH_TASK_NAME, "Key OFF");
        vYM2612_key_off(YM2612_CH_1);
        bUiTaskNotify(UI_SIGNAL_SYNTH_OFF);
    }
  }
}

static void vSynthTaskMain( void *pvParameters )
{
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
        uint8_t pu8TmpCmd[SYNTH_TMP_CMD_SIZE] = {0};

        /* Get cmd from buffer */
        xReceivedBytes = xMessageBufferReceive(MsgBuff, (void *) pu8TmpCmd, sizeof(pu8TmpCmd), pdMS_TO_TICKS(SYNTH_CMD_TIMEOUT));

        /* Handle cmd if not empty */
        if (xReceivedBytes != 0U)
        {
          vHandleMidiCmd(pu8TmpCmd, xReceivedBytes);
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