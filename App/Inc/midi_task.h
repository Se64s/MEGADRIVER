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

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define MIDI_TASK_NAME   "MIDI"
#define MIDI_TASK_STACK  128U
#define MIDI_TASK_PRIO   2

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Size of CMD buf */
#define MIDI_CMD_BUF_SIZE (3U)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for MIDI tasks
  * @retval operation result, true for correct creation, false for error
  */
bool MIDI_task_init(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool MIDI_task_notify(uint32_t u32Event);

/**
  * @brief Retrieve pointer to cmd buff.
  * @retval Pointer to midi cmd buf.
  */
uint8_t * MIDI_get_cmd_buf(void);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_TASK_H */

/*****END OF FILE****/
