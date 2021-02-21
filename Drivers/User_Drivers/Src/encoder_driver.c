/**
  ******************************************************************************
  * @file           : encoder_driver.c
  * @brief          : Low level driver to manage YM2612
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "encoder_driver.h"
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Hanlder for timer 3*/
TIM_HandleTypeDef htim3;

/* Counter value */
volatile uint32_t u32Encoder0Cnt = 0U;

/* Last encoder tick event */
volatile uint32_t u32Encoder0EcTick = 0U;
volatile uint32_t u32Encoder0SwTick = 0U;

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

/**
  * @brief  Get system time.
  * @retval None
*/
static uint32_t __enc_0_low_level_get_time(void);

/* Low level funtions to init peripherals */
extern void HAL_EXTI_GPIO_MspInit(void);

/* Low level funtions to deinit peripherals */
extern void HAL_EXTI_GPIO_MspDeInit(void);

/* Private user code ---------------------------------------------------------*/

static void __enc_error_handler(void)
{
    ERR_ASSERT(0U);
}

static void __enc_0_low_level_init(void)
{
    TIM_Encoder_InitTypeDef sEncoder = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* Init SW */
    HAL_EXTI_GPIO_MspInit();

    /* Init timer */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = ENCODER_0_RANGE;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    sEncoder.EncoderMode = TIM_ENCODERMODE_TI12;

    sEncoder.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoder.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoder.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoder.IC1Filter = 0x0FU;

    sEncoder.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoder.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoder.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoder.IC2Filter = 0x0FU;

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

    /* Init encoder */
    (&htim3)->Instance->CNT = ENCODER_0_REF_VALUE;
}

static void __enc_0_low_level_deinit(void)
{
    /* Deinit tim */
    HAL_TIM_Encoder_DeInit(&htim3);

    HAL_EXTI_GPIO_MspDeInit();
}

static uint32_t __enc_0_low_level_get_time(void)
{
    return HAL_GetTick();
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

encoder_status_t ENCODER_getCount(encoder_id_t xDevId, uint32_t * pu32Count)
{
    encoder_status_t retval = ENCODER_STATUS_NOTDEF;

    if (xDevId == ENCODER_ID_0)
    {
        *pu32Count = u32Encoder0Cnt;
        
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

void ENCODER_irqEncHandler(encoder_id_t xDevId, uint32_t u32IrqCount)
{
    if (xDevId == ENCODER_ID_0)
    {
        uint32_t u32EventTick = __enc_0_low_level_get_time();

        /* Filter event by time */
        if ((u32EventTick - u32Encoder0EcTick) > ENCODER_0_TICK_CNT_GUARD_EC)
        {
            uint32_t u32EncEvent = ENCODER_0_VALUE_NONE;

            /* Check encoder direction and generate event */
            if ((u32IrqCount > u32Encoder0Cnt) || (u32IrqCount == ENCODER_0_CNT_MAX))
            {
                u32EncEvent = ENCODER_0_VALUE_CCW;
            }
            else if ((u32IrqCount < u32Encoder0Cnt) || (u32IrqCount == ENCODER_0_CNT_MIN))
            {
                u32EncEvent = ENCODER_0_VALUE_CW;
            }

            if (u32EncEvent != ENCODER_0_VALUE_NONE)
            {
                /* Update internal CNT value */
                u32Encoder0Cnt = u32IrqCount;

                /* Report event */
                if (encoder_0_event_cb != NULL)
                {
                    encoder_0_event_cb(ENCODER_EVENT_UPDATE, u32EncEvent);
                }

                /* Register tick event */
                u32Encoder0EcTick = u32EventTick;
            }
        }
    }
}

void ENCODER_irqSwHandler(encoder_id_t xDevId, encoder_sw_state_t xSwitchState)
{
    if (xDevId == ENCODER_ID_0)
    {
        if (xSwitchState == ENCODER_SW_RESET)
        {
            /* Wait guard time */
            uint32_t u32SwEventTick = __enc_0_low_level_get_time();

            if ((u32SwEventTick - u32Encoder0SwTick) > ENCODER_0_TICK_CNT_GUARD_SW)
            {
                uint32_t u32SwState = (uint32_t)xSwitchState;

                if (encoder_0_event_cb != NULL)
                {
                    encoder_0_event_cb(ENCODER_EVENT_SW, u32SwState);
                }

                u32Encoder0SwTick = u32SwEventTick;
            }
        }
    }
}

/*****END OF FILE****/
