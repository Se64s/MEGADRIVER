/**
 * @file 	cli_cmd.h
 * @author  Sebastián Del Moral Gallardo.
 * @brief	CLI commands implemented in system.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CLI_CMD_TASK_H
#define __CLI_CMD_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Register functions used in CLI.
 * @retval None.
 */
void cli_cmd_init(void);

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __CLI_CMD_TASK_H */

/* EOF */
