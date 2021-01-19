/**
  ******************************************************************************
  * @file           : mapping_task.h
  * @brief          : Task to handle mapping actions
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAPPING_TASK_H
#define __MAPPING_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

/* Activate debug output */
//#define MAP_DEBUG

/* Task parameters */
#define MAP_TASK_NAME                   "MAP"
#define MAP_TASK_STACK                  (128U)
#define MAP_TASK_PRIO                   (2U)
#define MAP_TASK_INIT_DELAY             (500U)

/* MAPPING task signals */
#define MAP_SIGNAL_ERROR                (1UL << 0U)
#define MAP_SIGNAL_ADC_UPDATE           (1UL << 1U)
#define MAP_SIGNAL_MAPPING_UPDATE       (1UL << 2U)
#define MAP_SIGNAL_NOT_DEF              (1UL << 31U)

/* Exported types ------------------------------------------------------------*/

/** Defined mapping channels */
typedef enum 
{
    MAP_CH_1 = 0x00,
    MAP_CH_2,
    MAP_CH_3,
    MAP_CH_4,
    MAP_CH_NUM
} MapChannel_t;

/** Defined mapping modes */
typedef enum 
{
    MAP_MODE_NONE = 0x00,
    MAP_MODE_V_OCT,
    MAP_MODE_GATE,
    MAP_MODE_PARAMETER,
    MAP_MODE_NUM
} MapMode_t;

/** Mapping control element */
typedef struct
{
    MapMode_t xMode;
    uint8_t u8Voice;
    uint8_t u8Operator;
    uint8_t u8ParameterId;
    uint16_t u16Value;
} MapElement_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief Get mapping cfg mapping
  * @param u8MapId id of cfg to get.
  * @retval Copy of Map cfg value.
  */
MapElement_t xMapGetCfg(uint8_t u8MapId);

/**
  * @brief Set mapping cfg mapping
  * @param u8MapId id of cfg to set.
  * @param xMapValue cfg value.
  * @retval None.
  */
void vMapSetCfg(uint8_t u8MapId, MapElement_t xMapValue);

/**
  * @brief Init resources for MAPPING tasks
  * @retval operation result, true for correct creation, false for error
  */
bool bMapTaskInit(void);

/**
  * @brief Notify event to a task.
  * @param u32Event event to notify.
  * @retval operation result, true for correct read, false for error
  */
bool bMapTaskNotify(uint32_t u32Event);

#ifdef __cplusplus
}
#endif

#endif /* __MAPPING_TASK_H */

/*****END OF FILE****/
