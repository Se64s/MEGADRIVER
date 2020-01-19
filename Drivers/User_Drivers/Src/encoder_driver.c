/**
  ******************************************************************************
  * @file           : encoder_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "encoder_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Hanlder for timer 3*/
TIM_HandleTypeDef htim3;

/* Counter value */
volatile uint32_t u32Encoder0Count = 0;

/* Callback handler */
static encoder_event_cb encoder_0_event_cb = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  Error handler
  * @retval None
*/
static void __enc_error_handler(void);

/**
  * @brief  Init hardware for enc 0
  * @retval None
*/
static void __enc_0_low_level_init(void);

/**
  * @brief  Deinit hardware for enc 0
  * @retval None
*/
static void __enc_0_low_level_deinit(void);

/* Private user code ---------------------------------------------------------*/

static void __enc_error_handler(void)
{
    while (1);
}

static void __enc_0_low_level_init(void)
{
    /* Init gpio */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    TIM_Encoder_InitTypeDef sEncoder = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* Init SW */

    /**SW GPIO Configuration
     * PA11    ------> EXTI_IN 
    */
    ENCODER_0_SW_GPIO_CLK();
    GPIO_InitStruct.Pin = ENCODER_0_SW_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(ENCODER_0_SW_GPIO_PORT, &GPIO_InitStruct);

    /* TIM3 interrupt Init */
    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

    /* Init timer */

    /**TIM3 GPIO Configuration
     * PC6     ------> TIM3_CH1 
     * PC7     ------> TIM3_CH2 
    */
    ENCODER_0_ENC_GPIO_CLK();
    GPIO_InitStruct.Pin = ENCODER_0_ENC_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
    HAL_GPIO_Init(ENCODER_0_ENC_GPIO_PORT, &GPIO_InitStruct);

    /* TIM3 interrupt Init */
    __HAL_RCC_TIM3_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    /* Init timer */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = ENCODER_0_RANGE;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sEncoder.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoder.IC1Polarity = TIM_ICPOLARITY_FALLING;
    sEncoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoder.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoder.IC1Filter = 15;
    sEncoder.IC2Polarity = TIM_ICPOLARITY_FALLING;
    sEncoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoder.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoder.IC2Filter = 15;
    
    if (HAL_TIM_Encoder_Init(&htim3, &sEncoder) != HAL_OK)
    {
        __enc_error_handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    
    if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
    {
        __enc_error_handler();
    }

    /* Start encoder */
    if (HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL) != HAL_OK)
    {
        __enc_error_handler();
    }
}

static void __enc_0_low_level_deinit(void)
{
    /* Deinit tim */
    HAL_TIM_Encoder_DeInit(&htim3);
    __HAL_RCC_TIM3_CLK_DISABLE();
    /* Deinit gpio */
    HAL_GPIO_DeInit(ENCODER_0_SW_GPIO_PORT, ENCODER_0_SW_GPIO_PIN);
    HAL_GPIO_DeInit(ENCODER_0_ENC_GPIO_PIN, ENCODER_0_ENC_GPIO_PORT);
    /* Deinit irq */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}

/* Callback ------------------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

encoder_status_t ENCODER_init(encoder_id_t xDevId, encoder_event_cb xEventCb)
{
    encoder_status_t retval = ENCODER_STATUS_NOTDEF;

    if (xDevId == ENCODER_ID_0)
    {
        __enc_0_low_level_init();

        if (xEventCb != NULL)
        {
            encoder_0_event_cb = xEventCb;
        }

        retval = ENCODER_STATUS_OK;
    }

    return retval;
}

encoder_status_t ENCODER_getCount(encoder_id_t xDevId, uint32_t * pu32CountVal)
{
    encoder_status_t retval = ENCODER_STATUS_NOTDEF;

    if (xDevId == ENCODER_ID_0)
    {
        *pu32CountVal = u32Encoder0Count;
        
        retval = ENCODER_STATUS_OK;
    }

    return retval;
}

encoder_sw_state_t ENCODER_getSwState(encoder_id_t xDevId)
{
    encoder_sw_state_t xSwitchState = ENCODER_SW_NOTDEF;

    if (xDevId == ENCODER_ID_0)
    {
        if (HAL_GPIO_ReadPin(ENCODER_0_SW_GPIO_PORT, ENCODER_0_SW_GPIO_PIN) == GPIO_PIN_SET)
        {
            xSwitchState = ENCODER_SW_SET;
        }
        else
        {
            xSwitchState = ENCODER_SW_RESET;
        }
    }

    return xSwitchState;
}

encoder_status_t ENCODER_deinit(encoder_id_t xDevId)
{
    encoder_status_t retval = ENCODER_STATUS_NOTDEF;

    if (xDevId == ENCODER_ID_0)
    {
        __enc_0_low_level_deinit();
        retval = ENCODER_STATUS_OK;
    }

    return retval;
}

void ENCODER_irq_handler(encoder_id_t xDevId, uint32_t u32EncCount)
{
    if (xDevId == ENCODER_ID_0)
    {
        u32Encoder0Count = u32EncCount;

        if (encoder_0_event_cb != NULL)
        {
            encoder_0_event_cb(ENCODER_EVENT_UPDATE, u32EncCount);
        }
    }
}

void ENCODER_irqSwHandler(encoder_id_t xDevId, encoder_sw_state_t xSwitchState)
{
    if (xDevId == ENCODER_ID_0)
    {
        uint32_t u32SwState = (uint32_t)xSwitchState;

        if (encoder_0_event_cb != NULL)
        {
            encoder_0_event_cb(ENCODER_EVENT_SW, u32SwState);
        }
    }
}

/*****END OF FILE****/
