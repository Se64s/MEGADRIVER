/**
  ******************************************************************************
  * File Name          : stm32g0xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stm32g0xx_hal.h"
#include "stm32g0xx_ll_system.h"
#include "encoder_driver.h"
#include "user_error.h"

/* External decalrations -----------------------------------------------------*/

/* usart resources */
extern DMA_HandleTypeDef hdma_usart4_rx;
extern DMA_HandleTypeDef hdma_usart4_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;

/* i2c resources */
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;

/* ADC resources */
extern DMA_HandleTypeDef hdma_adc1;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* External functions --------------------------------------------------------*/

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/
    /* System clock init */
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Configure the main internal regulator output voltage 
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the CPU, AHB and APB busses clocks 
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    HAL_StatusTypeDef xRetval = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if (xRetval != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** Initializes the CPU, AHB and APB busses clocks 
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** Initializes the peripherals clocks 
     */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart2ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /* System flash init */
    FLASH_OBProgramInitTypeDef OBConsfig;

    /* read option bytes */
    HAL_FLASHEx_OBGetConfig(&OBConsfig);

    if ((OBConsfig.OptionType & OPTIONBYTE_USER) != 0x00u)
    {
        /* Check if serial bootloader is config, if not, enable it */
        if ((OBConsfig.USERConfig & OB_USER_nBOOT_SEL) != 0x00u)
        {
            HAL_FLASH_Unlock();
            HAL_FLASH_OB_Unlock();
            OBConsfig.OptionType = OPTIONBYTE_USER;
            OBConsfig.USERType = OB_USER_nBOOT_SEL | OB_USER_nBOOT1;
            OBConsfig.USERConfig = OB_BOOT0_FROM_PIN | OB_BOOT1_SYSTEM;
            HAL_FLASHEx_OBProgram(&OBConsfig);
        }
    }
}

/**
* @brief TIM_OC MSP Initialization
* This function configures the hardware resources used in this example
* @param htim_oc: TIM_OC handle pointer
* @retval None
*/
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef* htim_oc)
{
    if( htim_oc->Instance == TIM14 )
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM14_CLK_ENABLE();
    }
}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0U };

    if ( huart->Instance == USART4 )
    {
        /* DMA controller clock enable */
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* Peripheral clock enable */
        __HAL_RCC_USART4_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        /** USART4 GPIO Configuration
        PA0     ------> USART4_TX
        PA1     ------> USART4_RX 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_USART4;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART4 DMA Init */

        /* USART4_RX Init */
        hdma_usart4_rx.Instance = DMA1_Channel4;
        hdma_usart4_rx.Init.Request = DMA_REQUEST_USART4_RX;
        hdma_usart4_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart4_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart4_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart4_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart4_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart4_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart4_rx.Init.Priority = DMA_PRIORITY_LOW;

        if ( HAL_DMA_Init(&hdma_usart4_rx) != HAL_OK )
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(huart, hdmarx, hdma_usart4_rx);

        /* USART2_TX Init */
        hdma_usart4_tx.Instance = DMA1_Channel5;
        hdma_usart4_tx.Init.Request = DMA_REQUEST_USART4_TX;
        hdma_usart4_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart4_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart4_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart4_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart4_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart4_tx.Init.Mode = DMA_NORMAL;
        hdma_usart4_tx.Init.Priority = DMA_PRIORITY_LOW;

        if (HAL_DMA_Init(&hdma_usart4_tx) != HAL_OK)
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(huart, hdmatx, hdma_usart4_tx);

        /* DMA1_Channel2_3_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 3U, 0U);
        HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

        /* DMA1_Channel2_3_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(USART3_4_IRQn, 3U, 0U);
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
    else if ( huart->Instance == USART2 )
    {
        /* DMA controller clock enable */
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();

        /** USART2 GPIO Configuration
        PA3     ------> USART2_RX 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 DMA Init */
        hdma_usart2_rx.Instance = DMA1_Channel1;
        hdma_usart2_rx.Init.Request = DMA_REQUEST_USART2_RX;
        hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart2_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart2_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;

        if ( HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK )
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(huart, hdmarx, hdma_usart2_rx);

        /* DMA1_Channel1_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1U, 0U);
        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

        /* USART2 interrupt configuration */
        HAL_NVIC_SetPriority(USART2_IRQn, 1U, 0U);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if ( huart->Instance == USART4 )
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART4_CLK_DISABLE();

        /* Disable associated IRQ */
        HAL_NVIC_DisableIRQ(USART3_4_IRQn);

        /**USART2 GPIO Configuration
        PA0     ------> USART2_TX
        PA1     ------> USART2_RX 
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0 | GPIO_PIN_1);

        /* USART2 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);
        HAL_DMA_DeInit(huart->hdmatx);
    }
    else if ( huart->Instance == USART2 )
    {
        /* Peripheral clock disable */
        __HAL_RCC_USART2_CLK_DISABLE();

        /* Disable associated IRQ */
        HAL_NVIC_DisableIRQ(USART2_IRQn);

        /**USART3 GPIO Configuration
        PA3    ------> USART2_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_3);

        /* USART3 DMA DeInit */
        HAL_DMA_DeInit(huart->hdmarx);
    }
}

