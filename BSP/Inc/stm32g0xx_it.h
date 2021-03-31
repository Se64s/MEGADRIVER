/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.h
  * @brief   This file contains the headers of the interrupt handlers.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32G0xx_IT_H
#define __STM32G0xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Private includes ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

void NMI_Handler(void);
#ifndef CUSTOM_HARD_FAULT
void HardFault_Handler(void);
#endif
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_3_IRQHandler(void);
void DMA1_Ch4_7_DMAMUX1_OVR_IRQHandler(void);
void I2C1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_4_IRQHandler(void);
void TIM3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);
void ADCx_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32G0xx_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
