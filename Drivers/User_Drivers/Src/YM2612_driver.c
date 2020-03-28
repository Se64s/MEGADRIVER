/**
  ******************************************************************************
  * @file           : YM2612_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "YM2612_driver.h"
#include "stm32g0xx_hal.h"

#ifdef YM2612_USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#ifdef YM2612_DEBUG
#include "cli_task.h"
#endif
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Initialize TIMx peripheral as follows:
   + Prescaler = (SystemCoreClock / 64000000) - 1
   + Period = (4 - 1)
   + ClockDivision = 0
   + Counter direction = Up
*/
#define PRESCALER_VALUE (uint32_t)((SystemCoreClock / 64000000) - 1)
#define PERIOD_VALUE    (uint32_t)(4 - 1)            /* Period Value  */
#define PULSE1_VALUE    (uint32_t)(PERIOD_VALUE / 2) /* Capture Compare 1 Value  */

/* Number of ticks per microsec */
#define TICKS_USEC          (10U)

/* Number of notes per Octave */
#define NUM_NOTES_OCTAVE    (12U)

/* Base block define */
#define BASE_BLOCK          (4U)

/* Max block range */
#define MAX_BLOCK           (8U)

/* Private macro -------------------------------------------------------------*/

/* Bit handling macro */
#define YM_SET_BIT(PORT, BITS)    ((PORT)->BSRR = (uint32_t)(BITS))
#define YM_RESET_BIT(PORT, BITS)  ((PORT)->BRR = (uint32_t)(BITS))

/* Private variables ---------------------------------------------------------*/

/* F_num base values - 12 notes */
const static uint16_t u16OctaveBaseValues[] = {
    617, 654, 693, 734,
    778, 824, 873, 925,
    980, 1038, 1100, 1165
};

/* Timer hanlder */
TIM_HandleTypeDef htim14;

/* Chip control structure */
static xFmDevice_t xYmDevice = {0};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Get chanbel register value from device structure.
  * @param  pxDevice pointer to channel control structure.
  * @param  u8RegAddr Address of register to get.
  * @param  pu8RegData pointer where store register value.
  * @retval True if registere has been extracted, False ioc.
*/
static bool _get_device_register_value(xFmDevice_t * pxDevice, uint8_t u8RegAddr, uint8_t * pu8RegData);

/**
  * @brief  Get chanbel register value from device structure.
  * @param  pxChannel pointer to channel control structure.
  * @param  u8RegAddr Address of register to get.
  * @param  pu8RegData pointer where store register value.
  * @retval True if registere has been extracted, False ioc.
*/
static bool _get_channel_register_value(xFmChannel_t * pxChannel, uint8_t u8RegAddr, uint8_t * pu8RegData);

/**
  * @brief  Get channel operator register value from device structure.
  * @param  pxOperator pointer to operator structure.
  * @param  u8RegAddr Address of register to get.
  * @param  pu8RegData pointer where store register value.
  * @retval True if registere has been extracted, False ioc.
*/
static bool _get_operator_register_value(xFmOperator_t * pxOperator, uint8_t u8RegAddr, uint8_t * pu8RegData);

/**
  * @brief  Software delay
  * @param  microsec number of usecs to wait
  * @retval Operation status
*/
static void _us_delay(uint32_t microsec);

/**
  * @brief  Test output on each gpio out
  * @retval None
*/
static void _low_level_test(void);

/**
  * @brief  Init low level resources
  * @retval None
*/
static void _low_level_init(void);

/**
  * @brief  Deinit low level resources
  * @retval None
*/
static void _low_level_deinit(void);

/**
  * @brief TIM Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM_Init(void);

/**
  * @brief TIM Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_TIM_Deinit(void);

/* Private user code ---------------------------------------------------------*/

