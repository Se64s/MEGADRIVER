/**
  ******************************************************************************
  * @file           : YM2612_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __YM2612_DRIVER_H
#define __YM2612_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/

/* GPIO definitions */
#define YM2612_Dx_GPIO_PIN      (YM2612_D0_GPIO_PIN | YM2612_D1_GPIO_PIN | YM2612_D2_GPIO_PIN | YM2612_D3_GPIO_PIN | YM2612_D4_GPIO_PIN | YM2612_D5_GPIO_PIN | YM2612_D6_GPIO_PIN | YM2612_D7_GPIO_PIN)
#define YM2612_Dx_GPIO_PORT     GPIOB
#define YM2612_Dx_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D0_GPIO_PIN      GPIO_PIN_0
#define YM2612_D0_GPIO_PORT     GPIOB
#define YM2612_D0_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D1_GPIO_PIN      GPIO_PIN_1
#define YM2612_D1_GPIO_PORT     GPIOB
#define YM2612_D1_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D2_GPIO_PIN      GPIO_PIN_2
#define YM2612_D2_GPIO_PORT     GPIOB
#define YM2612_D2_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D3_GPIO_PIN      GPIO_PIN_3
#define YM2612_D3_GPIO_PORT     GPIOB
#define YM2612_D3_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D4_GPIO_PIN      GPIO_PIN_4
#define YM2612_D4_GPIO_PORT     GPIOB
#define YM2612_D4_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D5_GPIO_PIN      GPIO_PIN_5
#define YM2612_D5_GPIO_PORT     GPIOB
#define YM2612_D5_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D6_GPIO_PIN      GPIO_PIN_6
#define YM2612_D6_GPIO_PORT     GPIOB
#define YM2612_D6_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_D7_GPIO_PIN      GPIO_PIN_7
#define YM2612_D7_GPIO_PORT     GPIOB
#define YM2612_D7_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_REG_GPIO_PIN     GPIO_PIN_8
#define YM2612_REG_GPIO_PORT    GPIOB
#define YM2612_REG_GPIO_CLK     __HAL_RCC_GPIOB_CLK_ENABLE

// #define YM2612_A1_GPIO_PIN      GPIO_PIN_15
#define YM2612_A1_GPIO_PIN      GPIO_PIN_10 // only in nucleo board
#define YM2612_A1_GPIO_PORT     GPIOC
#define YM2612_A1_GPIO_CLK      __HAL_RCC_GPIOC_CLK_ENABLE

#define YM2612_A0_GPIO_PIN      GPIO_PIN_1
#define YM2612_A0_GPIO_PORT     GPIOF
#define YM2612_A0_GPIO_CLK      __HAL_RCC_GPIOF_CLK_ENABLE

// #define YM2612_RD_GPIO_PIN      GPIO_PIN_14
#define YM2612_RD_GPIO_PIN      GPIO_PIN_11 // only in nucleo board
#define YM2612_RD_GPIO_PORT     GPIOC
#define YM2612_RD_GPIO_CLK      __HAL_RCC_GPIOC_CLK_ENABLE

#define YM2612_WR_GPIO_PIN      GPIO_PIN_13
#define YM2612_WR_GPIO_PORT     GPIOC
#define YM2612_WR_GPIO_CLK      __HAL_RCC_GPIOC_CLK_ENABLE

#define YM2612_CS_GPIO_PIN      GPIO_PIN_9
#define YM2612_CS_GPIO_PORT     GPIOB
#define YM2612_CS_GPIO_CLK      __HAL_RCC_GPIOB_CLK_ENABLE

#define YM2612_CLK_GPIO_PIN     GPIO_PIN_0
#define YM2612_CLK_GPIO_PORT    GPIOF
#define YM2612_CLK_GPIO_CLK     __HAL_RCC_GPIOF_CLK_ENABLE
#define YM2612_CLK_GPIO_AF      GPIO_AF2_TIM14

/* Exported types ------------------------------------------------------------*/

/* Operation status */
typedef enum
{
    YM2612_STATUS_ERROR = 0U,
    YM2612_STATUS_OK = 1U,
    YM2612_STATUS_NODEF = 0xFFU,
} YM2612_status_t;

/* YM2612 register addresses */
typedef enum
{
    YM2612_ADDR_KEY_ON_OFF = 0x28U,
    YM2612_ADDR_NODEF = 0xFFU,
} YM2612_addr_t;

/* Bank selection */
typedef enum
{
    YM2612_BANK_0 = 0x00U,
    YM2612_BANK_1 = 0x01U
} YM2612_bank_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialization of hardware resources interface
  * @retval Operation status
*/
YM2612_status_t YM2612_init(void);

/**
  * @brief  Deinitialization of hardware resources interface
  * @retval Operation status
*/
YM2612_status_t YM2612_deinit(void);

/**
  * @brief write register value into YM2612
  * @param reg_addr register address
  * @param reg_data register data
  * @param bank selected bank
  * @retval None
  */
void YM2612_write_reg(uint8_t reg_addr, uint8_t reg_data, YM2612_bank_t bank);

#ifdef __cplusplus
}
#endif

#endif /* __YM2612_DRIVER_H */

/*****END OF FILE****/
