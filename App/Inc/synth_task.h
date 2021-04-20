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

#include "YM2612_driver.h"

#include "app_lfs.h"
#include "synth_app_data_const.h"

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define SYNTH_TASK_NAME                     "SYNTH"
#define SYNTH_TASK_STACK                    ( 1024U )
#define SYNTH_TASK_PRIO                     ( 4U )
#define SYNTH_TASK_INIT_DELAY               ( 500U )

/* Synth internal queue send timeout */
#define SYNTH_QUEUE_TIMEOUT                 ( 100U )

/* Maximun number of voices */
#define SYNTH_MAX_NUM_VOICE                 ( YM2612_NUM_CHANNEL )

/* Maximun number of user presets */
#define SYNTH_MAX_NUM_USER_PRESET           ( LFS_YM_SLOT_NUM )

/* Enable extended DBG */
#define SYNTH_DBG_VERBOSE

/* Exported types ------------------------------------------------------------*/

/** Synth commands */
typedef enum
{
    SYNTH_CMD_VOICE_UPDATE_MONO = 0x00U,
    SYNTH_CMD_VOICE_UPDATE_POLY,
    SYNTH_CMD_PARAM_UPDATE,
    SYNTH_CMD_PRESET_UPDATE,
    SYNTH_CMD_VOICE_MUTE,
    SYNTH_CMD_NO_DEF = 0xFFU
} SynthCmdType_t;

/** Payload for voice update */
typedef struct
{
    uint8_t u8VoiceDst;
    uint8_t u8VoiceState;
    uint8_t u8Note;
    uint8_t u8Velocity;
} SynthCmdPayloadVoiceUpdateMono_t;

/** Payload for voice update */
typedef struct
{
    uint8_t u8VoiceState;
    uint8_t u8Note;
    uint8_t u8Velocity;
} SynthCmdPayloadVoiceUpdatePoly_t;

/** Payload for parameter update command */
typedef struct
{
    uint8_t u8Id;
    uint8_t u8Data;
} SynthCmdPayloadParamUpdate_t;

/** Payload definition for preset update command */
typedef struct
{
    uint8_t u8Action;
    uint8_t u8Bank;
    uint8_t u8Program;
} SynthCmdPayloadPresetUpdate_t;

/** Union definitions with all event payload */
typedef union
{
    SynthCmdPayloadVoiceUpdateMono_t    xVoiceUpdateMono;
    SynthCmdPayloadVoiceUpdatePoly_t    xVoiceUpdatePoly;
    SynthCmdPayloadParamUpdate_t        xParamUpdate;
    SynthCmdPayloadPresetUpdate_t       xPresetUpdate;
} SynthCmdPayload_t;

/** Synth command definition */
typedef struct SynthCmd
{
    SynthCmdType_t eCmd;
    SynthCmdPayload_t uPayload;
} SynthCmd_t;

/** Synth voice cfg modes */
typedef enum
{
    SYNTH_VOICE_CFG_MONO = 0x00U,
    SYNTH_VOICE_CFG_POLY,
    SYNTH_VOICE_CFG_NUM
} SynthVoiceCfgMode_t;

/** Synth preset actions */
typedef enum
{
    SYNTH_PRESET_ACTION_SAVE = 0x00U,
    SYNTH_PRESET_ACTION_LOAD,
    SYNTH_PRESET_ACTION_NO_DEF = 0xFFU,
} SynthPresetAction_t;

/** Synth voice modes */
typedef enum
{
    SYNTH_VOICE_STATE_ON = 0x00U,
    SYNTH_VOICE_STATE_OFF,
    SYNTH_VOICE_STATE_NO_DEF = 0xFFU,
} SynthVoiceState_t;

/** Synth voice modes */
typedef enum
{
    SYNTH_PARAM_VOICE_0_NOTE = 0x00U,
    SYNTH_PARAM_VOICE_1_NOTE,
    SYNTH_PARAM_VOICE_2_NOTE,
    SYNTH_PARAM_VOICE_3_NOTE,
    SYNTH_PARAM_VOICE_4_NOTE,
    SYNTH_PARAM_VOICE_5_NOTE,
    SYNTH_PARAM_NOT_DEF = 0xFFU,
} SynthParamId_t;

/** Synth parameter definition */
typedef struct
{
    uint8_t u8ParamId;
    uint32_t u32ParamValue;
} SynthParam_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for SYNTH tasks
  * @retval None.
  */
void vSynthTaskInit(void);

/**
  * @brief Send a command to synthj task.
  * @param xSynthCmd Command to send.
  * @return true cmd queue.
  * @return false cmd not queue.
  */
bool bSynthSendCmd(SynthCmd_t xSynthCmd);

/**
 * @brief Get internal synth task paramter.
 * @param u8ParamId: Id of parameter to get.
 * @return SynthParam_t: structure with requested parameter data.
 */
SynthParam_t xSynthGetParam(uint8_t u8ParamId);

#ifdef __cplusplus
}
#endif

#endif /* __SYNTH_TASK_H */

/* EOF */
