/**
  ******************************************************************************
  * @file           : cli_task.h
  * @brief          : Task to handle cli actions and serial debug prints
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLI_TASK_H
#define __CLI_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Private defines -----------------------------------------------------------*/

/* Task parameters */
#define CLI_TASK_NAME   "CLI"
#define CLI_TASK_STACK  256U
#define CLI_TASK_PRIO   0U

/* Buffer sizes */
#define CLI_OUTPUT_BUFFER_SIZE  configCOMMAND_INT_MAX_OUTPUT_SIZE

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for CLI tasks
  * @retval operation result, true for correct creation, false for error
  */
bool CLI_task_init(void);

/**
  * @brief Printf implementation using the cli task
  * @param module_name name of calling task
  * @param Format output format to use
  * @retval None.
  */
void cli_printf(const char *module_name, const char *Format, ...);

/**
  * @brief Printf implementation using the cli task
  * @param Format output format to use
  * @retval None.
  */
void cli_raw_printf(const char *Format, ...);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool cli_task_notify(uint32_t u32Event);

#ifdef __cplusplus
}
#endif

#endif /* __CLI_TASK_H */

/*****END OF FILE****/
