/**
 * @file sys_mcu.h
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief BSP to init base clock and flash config.
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_MCU_H
#define __SYS_MCU_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ---------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions prototypes --------------------------------------------*/

/**
 * @brief Reset of all peripherals, Initializes the Flash interface and the Systick
 * @retval None
 */
void SYS_Init(void);

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SYS_SystemClockConfig(void);

/**
 * @brief Force system reset
 * @retval None
 */
void SYS_Reset(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_MCU_H */

/*EOF*/