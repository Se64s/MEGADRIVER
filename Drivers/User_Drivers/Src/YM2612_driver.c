/**
  ******************************************************************************
  * @file           : YM2612_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "YM2612_driver.h"
#include "stm32g0xx_hal.h"

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
#define TICKS_USEC  10U

/* Private macro -------------------------------------------------------------*/

/* Bit handling macro */
#define YM_SET_BIT(PORT, BITS)    ((PORT)->BSRR = (uint32_t)(BITS))
#define YM_RESET_BIT(PORT, BITS)  ((PORT)->BRR = (uint32_t)(BITS))

/* Private variables ---------------------------------------------------------*/

/* Timer hanlder */
TIM_HandleTypeDef htim14;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Software delay
  * @param  microsec number of usecs to wait
  * @retval Operation status
*/
static void _us_delay(uint32_t microsec);

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

static void _us_delay(uint32_t microsec)
{
    uint32_t tick_count = TICKS_USEC * microsec;
    while (tick_count-- != 0)
    {
        __NOP();
    }
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

YM2612_status_t YM2612_init(void)
{
    YM2612_status_t retval = YM2612_STATUS_ERROR;
    
    _low_level_init();
    
    /* Init clock */
    if (HAL_TIM_OC_Start(&htim14, TIM_CHANNEL_1) == HAL_OK)
    {
        retval = YM2612_STATUS_OK;
    }

    /* Reset register values */
    YM_SET_BIT(YM2612_RD_GPIO_PORT, YM2612_RD_GPIO_PIN);
    YM_SET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);

    _us_delay(10);
    YM_RESET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);
    _us_delay(10);
    YM_SET_BIT(YM2612_REG_GPIO_PORT, YM2612_REG_GPIO_PIN);

    return (retval);
}

YM2612_status_t YM2612_deinit(void)
{
    YM2612_status_t retval = YM2612_STATUS_OK;
    
    _low_level_deinit();
    
    return (retval);
}

void YM2612_write_reg(uint8_t reg_addr, uint8_t reg_data, YM2612_bank_t bank)
{
    /* Write address */
    if (bank == YM2612_BANK_0)
    {
        YM_RESET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    }
    else
    {
        YM_SET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    }
    YM_RESET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
    YM_RESET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_Dx_GPIO_PORT, (reg_addr & 0xFF));
    YM_RESET_BIT(YM2612_Dx_GPIO_PORT, ((~reg_addr) & 0xFF));
    YM_RESET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);

    /* Operation delay */
    _us_delay(1);

    /* Clear bus */
    YM_SET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_SET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);

    /* Write data */
    YM_RESET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_SET_BIT(YM2612_Dx_GPIO_PORT, (reg_data & 0xFF));
    YM_RESET_BIT(YM2612_Dx_GPIO_PORT, ((~reg_data) & 0xFF));
    YM_RESET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);

    /* Operation delay */
    _us_delay(1);

    /* Clear resources */
    YM_SET_BIT(YM2612_WR_GPIO_PORT, YM2612_WR_GPIO_PIN);
    YM_SET_BIT(YM2612_CS_GPIO_PORT, YM2612_CS_GPIO_PIN);
    YM_RESET_BIT(YM2612_A1_GPIO_PORT, YM2612_A1_GPIO_PIN);
    YM_RESET_BIT(YM2612_A0_GPIO_PORT, YM2612_A0_GPIO_PIN);
}

/*****END OF FILE****/
