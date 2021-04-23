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

#include "sys_rtos.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define MIDI_TASK_NAME                  "MIDI"
#define MIDI_TASK_STACK                 ( 512U )
#define MIDI_TASK_PRIO                  ( 4U )
#define MIDI_TASK_INIT_DELAY            ( 500U )

/* Midi msg parameter */
#define MIDI_MSG_DATA_LEN               ( 4U )

/* Signal definition */
#define MIDI_SIGNAL_RX_DATA             ( 1UL << 0 )
#define MIDI_SIGNAL_ERROR               ( 1UL << 2 )
#define MIDI_SIGNAL_CMD_IN              ( 1UL << 3 )
#define MIDI_SIGNAL_ALL                 ( 0xFFFFFFFFU )

/* Extended debug output */
// #define MIDI_DBG_STATS
// #define MIDI_DBG_VERBOSE

/* Exported types ------------------------------------------------------------*/

/** Midi Cmd Type definition */
typedef enum
{
    MIDI_CMD_SET_MODE = 0x00U,
    MIDI_CMD_SET_CH,
    MIDI_CMD_SET_PRESET,
    MIDI_CMD_SAVE_MIDI_CFG,
    MIDI_CMF_NOT_DEF = 0xFFU,
} MidiCmdType_t;

/** SysEx defined cmd */
typedef enum
{
    MIDI_SYSEX_CMD_SAVE_PRESET = 0x00U,
    MIDI_SYSEX_CMD_LOAD_PRESET = 0x01U,
    MIDI_SYSEX_CMD_NO_DEF = 0x1FU
} MidiSysExCmdDef_t;

/* Command payload definition*/

typedef struct MidiCmdTaskPayloadSetMode
{
    uint8_t u8Mode;
} MidiCmdTaskPayloadSetMode_t;

typedef struct MidiCmdTaskPayloadSetCh
{
    uint8_t u8Channel;
} MidiCmdTaskPayloadSetCh_t;

typedef struct MidiCmdTaskPayloadSetPreset
{
    uint8_t u8Bank;
    uint8_t u8Program;
} MidiCmdTaskPayloadSetPreset_t;

/** Union definitions with all event payload */
typedef union MidiCmdTaskPayload
{
    MidiCmdTaskPayloadSetMode_t xSetMode;
    MidiCmdTaskPayloadSetCh_t xSetCh;
    MidiCmdTaskPayloadSetPreset_t xSetPreset;
} MidiCmdTaskPayload_t;

/** Midi taks cmd definition */
typedef struct MidiTaskCmd
{
    MidiCmdType_t eCmd;
    MidiCmdTaskPayload_t uPayload;
} MidiTaskCmd_t;

/** Midi param id definition */
typedef enum MidiParamType
{
    MIDI_PARAM_MODE = 0x00U,
    MIDI_PARAM_CHANNEL,
    MIDI_PARAM_BANK,
    MIDI_PARAM_PROGRAM,
    MIDI_PARAM_NOT_DEF = 0xFFU,
} MidiParamType_t;

/** Midi param data definition */
typedef union MidiParamData
{
    uint8_t u8Mode;
    uint8_t u8Channel;
    uint8_t u8Bank;
    uint8_t u8Program;
} MidiParamData_t;

/** Midi parameter definition */
typedef struct MidiParam
{
    MidiParamType_t eParam;
    MidiParamData_t uData;
} MidiParam_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief Init resources for MIDI tasks
 * @retval None.
 */
void vMidiTaskInit(void);

/**
 * @brief Notify event to a task.
 * @param u32Event event to notify.
 * @retval operation result, true for correct read, false for error
 */
bool bMidiTaskNotify(uint32_t u32Event);

/**
  * @brief Send a command to midi task.
  * @param xMidiTaskCmd Command to send.
  * @return true cmd queue.
  * @return false cmd not queue.
  */
bool bMidiSendCmd(MidiTaskCmd_t xMidiTaskCmd);

/**
 * @brief Get internal midi task parameter.
 * @param ePatamId: Id of parameter to get.
 * @return MidiParam_t: structure with requested parameter data.
 */
MidiParam_t xMidiGetParam(MidiParamType_t ePatamId);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_TASK_H */

/*****END OF FILE****/
