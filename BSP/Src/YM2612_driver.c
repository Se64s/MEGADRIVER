/**
  ******************************************************************************
  * @file           : YM2612_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "YM2612_driver.h"
#include "user_error.h"

#ifdef YM2612_DEBUG
#include "cli_task.h"
#endif

/* External hardware */
#include "spi_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Number of ticks per microsec */
#define TICKS_USEC          (10U)

/* Number of notes per Octave */
#define NUM_NOTES_OCTAVE    (12U)

/* Base block define */
#define BASE_BLOCK          (4U)

/* Max block range */
#define MAX_BLOCK           (8U)

/* Internal peripheral assignation */
#define YM2612_SPI          ( SPI_1 )

/* Private macro -------------------------------------------------------------*/

/** Bit control macro - set bit */
#define YM_SET_BIT(reg, pos)    ( (reg) |= (uint16_t)( 1U << (pos) ) )

/** Bit control macro - reset bit */
#define YM_RESET_BIT(reg, pos)  ( (reg) &= (uint16_t)( ~( 1U << (pos) ) ) )

/* Private variables ---------------------------------------------------------*/

/* F_num base values - 12 notes */
const static uint16_t u16OctaveBaseValues[] = {
    617, 654, 693, 734,
    778, 824, 873, 925,
    980, 1038, 1100, 1165
};

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

/* Low level implementation  -------------------------------------------------*/

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
  * @brief  Low level function to write register values.
  * @param  u8RegAddr register address.
  * @param  u8RegData register data.
  * @param  xBank selected bank.
  * @retval None.
*/
static void _low_level_writeRegister(uint8_t u8RegAddr, uint8_t u8RegData, YM2612_bank_t xBank);

/**
 * @brief YM 2612 hardware reset.
 * @retval None.
 */
static void _low_level_resetDevice(void);

/**
 * @brief ll set Dx.
 * @param  u16DxData data to set.
 * @retval None.
 */
static void _low_level_SetRegData(uint16_t u16DxData);

/* Private user code ---------------------------------------------------------*/

static bool _get_device_register_value(xFmDevice_t * pxDevice, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    ERR_ASSERT(pxDevice != NULL);
    ERR_ASSERT(pu8RegData != NULL);

    uint8_t bRetVal = false;
    uint8_t u8RegData = 0U;

    if (u8RegAddr == YM2612_ADDR_LFO)
    {
        u8RegData = ((pxDevice->u8LfoOn & 0x01) << 3U) | (pxDevice->u8LfoFreq & 0x07);
        *pu8RegData = u8RegData;
        bRetVal = true;
    }

    return bRetVal;
}

static bool _get_channel_register_value(xFmChannel_t * pxChannel, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    ERR_ASSERT(pxChannel != NULL);
    ERR_ASSERT(pu8RegData != NULL);

    uint8_t bRetVal = false;
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

    return bRetVal;
}

static bool _get_operator_register_value(xFmOperator_t * pxOperator, uint8_t u8RegAddr, uint8_t * pu8RegData)
{
    ERR_ASSERT(pxOperator != NULL);
    ERR_ASSERT(pu8RegData != NULL);

    uint8_t bRetVal = false;
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

    return bRetVal;
}

static void _low_level_init(void)
{
    (void)SPI_init(YM2612_SPI, NULL);
}

static void _low_level_deinit(void)
{
    (void)SPI_deinit(YM2612_SPI);
}

