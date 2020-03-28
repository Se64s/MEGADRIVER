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
// #define YM2612_DEBUG
// #define YM2612_TEST_GPIO

/* Number of channels by device */
#define YM2612_NUM_CHANNEL      (6U)

/* Number of operator by channel */
#define YM2612_NUM_OP_CHANNEL   (4U)

/* Device parameters */
#define YM2612_MAX_NUM_VOICE    (6U)

/* Max and min values of FM parameters */
#define MAX_VALUE_LFO_FREQ      (8U)
#define MAX_VALUE_FEEDBACK      (8U)
#define MAX_VALUE_ALGORITHM     (8U)
#define MAX_VALUE_VOICE_OUT     (4U)
#define MAX_VALUE_AMP_MOD_SENS  (4U)
#define MAX_VALUE_PHA_MOD_SENS  (8U)

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

/* Operator structure */
typedef struct 
{
  uint8_t u8Detune;
  uint8_t u8Multiple;
  uint8_t u8TotalLevel;
  uint8_t u8KeyScale;
  uint8_t u8AttackRate;
  uint8_t u8AmpMod;
  uint8_t u8DecayRate;
  uint8_t u8SustainRate;
  uint8_t u8SustainLevel;
  uint8_t u8ReleaseRate;
  uint8_t u8SsgEg;
} xFmOperator_t;

/* Channel voice structure */
typedef struct 
{
  uint8_t u8Feedback;
  uint8_t u8Algorithm;
  uint8_t u8AudioOut;
  uint8_t u8AmpModSens;
  uint8_t u8PhaseModSens;
  xFmOperator_t xOperator[YM2612_NUM_OP_CHANNEL];
} xFmChannel_t;

/* Chip voice structure */
typedef struct 
{
  uint8_t u8LfoOn;
  uint8_t u8LfoFreq;
  xFmChannel_t xChannel[YM2612_NUM_CHANNEL];
} xFmDevice_t;

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
    YM2612_ADDR_LFO = 0x22U,
    YM2612_ADDR_KEY_ON_OFF = 0x28U,
    YM2612_ADDR_DAC_DATA = 0x2AU,
    YM2612_ADDR_DAC_SEL = 0x2BU,
    YM2612_ADDR_DET_MULT = 0x30U,
    YM2612_ADDR_TOT_LVL = 0x40U,
    YM2612_ADDR_KS_AR = 0x50U,
    YM2612_ADDR_AM_DR = 0x60U,
    YM2612_ADDR_SR = 0x70U,
    YM2612_ADDR_SL_RR = 0x80U,
    YM2612_ADDR_SSG_EG = 0x90U,
    YM2612_ADDR_FNUM_1 = 0xA0U,
    YM2612_ADDR_FNUM_2 = 0xA4U,
    YM2612_ADDR_FB_ALG = 0xB0U,
    YM2612_ADDR_LR_AMS_PMS = 0xB4U,
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
  * @brief Set reg preset.
  * @param pxRegPreset pointer with reg preset to set
  * @retval None
  */
void vYM2612_set_reg_preset(xFmDevice_t * pxRegPreset);

/**
  * @brief Get reg preset.
  * @retval address of actual reg preset.
  */
xFmDevice_t * pxYM2612_set_reg_preset(void);

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
