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
#define SYNTH_TASK_NAME   "SYNTH"
#define SYNTH_TASK_STACK  (128U)
#define SYNTH_TASK_PRIO   (2U)

/* SysEx CMD parameters */
#define SYNTH_LEN_VENDOR_ID       (3U)
#define SYNTH_LEN_MIN_SYSEX_CMD   (4U)
#define SYNTH_LEN_SET_REG_CMD     (300U)

/* Exported types ------------------------------------------------------------*/

/* SysEx defined cmd */
typedef enum
{
  SYNTH_SYSEX_CMD_SET_PRESET = 0x00U,
  SYNTH_SYSEX_CMD_NO_DEF = 0x1FU
} SynthSysExCmdDef_t;

/* SysEx command format */
typedef struct 
{
  uint8_t pu8VendorId[SYNTH_LEN_VENDOR_ID];
  SynthSysExCmdDef_t xSysExCmd;
  uint8_t * pu8CmdData;
} SynthSysExCmd_t;

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
