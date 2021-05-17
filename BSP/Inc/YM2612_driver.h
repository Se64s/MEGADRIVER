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
#include <stdint.h>

/* Private defines -----------------------------------------------------------*/

/* Option defines */
// #define YM2612_DEBUG

/** Number of channels by device */
#define YM2612_NUM_CHANNEL      (YM2612_NUM_CH)

/** Number of operator by channel */
#define YM2612_NUM_OP_CHANNEL   (YM2612_NUM_OP)

/** Number of total parameters */
#define YM2612_NUM_PARAMETERS   (FM_VAR_SIZE_NUMBER)

/* Bit position in shift reg */
#define YM_POS_Dx               (0U)
#define YM_POS_REG              (8U)
#define YM_POS_CS               (9U)
#define YM_POS_WR               (10U)
#define YM_POS_RD               (11U)
#define YM_POS_A0               (12U)
#define YM_POS_A1               (13U)

/* Max and min values of FM parameters */
#define MAX_VALUE_LFO_ON        (2U)
#define MAX_VALUE_LFO_FREQ      (8U)
#define MAX_VALUE_FEEDBACK      (8U)
#define MAX_VALUE_ALGORITHM     (8U)
#define MAX_VALUE_VOICE_OUT     (4U)
#define MAX_VALUE_AMP_MOD_SENS  (4U)
#define MAX_VALUE_PHA_MOD_SENS  (8U)
#define MAX_VALUE_DETUNE        (8U)
#define MAX_VALUE_MULTIPLE      (16U)
#define MAX_VALUE_TOTAL_LEVEL   (128U)
#define MAX_VALUE_KEY_SCALE     (4U)
#define MAX_VALUE_ATTACK_RATE   (32U)
#define MAX_VALUE_AMP_MOD_EN    (2U)
#define MAX_VALUE_DECAY_RATE    (32U)
#define MAX_VALUE_SUSTAIN_RATE  (32U)
#define MAX_VALUE_SUSTAIN_LEVEL (16U)
#define MAX_VALUE_RELEASE_RATE  (16U)
#define MAX_VALUE_SSG_ENVELOPE  (8U)

/* Exported types ------------------------------------------------------------*/

/** List of available parameters */
typedef enum
{
    FM_VAR_LFO_ON = 0U,
    FM_VAR_LFO_FREQ,
    FM_VAR_VOICE_FEEDBACK,
    FM_VAR_VOICE_ALGORITHM,
    FM_VAR_VOICE_AUDIO_OUT,
    FM_VAR_VOICE_AMP_MOD_SENS,
    FM_VAR_VOICE_PHA_MOD_SENS,
    FM_VAR_OPERATOR_DETUNE,
    FM_VAR_OPERATOR_MULTIPLE,
    FM_VAR_OPERATOR_TOTAL_LEVEL,
    FM_VAR_OPERATOR_KEY_SCALE,
    FM_VAR_OPERATOR_ATTACK_RATE,
    FM_VAR_OPERATOR_AMP_MOD,
    FM_VAR_OPERATOR_DECAY_RATE,
    FM_VAR_OPERATOR_SUSTAIN_RATE,
    FM_VAR_OPERATOR_SUSTAIN_LEVEL,
    FM_VAR_OPERATOR_RELEASE_RATE,
    FM_VAR_OPERATOR_SSG_ENVELOPE,
    FM_VAR_SIZE_NUMBER
} eFmParameter_t;

/** Operation status */
typedef enum
{
    YM2612_STATUS_ERROR = 0U,
    YM2612_STATUS_OK = 1U,
    YM2612_STATUS_NODEF = 0xFFU,
} YM2612_status_t;

/** YM2612 register addresses */
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

/** Bank selection options */
typedef enum
{
    YM2612_BANK_0 = 0x00U,
    YM2612_BANK_1 = 0x01U
} YM2612_bank_t;

/** Synth voice channels */
typedef enum
{
    YM2612_CH_1 = 0x00U,
    YM2612_CH_2 = 0x01U,
    YM2612_CH_3 = 0x02U,
    YM2612_CH_4 = 0x03U,
    YM2612_CH_5 = 0x04U,
    YM2612_CH_6 = 0x05U,
    YM2612_NUM_CH
} YM2612_ch_id_t;

/** Synth voice operators */
typedef enum
{
    YM2612_OP_1 = 0x00U,
    YM2612_OP_2 = 0x01U,
    YM2612_OP_3 = 0x02U,
    YM2612_OP_4 = 0x03U,
    YM2612_NUM_OP
} YM2612_op_id_t;

/** Operator parameter structure */
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

/** Channel voice parameter structure */
typedef struct 
{
    uint8_t u8Feedback;
    uint8_t u8Algorithm;
    uint8_t u8AudioOut;
    uint8_t u8AmpModSens;
    uint8_t u8PhaseModSens;
    xFmOperator_t xOperator[YM2612_NUM_OP_CHANNEL];
} xFmChannel_t;

/** Overall paramater structure */
typedef struct xFmDevice
{
    uint8_t u8LfoOn;
    uint8_t u8LfoFreq;
    xFmChannel_t xChannel[YM2612_NUM_CHANNEL];
} xFmDevice_t;

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
xFmDevice_t * pxYM2612_get_reg_preset(void);

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