static bool _get_device_register_value(xFmDevice_t * pxDevice, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    uint8_t bRetVal = false;

    if ((pxDevice != NULL) && (pu8RegData != NULL))
    {
        uint8_t u8RegData = 0U;

        if (u8RegAddr == YM2612_ADDR_LFO)
        {
            u8RegData = ((pxDevice->u8LfoOn & 0x01) << 3U) | (pxDevice->u8LfoFreq & 0x07);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
    }

    return bRetVal;
}

static bool _get_channel_register_value(xFmChannel_t * pxChannel, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    uint8_t bRetVal = false;
    
    if ((pxChannel != NULL) && (pu8RegData != NULL))
    {
        uint8_t u8RegData = 0U;

        if (u8RegAddr == YM2612_ADDR_FB_ALG)
        {
            u8RegData = ((pxChannel->u8Feedback & 0x07) << 3U) | (pxChannel->u8Algorithm & 0x07);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_LR_AMS_PMS)
        {
            u8RegData = ((pxChannel->u8AudioOut & 0x03) << 6U) | 
            ((pxChannel->u8AmpModSens & 0x03) << 4U) | 
            (pxChannel->u8PhaseModSens & 0x07);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
    }

    return bRetVal;
}

static bool _get_operator_register_value(xFmOperator_t * pxOperator, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    uint8_t bRetVal = false;
    
    if ((pxOperator != NULL) && (pu8RegData != NULL))
    {
        uint8_t u8RegData = 0U;

        if (u8RegAddr == YM2612_ADDR_DET_MULT)
        {
            u8RegData = ((pxOperator->u8Detune & 0x07) << 4U) | (pxOperator->u8Multiple & 0x0F);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_TOT_LVL)
        {
            u8RegData = pxOperator->u8TotalLevel & 0x1F;
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_KS_AR)
        {
            u8RegData = ((pxOperator->u8KeyScale & 0x03) << 6U) | (pxOperator->u8AttackRate & 0x1F);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_AM_DR)
        {
            u8RegData = ((pxOperator->u8AmpMod & 0x01) << 7U) | (pxOperator->u8DecayRate & 0x1F);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_SR)
        {
            u8RegData = pxOperator->u8SustainRate & 0x1F;
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_SL_RR)
        {
            u8RegData = ((pxOperator->u8SustainLevel & 0x0F) << 4U) | (pxOperator->u8ReleaseRate & 0x0F);
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
        else if (u8RegAddr == YM2612_ADDR_SSG_EG)
        {
            u8RegData = pxOperator->u8SsgEg & 0x0F;
            *pu8RegData = u8RegData;
            bRetVal = true;
        }
    }
    
    return bRetVal;
}

static void _us_delay(uint32_t microsec)
{
    uint32_t tick_count = TICKS_USEC * microsec;
    while (tick_count-- != 0)
    {
        __NOP();
    }
}

static void _low_level_test(void)
{
    YM_SET_BIT(YM2612_Dx_GPIO_PORT, YM2612_Dx_GPIO_PIN);
    YM_SET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    YM_SET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    YM_SET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    YM_SET_BIT(YM2612_RD_GPIO_PORT, YM2612_RD_GPIO_PIN);
    YM_SET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_SET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    _us_delay(10);

    YM_RESET_BIT(YM2612_Dx_GPIO_PORT, YM2612_Dx_GPIO_PIN);
    YM_RESET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    YM_RESET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    YM_RESET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    YM_RESET_BIT(YM2612_RD_GPIO_PORT, YM2612_RD_GPIO_PIN);
    YM_RESET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_RESET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    _us_delay(10);
}

static void _low_level_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /* GPIO init */
    YM2612_Dx_GPIO_CLK();
    GPIO_InitStruct.Pin =   YM2612_Dx_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(YM2612_Dx_GPIO_PORT, &GPIO_InitStruct);

    YM2612_REG_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_REG_GPIO_PIN;
    HAL_GPIO_Init(YM2612_REG_GPIO_PORT, &GPIO_InitStruct);

    YM2612_A1_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_A1_GPIO_PIN;
    HAL_GPIO_Init(YM2612_A1_GPIO_PORT, &GPIO_InitStruct);

    YM2612_A0_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_A0_GPIO_PIN;
    HAL_GPIO_Init(YM2612_A0_GPIO_PORT, &GPIO_InitStruct);

    YM2612_RD_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_RD_GPIO_PIN;
    HAL_GPIO_Init(YM2612_RD_GPIO_PORT, &GPIO_InitStruct);

    YM2612_WR_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_WR_GPIO_PIN;
    HAL_GPIO_Init(YM2612_WR_GPIO_PORT, &GPIO_InitStruct);
    
    YM2612_CS_GPIO_CLK();
    GPIO_InitStruct.Pin = YM2612_CS_GPIO_PIN;
    HAL_GPIO_Init(YM2612_CS_GPIO_PORT, &GPIO_InitStruct);

#ifdef YM2612_TEST_GPIO
    _low_level_test();
#endif

    /* CLK_LINE */
    MX_TIM_Init();
}

static void _low_level_deinit(void)
{
    /* TODO: disable used gpio */
    HAL_GPIO_DeInit(YM2612_Dx_GPIO_PORT, YM2612_Dx_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_RD_GPIO_PORT, YM2612_RD_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    HAL_GPIO_DeInit(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);

    /* Timer clock deinit */
    MX_TIM_Deinit();
}

static void MX_TIM_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim14.Instance = TIM14;
    htim14.Init.Prescaler = PRESCALER_VALUE;
    htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim14.Init.Period = PERIOD_VALUE;
    htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim14.Init.RepetitionCounter = 0;
    htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_OC_Init(&htim14) != HAL_OK)
    {
        while (1);
    }
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = PULSE1_VALUE;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_OC_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        while (1);
    }

    /* Setup gpio */
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOF_CLK_ENABLE();
    
    /** TIM14 GPIO Configuration    
    PF0     ------> TIM14_CH1
    */
    GPIO_InitStruct.Pin = YM2612_CLK_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = YM2612_CLK_GPIO_AF;
    HAL_GPIO_Init(YM2612_CLK_GPIO_PORT, &GPIO_InitStruct);
}

