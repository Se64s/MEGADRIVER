/**
  ******************************************************************************
  * @file           : midi_task.h
  * @brief          : Task to handle midi events
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIDI_TASK_H
#define __MIDI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "midi_lib.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Midi app parameter */
#define MIDI_APP_MAX_BANK               ((uint8_t)SYNTH_PRESET_SOURCE_MAX)
#define MIDI_APP_BANK_DEFAULT           ((uint8_t)SYNTH_PRESET_SOURCE_DEFAULT)
#define MIDI_APP_BANK_USER              ((uint8_t)SYNTH_PRESET_SOURCE_USER)

/* Task parameters */
#define MIDI_TASK_NAME                  "MIDI"
#define MIDI_TASK_STACK                 (254U)
#define MIDI_TASK_PRIO                  (1U)

/* Midi msg parameter */
#define MIDI_MSG_DATA_LEN               (4U)

/* Signal definition */
#define MIDI_SIGNAL_RX_DATA             (1UL << 0)
#define MIDI_SIGNAL_ERROR               (1UL << 2)
#define MIDI_SIGNAL_CHANGE_MODE_POLY    (1UL << 3)
#define MIDI_SIGNAL_CHANGE_MODE_MONO    (1UL << 4)

/* Exported types ------------------------------------------------------------*/

/* Midi message type definition */
typedef enum
{
  MIDI_TYPE_CMD = 0x00U,
  MIDI_TYPE_RT,
  MIDI_TYPE_SYSEX,
  MIDI_TYPE_NO_DEF = 0xFFU
} MidiMsgType_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Get current midi note set on output channel.
  * @param u8Channel channel number to read.
  * @retval midi note in channel or 0xff if input channel not valid.
  */
uint8_t xMidiTaskGetNote(uint8_t u8Channel);

/**
  * @brief Get parameter from midi conf
  * @retval requested parameter
  */
midiMode_t xMidiTaskGetMode(void);

/**
  * @brief Get parameter from midi conf
  * @retval requested parameter
  */
uint8_t u8MidiTaskGetChannel(void);

/**
  * @brief Get parameter from midi conf
  * @retval requested parameter
  */
uint8_t u8MidiTaskGetBank(void);

/**
  * @brief Get parameter from midi conf
  * @retval requested parameter
  */
uint8_t u8MidiTaskGetProgram(void);

/**
  * @brief Set parameter from midi conf
  * @param xNewMode new value to set.
  * @retval operation result
  */
bool bMidiTaskSetMode(midiMode_t xNewMode);

/**
  * @brief Set parameter from midi conf
  * @param u8NewChannel new value to set.
  * @retval operation result
  */
bool bMidiTaskSetChannel(uint8_t u8NewChannel);

/**
  * @brief Set parameter from midi conf
  * @param u8NewBank new value to set.
  * @retval operation result
  */
bool bMidiTaskSetBank(uint8_t u8NewBank);

/**
  * @brief Set parameter from midi conf
  * @param u8NewBank new value to set.
  * @retval operation result
  */
bool bMidiTaskSetProgram(uint8_t u8NewProgram);

/**
  * @brief Save current setup into flash
  * @retval operation result
  */
bool bMidiTaskSaveCfg(void);

/**
  * @brief Init resources for MIDI tasks
  * @retval operation result, true for correct creation, false for error
  */
bool bMidiTaskInit(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool bMidiTaskNotify(uint32_t u32Event);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_TASK_H */

/*****END OF FILE****/
