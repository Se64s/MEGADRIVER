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

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#include "YM2612_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define SYNTH_TASK_NAME             "SYNTH"
#define SYNTH_TASK_STACK            (128U)
#define SYNTH_TASK_PRIO             (2U)

/* SysEx CMD parameters */
#define SYNTH_LEN_VENDOR_ID                 (3U)
#define SYNTH_LEN_PRESET_NAME               (16U)
#define SYNTH_LEN_PRESET_CODED_NAME         (30U)
#define SYNTH_LEN_MIN_SYSEX_CMD             (4U)
#define SYNTH_LEN_SET_REG_CMD               (300U)
#define SYNTH_LEN_SAVE_PRESET_CMD           (331U)
#define SYNTH_LEN_LOAD_PRESET_CMD           (5U)
#define SYNTH_LEN_LOAD_DEFAULT_PRESET_CMD   (5U)

/* Synth message parameters */
#define SYNTH_LEN_MSG               (4U)

/* Maximun number of voices */
#define SYNTH_MAX_NUM_VOICE     (YM2612_MAX_NUM_VOICE)

/* Exported types ------------------------------------------------------------*/

/* Synth commands */
typedef enum
{
    SYNTH_CMD_NOTE_ON = 0x00U,
    SYNTH_CMD_NOTE_OFF,
    SYNTH_CMD_NOTE_OFF_ALL,
    SYNTH_CMD_SYSEX,
    SYNTH_CMD_NO_DEF = 0xFFU
} SynthMsgType_t;

/* SysEx defined cmd */
typedef enum
{
  SYNTH_SYSEX_CMD_SET_PRESET = 0x00U,
  SYNTH_SYSEX_CMD_SAVE_PRESET = 0x01U,
  SYNTH_SYSEX_CMD_LOAD_PRESET = 0x02U,
  SYNTH_SYSEX_CMD_LOAD_DEFAULT_PRESET = 0x03U,
  SYNTH_SYSEX_CMD_NO_DEF = 0x1FU
} SynthSysExCmdDef_t;

/* SysEx command format */
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

/* Synth cmd message structure */
typedef struct
{
  SynthMsgType_t xType;
  uint8_t u8Data[SYNTH_LEN_MSG];
} SynthMsg_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

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

#ifdef __cplusplus
}
#endif

#endif /* __SYNTH_TASK_H */

/*****END OF FILE****/