/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if ( hi2c->Instance == I2C1 )
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**I2C1 GPIO Configuration    
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();

        /* I2C1 DMA Init */

        /* DMA controller clock enable */
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* I2C1_RX Init */
        hdma_i2c1_rx.Instance = DMA1_Channel3;
        hdma_i2c1_rx.Init.Request = DMA_REQUEST_I2C1_RX;
        hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
        hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;

        if (HAL_DMA_Init(&hdma_i2c1_rx) != HAL_OK)
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(hi2c,hdmarx,hdma_i2c1_rx);

        /* I2C1_TX Init */
        hdma_i2c1_tx.Instance = DMA1_Channel2;
        hdma_i2c1_tx.Init.Request = DMA_REQUEST_I2C1_TX;
        hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
        hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;

        if (HAL_DMA_Init(&hdma_i2c1_tx) != HAL_OK)
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(hi2c,hdmatx,hdma_i2c1_tx);

        /* DMA1_Channel2_3_IRQn interrupt configuration */
        HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

        HAL_NVIC_SetPriority(I2C1_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(I2C1_IRQn);
  }
}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
    if( hi2c->Instance == I2C1 )
    {
        /* Peripheral clock disable */
        __HAL_RCC_I2C1_CLK_DISABLE();

        /**I2C1 GPIO Configuration    
        PB6     ------> I2C1_SCL
        PB7     ------> I2C1_SDA 
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);

        /* I2C1 DMA DeInit */
        HAL_DMA_DeInit(hi2c->hdmarx);
        HAL_DMA_DeInit(hi2c->hdmatx);

        HAL_NVIC_DisableIRQ(DMA1_Channel2_3_IRQn);
        HAL_NVIC_DisableIRQ(I2C1_IRQn);
    }
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if ( hadc->Instance == ADC1 )
    {
        RCC_PeriphCLKInitTypeDef PeriphClkInit;

        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
        PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;

        if ( HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK )
        {
            ERR_ASSERT(0U);
        }

        /* Peripheral clock enable */
        __HAL_RCC_ADC_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /** ADC1 GPIO Configuration 
        PB0     ------> ADC1_IN8 
        PB1     ------> ADC1_IN9 
        PB2     ------> ADC1_IN10 
        PB10    ------> ADC1_IN11
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* DMA controller clock enable */
        __HAL_RCC_DMA1_CLK_ENABLE();

        /* ADC1 DMA Init */
        hdma_adc1.Instance = DMA1_Channel6;
        hdma_adc1.Init.Request = DMA_REQUEST_ADC1;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_adc1.Init.Mode = DMA_CIRCULAR;
        hdma_adc1.Init.Priority = DMA_PRIORITY_MEDIUM;

        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
        {
            ERR_ASSERT(0U);
        }

        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

        /* DMA interrupt init */
        HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 3U, 0U);
        HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

        /* ADC1 interrupt Init */
        HAL_NVIC_SetPriority(ADC1_IRQn, 3U, 0U);
        HAL_NVIC_EnableIRQ(ADC1_IRQn);
    }
}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if( hadc->Instance == ADC1 )
    {
        /* Peripheral clock disable */
        __HAL_RCC_ADC_CLK_DISABLE();

        /**ADC1 GPIO Configuration
        PB0     ------> ADC1_IN8 
        PB1     ------> ADC1_IN9 
        PB2     ------> ADC1_IN10 
        PB10    ------> ADC1_IN11
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_10);

        /* ADC1 DMA DeInit */
        HAL_DMA_DeInit(hadc->DMA_Handle);

        /* ADC1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(ADC1_IRQn);
    }
}

/**
  * @brief  Initializes the TIM Encoder Interface MSP.
  * @param  htim TIM Encoder Interface handle
  * @retval None
  */
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = { 0U };

    if ( htim->Instance == TIM3 )
    {
        /* Init timer */
        __HAL_RCC_TIM3_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();

        /**TIM3 GPIO Configuration
         * PB4     ------> TIM3_CH1 
         * PB5     ------> TIM3_CH2 
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 3U, 0U);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
}

/**
  * @brief  DeInitializes TIM Encoder Interface MSP.
  * @param  htim TIM Encoder Interface handle
  * @retval None
  */
void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM3)
  {
    HAL_GPIO_DeInit(GPIOB, (GPIO_PIN_4 | GPIO_PIN_5));

    HAL_NVIC_DisableIRQ(TIM3_IRQn);
  }
}

/**
  * @brief  Init EXTI lines.
  * @param  None.
  * @retval None.
  */
void HAL_EXTI_GPIO_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /**SW GPIO Configuration
     * PB3    ------> EXTI_IN 
     */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 3U, 0U);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

/**
  * @brief  Deinit EXTI lines.
  * @param  None.
  * @retval None.
  */
void HAL_EXTI_GPIO_MspDeInit(void)
{
    /* Deinit gpio */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3);

    /* Deinit irq */
    HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);
}

/*****END OF FILE****/