static void _low_level_writeRegister(uint8_t u8RegAddr, uint8_t u8RegData, YM2612_bank_t xBank)
{
#ifdef YM2612_DEBUG
    vCliPrintf("DBG", "WR REG: %02X-%02X-%02X", u8RegAddr, u8RegData, xBank);
#endif

    uint16_t u16ShiftRegData = 0U;

    /* Set reset bit */
    YM_SET_BIT(u16ShiftRegData, YM_POS_RD);
    YM_SET_BIT(u16ShiftRegData, YM_POS_REG);

    /* Select address config */
    if (xBank == YM2612_BANK_0)
    {
        YM_RESET_BIT(u16ShiftRegData, YM_POS_A1);
    }
    else
    {
        YM_SET_BIT(u16ShiftRegData, YM_POS_A1);
    }
    YM_RESET_BIT(u16ShiftRegData, YM_POS_A0);
    YM_SET_BIT(u16ShiftRegData, YM_POS_WR);
    YM_SET_BIT(u16ShiftRegData, YM_POS_CS);
    _low_level_SetRegData(u16ShiftRegData);

    YM_RESET_BIT(u16ShiftRegData, YM_POS_CS);
    _low_level_SetRegData(u16ShiftRegData);

    u16ShiftRegData |= (uint16_t)u8RegAddr;
    YM_RESET_BIT(u16ShiftRegData, YM_POS_WR);
    _low_level_SetRegData(u16ShiftRegData);

    /* Clear bus */
    YM_SET_BIT(u16ShiftRegData, YM_POS_WR);
    YM_SET_BIT(u16ShiftRegData, YM_POS_CS);
    YM_SET_BIT(u16ShiftRegData, YM_POS_A0);
    _low_level_SetRegData(u16ShiftRegData);

    YM_RESET_BIT(u16ShiftRegData, YM_POS_CS);
    _low_level_SetRegData(u16ShiftRegData);

    /* Write data */
    u16ShiftRegData &= 0xFF00;
    u16ShiftRegData |= (uint16_t)u8RegData;
    YM_RESET_BIT(u16ShiftRegData, YM_POS_WR);
    _low_level_SetRegData(u16ShiftRegData);

    /* Clear resources */
    YM_SET_BIT(u16ShiftRegData, YM_POS_WR);
    YM_SET_BIT(u16ShiftRegData, YM_POS_CS);
    YM_RESET_BIT(u16ShiftRegData, YM_POS_A1);
    YM_RESET_BIT(u16ShiftRegData, YM_POS_A0);
    _low_level_SetRegData(u16ShiftRegData);
}

static void _low_level_resetDevice(void)
{
    uint16_t u16ShiftRegData = 0U;

    /* Reset register values */
    YM_SET_BIT(u16ShiftRegData, YM_POS_RD);
    YM_SET_BIT(u16ShiftRegData, YM_POS_REG);
    _low_level_SetRegData(u16ShiftRegData);

    YM_RESET_BIT(u16ShiftRegData, YM_POS_REG);
    _low_level_SetRegData(u16ShiftRegData);

    YM_SET_BIT(u16ShiftRegData, YM_POS_REG);
    _low_level_SetRegData(u16ShiftRegData);

    /* Set default state */
    YM_SET_BIT(u16ShiftRegData, YM_POS_WR);
    YM_SET_BIT(u16ShiftRegData, YM_POS_CS);
    YM_RESET_BIT(u16ShiftRegData, YM_POS_A1);
    YM_RESET_BIT(u16ShiftRegData, YM_POS_A0);
    _low_level_SetRegData(u16ShiftRegData);
}

static void _low_level_SetRegData(uint16_t u16DxData)
{
    uint8_t pu8RegData[2U] = { 0U };

    pu8RegData[0U] = u16DxData & 0xFF;
    pu8RegData[1U] = (u16DxData >> 8U) & 0xFF;

    (void)SPI_send(YM2612_SPI, pu8RegData, 2U);
}

/* Callback ------------------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

YM2612_status_t xYM2612_init(void)
{
    YM2612_status_t retval = YM2612_STATUS_OK;

    _low_level_init();

    _low_level_resetDevice();

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
    /* Wrapper for low level abstraction */
    _low_level_writeRegister(u8RegAddr, u8RegData, xBank);
}

void vYM2612_set_reg_preset(xFmDevice_t * pxRegPreset)
{
    ERR_ASSERT(pxRegPreset != NULL);

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
    ERR_ASSERT(xChannel < YM2612_NUM_CH);

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
    ERR_ASSERT(xChannel < YM2612_NUM_CH);

    uint8_t u8ChannelOffset = (xChannel % 3U) | ((xChannel / 3U) << 2U);
    vYM2612_write_reg(YM2612_ADDR_KEY_ON_OFF, 0xF0 | u8ChannelOffset, 0);
}

void vYM2612_key_off(YM2612_ch_id_t xChannel)
{
    ERR_ASSERT(xChannel < YM2612_NUM_CH);

    uint8_t u8ChannelOffset = (xChannel % 3U) | ((xChannel / 3U) << 2U);
    vYM2612_write_reg(YM2612_ADDR_KEY_ON_OFF, 0x00 | u8ChannelOffset, 0);
}

/*****END OF FILE****/