static void MX_TIM_Deinit(void)
{
    /* Peripheral clock disable */
    HAL_TIM_OC_DeInit(&htim14);
    __HAL_RCC_TIM14_CLK_DISABLE();
    /* Deinit gpios */
    HAL_GPIO_DeInit(YM2612_CLK_GPIO_PORT, YM2612_CLK_GPIO_PIN);
}

/* Callback ------------------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

YM2612_status_t xYM2612_init(void)
{
    YM2612_status_t retval = YM2612_STATUS_ERROR;
    
    _low_level_init();
    
    /* Init clock */
    if (HAL_TIM_OC_Start(&htim14, TIM_CHANNEL_1) == HAL_OK)
    {
        retval = YM2612_STATUS_OK;
    }

#ifdef YM2612_USE_RTOS
    taskENTER_CRITICAL();
#endif

    /* Reset register values */
    YM_SET_BIT(YM2612_RD_GPIO_PORT, YM2612_RD_GPIO_PIN);
    YM_SET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    _us_delay(10);
    YM_RESET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    _us_delay(10);
    YM_SET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    _us_delay(100);

#ifdef YM2612_USE_RTOS
    taskEXIT_CRITICAL();
#endif

    return (retval);
}

YM2612_status_t xYM2612_deinit(void)
{
    YM2612_status_t retval = YM2612_STATUS_OK;
    
    _low_level_deinit();
    
    return (retval);
}

void vYM2612_write_reg(uint8_t u8RegAddr, uint8_t u8RegData, YM2612_bank_t xBank)
{
#ifdef YM2612_DEBUG
    vCliPrintf("DBG", "WR REG: %02X-%02X-%02X", u8RegAddr, u8RegData, xBank);
#endif
#ifdef YM2612_USE_RTOS
    taskENTER_CRITICAL();
#endif

    /* Write address */
    if (xBank == YM2612_BANK_0)
    {
        YM_RESET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    }
    else
    {
        YM_SET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    }
    YM_RESET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    YM_RESET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_Dx_GPIO_PORT, (u8RegAddr & 0xFF));
    YM_RESET_BIT(YM2612_Dx_GPIO_PORT, ((~u8RegAddr) & 0xFF));
    YM_RESET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);

    /* Operation delay */
    _us_delay(1);

    /* Clear bus */
    YM_SET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_SET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);

    /* Write data */
    YM_RESET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_Dx_GPIO_PORT, (u8RegData & 0xFF));
    YM_RESET_BIT(YM2612_Dx_GPIO_PORT, ((~u8RegData) & 0xFF));
    YM_RESET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);

    /* Operation delay */
    _us_delay(1);

    /* Clear resources */
    YM_SET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_SET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_RESET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    YM_RESET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    _us_delay(5);

#ifdef YM2612_USE_RTOS
    taskEXIT_CRITICAL();
#endif
}

