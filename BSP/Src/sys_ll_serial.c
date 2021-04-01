/**
 * @file sys_ll_serial.c
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief System support packet to handle low level serial interfaces
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Private includes --------------------------------------------------------*/

#include "sys_ll_serial.h"

#include "user_error.h"

#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"

/* Private defines ---------------------------------------------------------*/
/* Private variable --------------------------------------------------------*/
/* Private macro -----------------------------------------------------------*/
/* Private functions prototypes --------------------------------------------*/

/**
 * @brief Init serial interface 0. BSP wrapper.
 * 
 */
static void BSP_Serial0Init(void);

/**
 * @brief Send data over serial interface 0. BSP wrapper.
 * 
 * @param buf buffer to send
 * @param len number of bytes to send
 */
static void BSP_Serial0Send(uint8_t* buf, uint8_t len);

/* Private functions definition --------------------------------------------*/

static void BSP_Serial0Init(void)
{
    LL_USART_InitTypeDef USART_InitStruct = {0U};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0U};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    /**USART2 GPIO Configuration  
     PA2   ------> USART2_TX
    PA3   ------> USART2_RX 
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate = 115200U;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);
    LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_DisableFIFO(USART2);
    LL_USART_ConfigAsyncMode(USART2);

    LL_USART_Enable(USART2);

    /* Polling USART2 initialisation */
    while((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2))))
    {
    }
}

static void BSP_Serial0Send(uint8_t* buf, uint8_t len)
{
    uint8_t l = 0U;
    while (l < len)
    {
        /* Wait for TXE flag to be raised */
        while (!LL_USART_IsActiveFlag_TXE(USART2))
        {
        }

        /* If last char to be sent, clear TC flag */
        if (l == (len - 1U))
        {
            LL_USART_ClearFlag_TC(USART2);
        }

        /* Write character in Transmit Data register.
        TXE flag is cleared by writing data in TDR register */
        LL_USART_TransmitData8(USART2, buf[l]);

        l++;
    }

    /* Wait for TC flag to be raised for last char */
    while (!LL_USART_IsActiveFlag_TC(USART2))
    {
    }
}

/* Public function definition ----------------------------------------------*/

sys_ll_serial_status_t SYS_LL_UartInit(sys_ll_serial_port_t dev)
{
    ERR_ASSERT(dev == SYS_LL_SERIAL_0);

    sys_ll_serial_status_t eRetval = SYS_LL_SERIAL_STATUS_ERROR;

    if (dev == SYS_LL_SERIAL_0)
    {
        BSP_Serial0Init();
    }

    return eRetval;
}

sys_ll_serial_status_t SYS_LL_UartSend(sys_ll_serial_port_t dev, uint8_t* buf, uint8_t len)
{
    ERR_ASSERT(dev == SYS_LL_SERIAL_0);

    sys_ll_serial_status_t eRetval = SYS_LL_SERIAL_STATUS_ERROR;

    if (dev == SYS_LL_SERIAL_0)
    {
        BSP_Serial0Send(buf, len);
    }

    return eRetval;
}

/*EOF*/