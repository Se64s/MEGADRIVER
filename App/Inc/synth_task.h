/**
  ******************************************************************************
  * @file           : synth_task.h
  * @brief          : Task to handle cli actions and serial debug prints
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYNTH_TASK_H
#define __SYNTH_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "YM2612_driver.h"

#include "synth_app_data.h"
#include "synth_app_data_const.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define SYNTH_TASK_NAME                     "SYNTH"
#define SYNTH_TASK_STACK                    (256U)
#define SYNTH_TASK_PRIO                     (2U)

/* SysEx CMD parameters */
#define SYNTH_LEN_VENDOR_ID                 (3U)
#define SYNTH_LEN_PRESET_NAME               (SYNTH_APP_DATA_LEN_PRESET_NAME)
#define SYNTH_LEN_PRESET_CODED_NAME         (30U)
#define SYNTH_LEN_MIN_SYSEX_CMD             (4U)
#define SYNTH_LEN_SET_REG_CMD               (300U)
#define SYNTH_LEN_SAVE_PRESET_CMD           (331U)
#define SYNTH_LEN_LOAD_PRESET_CMD           (5U)
#define SYNTH_LEN_LOAD_DEFAULT_PRESET_CMD   (5U)

/* Synth message parameters */
#define SYNTH_LEN_MIDI_MSG                  (3U)

/* Maximun number of voices */
#define SYNTH_MAX_NUM_VOICE                 (YM2612_MAX_NUM_VOICE)

/* Maximun number of user presets */
#define SYNTH_MAX_NUM_USER_PRESET           (SYNTH_APP_DATA_NUM_PRESETS)

/* Exported types ------------------------------------------------------------*/

/** Synth commands */
typedef enum
{
    SYNTH_CMD_NOTE_ON = 0x00U,
    SYNTH_CMD_NOTE_OFF,
    SYNTH_CMD_NOTE_OFF_ALL,
    SYNTH_CMD_NO_DEF = 0xFFU
} SynthMsgType_t;

/** SysEx defined cmd */
typedef enum
{
  SYNTH_SYSEX_CMD_SET_PRESET = 0x00U,
  SYNTH_SYSEX_CMD_SAVE_PRESET = 0x01U,
  SYNTH_SYSEX_CMD_LOAD_PRESET = 0x02U,
  SYNTH_SYSEX_CMD_LOAD_DEFAULT_PRESET = 0x03U,
  SYNTH_SYSEX_CMD_NO_DEF = 0x1FU
} SynthSysExCmdDef_t;

/** SysEx defined cmd */
typedef enum
{
  SYNTH_PRESET_SOURCE_DEFAULT = 0U,
  SYNTH_PRESET_SOURCE_USER,
  SYNTH_PRESET_SOURCE_MAX
} SynthPresetSource_t;

/** Synth task defined events */
typedef enum
{
  SYNTH_EVENT_MIDI_MSG = 0U,
  SYNTH_EVENT_MIDI_SYSEX_MSG,
  SYNTH_EVENT_NOTE_ON_OFF,
  SYNTH_EVENT_CHANGE_NOTE,
  SYNTH_EVENT_MOD_PARAM,
  SYNTH_EVENT_NOT_DEF = 0xFFU
} SynthEventType_t;

/** Payload for MIDI msg event */
typedef struct
{
  SynthMsgType_t xType;
  uint8_t u8Data[SYNTH_LEN_MIDI_MSG];
} SynthEventPayloadMidi_t;

/** Payload for SYSEX MIDI msg event */
typedef struct
{
  SynthMsgType_t xType;
  uint32_t u32Len;
  uint8_t * pu8Data;
} SynthEventPayloadMidiSysEx_t;

/** Payload for Note On Off */
typedef struct
{
  uint8_t u8VoiceId;
  bool bGateState;
} SynthEventPayloadNoteOnOff_t;

/** Payload for Change Note */
typedef struct
{
  uint8_t u8VoiceId;
  uint8_t u8Note;
} SynthEventPayloadChangeNote_t;

/** Union definitions with all event payload */
typedef union
{
  SynthEventPayloadMidi_t xMidi;
  SynthEventPayloadMidiSysEx_t xMidiSysEx;
  SynthEventPayloadNoteOnOff_t xNoteOnOff;
  SynthEventPayloadChangeNote_t xChangeNote;
} SynthPayload_t;

/** SysEx command format */
typedef struct 
{
  uint8_t pu8VendorId[SYNTH_LEN_VENDOR_ID];
  SynthSysExCmdDef_t xSysExCmd;
  uint8_t * pu8CmdData;
} SynthSysExCmd_t;

/* Payload format for save preset cmd*/
typedef struct 
{
  uint8_t u8Position;
  uint8_t u8CodedName[SYNTH_LEN_PRESET_CODED_NAME];
  xFmDevice_t xRegData;
} SynthSysExCmdSavePreset_t;

/* Payload format for load preset cmd*/
typedef struct 
{
  uint8_t u8Position;
} SynthSysExCmdLoadPreset_t;

/** Synth Event definition */
typedef struct
{
  SynthEventType_t eType;
  SynthPayload_t uPayload;
} SynthEvent_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Load flahs stored preset
  * @param u8PresetSource preset source, default or user
  * @param u8PresetId preset id.
  * @retval operation result, true for correct load, false for error
  */
bool bSynthLoadPreset(SynthPresetSource_t u8PresetSource, uint8_t u8PresetId);

/**
  * @brief Save user preset
  * @param pxPreset pointer to preset to save.
  * @param u8PresetId preset position id.
  * @retval operation result, true for correct save action, false for error.
  */
bool bSynthSaveUserPreset(xFmDevice_t * pxPreset, uint8_t u8PresetId);

/**
  * @brief Init resources for SYNTH tasks
  * @retval operation result, true for correct creation, false for error
  */
bool bSynthTaskInit(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool bSynthTaskNotify(uint32_t u32Event);

/**
  * @brief Get task event queue handler.
  * @retval Queue handler in case of queue init, NULL in other case.
  */
QueueHandle_t pxSynthTaskGetQueue(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYNTH_TASK_H */

/*****END OF FILE****/
