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

extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
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
  * @brief This function handles DMA1 channel 4, channel 5, channel 6, channel 7 and DMAMUX1 interrupts.
  */
void DMA1_Ch4_7_DMAMUX1_OVR_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
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
  
  /* Handle min value to prevent overflow */
  if (u32tmpCout < ENCODER_0_MIN_TH)
  {
    (&htim3)->Instance->CNT = ENCODER_0_MIN_TH;
    u32tmpCout = 0;
  }
  /* Handle max value to prevent overflow */
  else if (u32tmpCout > ENCODER_0_MAX_TH)
  {
    (&htim3)->Instance->CNT = ENCODER_0_MAX_TH;
    u32tmpCout = ENCODER_0_RANGE;
  }
  
  /* Generate event */
  ENCODER_irq_handler(ENCODER_ID_0, u32tmpCout);

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
