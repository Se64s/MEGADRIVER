/**
 * @file sys_rtos.h
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief Support code for RTOS.
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_RTOS_H
#define __SYS_RTOS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"

/* Exported defines ---------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/

/* RTOS check signal */
#define RTOS_CHECK_SIGNAL(VAR, SIG)     (((VAR) & (SIG)) == (SIG))

/* Exported functions prototypes --------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __SYS_RTOS_H */

/*EOF*/