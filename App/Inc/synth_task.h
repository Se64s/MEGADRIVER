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

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define SYNTH_TASK_NAME   "SYNTH"
#define SYNTH_TASK_STACK  128U
#define SYNTH_TASK_PRIO   2

/* Exported types ------------------------------------------------------------*/
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
