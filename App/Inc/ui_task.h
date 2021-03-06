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
#define UI_TASK_NAME                    "UI"
#define UI_TASK_STACK                   (256U)
#define UI_TASK_PRIO                    (1U)

/* Check signals */
#define CHECK_SIGNAL(VAR, SIG)          (((VAR) & (SIG)) == (SIG))

/* UI signals */
#define UI_SIGNAL_ENC_UPDATE_CCW        (1UL << 0U)
#define UI_SIGNAL_ENC_UPDATE_CW         (1UL << 1U)
#define UI_SIGNAL_ENC_UPDATE_SW_SET     (1UL << 2U)
#define UI_SIGNAL_ENC_UPDATE_SW_RESET   (1UL << 3U)
#define UI_SIGNAL_SYNTH_ON              (1UL << 4U)
#define UI_SIGNAL_SYNTH_OFF             (1UL << 5U)
#define UI_SIGNAL_MIDI_DATA             (1UL << 6U)
#define UI_SIGNAL_SCREEN_UPDATE         (1UL << 7U)
#define UI_SIGNAL_IDLE                  (1UL << 8U)
#define UI_SIGNAL_RESTORE_IDLE          (1UL << 9U)
#define UI_SIGNAL_ERROR                 (1UL << 10U)
#define UI_SIGNAL_NOT_DEF               (1UL << 31U)

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Init resources for UI tasks
  * @retval operation result, true for correct creation, false for error
  */
bool bUiTaskInit(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool bUiTaskNotify(uint32_t u32Event);

#ifdef __cplusplus
}
#endif

#endif /* __UI_TASK_H */

/*****END OF FILE****/
