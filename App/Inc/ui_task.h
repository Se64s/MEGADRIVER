/**
  ******************************************************************************
  * @file           : ui_task.h
  * @brief          : Task to handle ui actions
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UI_TASK_H
#define __UI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define UI_TASK_NAME   "UI"
#define UI_TASK_STACK  128U
#define UI_TASK_PRIO   1

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for UI tasks
  * @retval operation result, true for correct creation, false for error
  */
bool UI_task_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __UI_TASK_H */

/*****END OF FILE****/
