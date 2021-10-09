/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* App version numver provide by makefile */
#ifndef MAIN_APP_VERSION
#define MAIN_APP_VERSION "X.Y.Z"
#endif

#ifndef GIT_REVISION
#define GIT_REVISION "AABBCCDD"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/*****END OF FILE****/