void vYM2612_set_reg_preset(xFmDevice_t * pxRegPreset)
{
    uint8_t u8RegValue = 0U;

    /* Copy register preset */
    xYmDevice = *pxRegPreset;

    /* Set values on chip */
    (void)_get_device_register_value(&xYmDevice, YM2612_ADDR_LFO, &u8RegValue);
    vYM2612_write_reg(YM2612_ADDR_LFO, u8RegValue, YM2612_BANK_0);

    for (uint32_t u32IVoice = 0U; u32IVoice < YM2612_NUM_CHANNEL; u32IVoice++)
    {
        uint8_t u8BankOffset = u32IVoice / 3U;
        uint8_t u8ChannelOffset = u32IVoice % 3U;

        (void)_get_channel_register_value(&xYmDevice.xChannel[u32IVoice], YM2612_ADDR_FB_ALG, &u8RegValue);
        vYM2612_write_reg(YM2612_ADDR_FB_ALG + u8ChannelOffset, u8RegValue, u8BankOffset);

        (void)_get_channel_register_value(&xYmDevice.xChannel[u32IVoice], YM2612_ADDR_LR_AMS_PMS, &u8RegValue);
        vYM2612_write_reg(YM2612_ADDR_LR_AMS_PMS + u8ChannelOffset, u8RegValue, u8BankOffset);

        for (uint32_t u32IOperator = 0U; u32IOperator < YM2612_NUM_OP_CHANNEL; u32IOperator++)
        {
            uint8_t u8OpOffset = u32IOperator * 4;

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_DET_MULT, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_DET_MULT + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_TOT_LVL, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_TOT_LVL + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_KS_AR, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_KS_AR + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_AM_DR, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_AM_DR + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_SR, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_SR + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_SL_RR, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_SL_RR + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);

            (void)_get_operator_register_value(&xYmDevice.xChannel[u32IVoice].xOperator[u32IOperator], YM2612_ADDR_SSG_EG, &u8RegValue);
            vYM2612_write_reg(YM2612_ADDR_SSG_EG + u8ChannelOffset + u8OpOffset, u8RegValue, u8BankOffset);
        }
    }
}

xFmDevice_t * pxYM2612_get_reg_preset (void)
{
    return &xYmDevice;
}

bool bYM2612_set_note(YM2612_ch_id_t xChannel, uint8_t u8MidiNote)
{
    bool bRetval = false;
    bool bBankSelection = xChannel / 3;
    uint8_t u8Addr = 0U;
    uint8_t u8Data = 0U;
    uint8_t u8ChannelOffset = xChannel % 3U;
    uint8_t u8NoteIndex = u8MidiNote % NUM_NOTES_OCTAVE;
    uint8_t u8OctaveIndex = u8MidiNote / NUM_NOTES_OCTAVE;
    uint16_t u16fnum = u16OctaveBaseValues[u8NoteIndex];
    uint8_t u8BlockOffset = BASE_BLOCK;

    if (BASE_BLOCK > u8OctaveIndex)
    {
        u8BlockOffset -= BASE_BLOCK - u8OctaveIndex;
    }
    else if (BASE_BLOCK < u8OctaveIndex)
    {
        u8BlockOffset += u8OctaveIndex - BASE_BLOCK;
    }

    if (u8BlockOffset < MAX_BLOCK)
    {
#ifdef YM2612_DEBUG
        vCliPrintf("DBG", "Fnum %d, Block %d", u16fnum, u8BlockOffset);
#endif

        u8Addr = YM2612_ADDR_FNUM_2 + u8ChannelOffset;
        u8Data = (u16fnum >> 8U) & 0x07U;
        u8Data |= (u8BlockOffset & 0x07U) << 3U;
        vYM2612_write_reg(u8Addr, u8Data, bBankSelection);

        u8Addr = YM2612_ADDR_FNUM_1 + u8ChannelOffset;
        u8Data = u16fnum & 0xFFU;
        vYM2612_write_reg(u8Addr, u8Data, bBankSelection);

        bRetval = true;
    }
    else
    {
#ifdef YM2612_DEBUG
        vCliPrintf("DBG", "Block %d Out of range", u8BlockOffset);
#endif
    }

    return bRetval;
}

void vYM2612_key_on(YM2612_ch_id_t xChannel)
{
    uint8_t u8ChannelOffset = (xChannel % 3U) | ((xChannel / 3U) << 2U);
    vYM2612_write_reg(YM2612_ADDR_KEY_ON_OFF, 0xF0 | u8ChannelOffset, 0);
}

void vYM2612_key_off(YM2612_ch_id_t xChannel)
{
    uint8_t u8ChannelOffset = (xChannel % 3U) | ((xChannel / 3U) << 2U);
    vYM2612_write_reg(YM2612_ADDR_KEY_ON_OFF, 0x00 | u8ChannelOffset, 0);
}

/*****END OF FILE****/
