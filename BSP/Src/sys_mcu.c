/**
 * @file sys_mcu.c
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief BSP to init base clock and flash config
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Includes -----------------------------------------------------------------*/
#include "sys_mcu.h"
#include "stm32g0xx_hal.h"
#include "user_error.h"
#include "sys_ll_serial.h"

/* Private variables --------------------------------------------------------*/
/* Private macro -----------------------------------------------------------*/
/* Private declarations -----------------------------------------------------*/

/**
  * @brief  Init resources for init errors and show info by serial interface
  * @param  msg: error message
  * @param  len: size of error msg
  * @retval None
  */
static void vErrorHandler(char *msg, uint32_t len);

/* Private definitions ------------------------------------------------------*/

static void vErrorHandler(char *msg, uint32_t len)
{
    uint8_t *pu8MsgAddr = (uint8_t*)msg;
    uint16_t u8MsgLen = (uint16_t)len;

    /* Init interface */
    (void)SYS_LL_UartInit(SYS_LL_SERIAL_0);

    /* Send data */
    (void)SYS_LL_UartSend(SYS_LL_SERIAL_0, pu8MsgAddr, u8MsgLen);
}

/* Public functions ---------------------------------------------------------*/

void SYS_Init(void)
{
    HAL_Init();

    /* Init Error interface */
    vErrorInit(vErrorHandler);
}

void SYS_SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0U};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0U};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0U};

    /** Configure the main internal regulator output voltage
     */
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
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

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** Initializes the CPU, AHB and APB buses clocks
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
}

void SYS_Reset(void)
{
    (void)HAL_NVIC_SystemReset();
}

/*EOF*/