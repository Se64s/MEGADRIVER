/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
#include "encoder_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/

extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim3;

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel 1 interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_i2c1_rx);
}

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_i2c1_tx);
}

/**
  * @brief This function handles DMA1 channel 4, channel 5, channel 6, channel 7 and DMAMUX1 interrupts.
  */
void DMA1_Ch4_7_DMAMUX1_OVR_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
}

/**
  * @brief This function handles I2C1 interrupts.
  */
void I2C1_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&hi2c1);
}

/**
  * @brief This function handles USART2 interrupts.
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart2);

  /* Handle idle event on usart */
  if (__HAL_UART_GET_IT(&huart2, UART_IT_IDLE))
  {
    __HAL_UART_CLEAR_IT(&huart2, UART_CLEAR_IDLEF);

    /* Abort and retrigger reception */
    HAL_UART_Abort_IT(&huart2);
  }
}

/**
  * @brief This function handles TIM3 interrupt.
  */
void TIM3_IRQHandler(void)
{
  uint32_t u32tmpCout = (&htim3)->Instance->CNT;
  
  /* Check encoder direction and generate event */
  if (u32tmpCout < ENCODER_0_MIN_TH)
  {
    (&htim3)->Instance->CNT = ENCODER_0_REF_VALUE;
    u32tmpCout = ENCODER_0_VALUE_CW;
    ENCODER_irq_handler(ENCODER_ID_0, u32tmpCout);
  }
  else if (u32tmpCout > ENCODER_0_MAX_TH)
  {
    (&htim3)->Instance->CNT = ENCODER_0_REF_VALUE;
    u32tmpCout = ENCODER_0_VALUE_CCW;
    ENCODER_irq_handler(ENCODER_ID_0, u32tmpCout);
  }

  /* Clear flags */
  HAL_TIM_IRQHandler(&htim3);
}

/**
  * @brief This function handles exti interrupt.
  */
void EXTI4_15_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(ENCODER_0_SW_GPIO_PIN);
}

/* Global callbacks */

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ENCODER_0_SW_GPIO_PIN)
  {
    ENCODER_irqSwHandler(ENCODER_ID_0, ENCODER_SW_SET);
  }
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == ENCODER_0_SW_GPIO_PIN)
  {
    ENCODER_irqSwHandler(ENCODER_ID_0, ENCODER_SW_RESET);
  }
}

/*****END OF FILE****/
