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

#include <stdbool.h>
#include "stm32g0xx_hal.h"

/* Private defines -----------------------------------------------------------*/

/* Option defines */
#define YM2612_USE_RTOS
#define YM2612_DEBUG

/* Device parameters */
#define YM2612_MAX_NUM_VOICE    (6U)

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

#define YM2612_A1_GPIO_PIN      GPIO_PIN_15
#define YM2612_A1_GPIO_PORT     GPIOC
#define YM2612_A1_GPIO_CLK      __HAL_RCC_GPIOC_CLK_ENABLE

#define YM2612_A0_GPIO_PIN      GPIO_PIN_1
#define YM2612_A0_GPIO_PORT     GPIOF
#define YM2612_A0_GPIO_CLK      __HAL_RCC_GPIOF_CLK_ENABLE

#define YM2612_RD_GPIO_PIN      GPIO_PIN_14
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
    YM2612_ADDR_TEST0 = 0x21U,      /* LSI test data */
    YM2612_ADDR_LFO = 0x22U,        /* LFO freq control */
    YM2612_ADDR_TIMA_0 = 0x24U,     /* TIMA ?? */
    YM2612_ADDR_TIMA_1 = 0x25U,     /* TIMA ?? */
    YM2612_ADDR_TIMB_0 = 0x26U,     /* TIMB ?? */
    YM2612_ADDR_TIMx_CTRL = 0x27U,  /* TIMAB Ctrl */
    YM2612_ADDR_KEY_ON_OFF = 0x28U, /* Key on/off */
    YM2612_ADDR_DAC_DATA = 0x2AU,   /* DAC data */
    YM2612_ADDR_DAC_SEL = 0x2BU,    /* DAC sel */
    YM2612_ADDR_TEST1 = 0x2CU,      /* LSI test data */
    YM2612_ADDR_CH0_FNUM_1 = 0xA0,  /* LSI test data */
    YM2612_ADDR_FNUM_1 = 0xA0,      /* FNUM1 */
    YM2612_ADDR_FNUM_2 = 0xA4,      /* FNUM2 */
    YM2612_ADDR_NODEF = 0xFFU,
} YM2612_addr_t;

/* Bank selection */
typedef enum
{
    YM2612_BANK_0 = 0x00U,
    YM2612_BANK_1 = 0x01U
} YM2612_bank_t;

/* Synth channels */
typedef enum
{
    YM2612_CH_1 = 0x00U,
    YM2612_CH_2 = 0x01U,
    YM2612_CH_3 = 0x02U,
    YM2612_CH_4 = 0x03U,
    YM2612_CH_5 = 0x04U,
    YM2612_CH_6 = 0x05U
} YM2612_ch_id_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Initialization of hardware resources interface
  * @retval Operation status
*/
YM2612_status_t xYM2612_init(void);

/**
  * @brief  Deinitialization of hardware resources interface
  * @retval Operation status
*/
YM2612_status_t xYM2612_deinit(void);

/**
  * @brief write register value into YM2612
  * @param u8RegAddr register address
  * @param u8RegData register data
  * @param xBank selected bank
  * @retval None
  */
void vYM2612_write_reg(uint8_t u8RegAddr, uint8_t u8RegData, YM2612_bank_t xBank);

/**
  * @brief Set midi note into channel
  * @param xChannel synth channel
  * @param u8MidiNote Midi note to set on channel
  * @retval True if freq has been applied, false ioc.
  */
bool bYM2612_set_note(YM2612_ch_id_t xChannel, uint8_t u8MidiNote);

/**
  * @brief Set key on on specified channel
  * @param xChannel synth channel
  * @retval None
  */
void vYM2612_key_on(YM2612_ch_id_t xChannel);

/**
  * @brief Set key off on specified channel
  * @param xChannel synth channel
  * @retval None
  */
void vYM2612_key_off(YM2612_ch_id_t xChannel);

#ifdef __cplusplus
}
#endif

#endif /* __YM2612_DRIVER_H */

/*****END OF FILE****/
