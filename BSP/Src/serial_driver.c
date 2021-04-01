/**
  ******************************************************************************
  * @file           : serial_driver.c
  * @brief          : Driver to handle serial peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "serial_driver.h"
#include "circular_buffer.h"
#include "user_error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/** Serial port handler */
typedef struct SerialDevHandler
{
    serial_port_t xSerialId;
    UART_HandleTypeDef *pxHalPeriphHandler;
    circular_buf_t *pxAppCircularBuffer;
    uint8_t *pu8LowLevelBuffer;
    uint32_t u32LowLevelBufferSize;
    uint32_t u32LowLevelBufferPos;
    serial_event_cb pxEventCb;
} SerialDevHandler_t;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Serial 0 resources */
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
static uint8_t rx_buf_uart3[SERIAL_0_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart3[SERIAL_0_CBUF_SIZE] = {0};
static circular_buf_t cbuff_uart3 = {
    .buffer = rx_cbuf_uart3,
    .head = 0U,
    .tail = 0U,
    .max = SERIAL_0_CBUF_SIZE,
    .full = false,
};

static SerialDevHandler_t xSerial0Handler = {
    .xSerialId = SERIAL_0,
    .pxHalPeriphHandler = &huart3,
    .pxAppCircularBuffer = &cbuff_uart3,
    .pu8LowLevelBuffer = rx_buf_uart3,
    .u32LowLevelBufferSize = SERIAL_0_RX_SIZE,
    .pxEventCb = NULL
};

/* Serial 1 resources */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
static uint8_t rx_buf_uart2[SERIAL_1_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart2[SERIAL_1_CBUF_SIZE] = {0};
static circular_buf_t cbuff_uart2 = {
    .buffer = rx_cbuf_uart2,
    .head = 0U,
    .tail = 0U,
    .max = SERIAL_1_CBUF_SIZE,
    .full = false,
};

static SerialDevHandler_t xSerial1Handler = {
    .xSerialId = SERIAL_1,
    .pxHalPeriphHandler = &huart2,
    .pxAppCircularBuffer = &cbuff_uart2,
    .pu8LowLevelBuffer = rx_buf_uart2,
    .u32LowLevelBufferSize = SERIAL_1_RX_SIZE,
    .pxEventCb = NULL
};

/* Private function prototypes -----------------------------------------------*/

static void BSP_Init_Serial_0(void);
static void BSP_DeInit_Serial_0(void);

static void BSP_Init_Serial_1(void);
static void BSP_DeInit_Serial_1(void);

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief Function to proces rx buffer in following IRQs: IDLE DMA_TC and DMA_HT
 * 
 * @param huart 
 */
extern void HAL_UART_HandleRxEvent(UART_HandleTypeDef *huart);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void BSP_Init_Serial_1(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }
    if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /* Init additional resurces */
    circular_buf_init(&cbuff_uart2, rx_cbuf_uart2, SERIAL_1_CBUF_SIZE);

    /* Enable idle irq */
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
}

/**
  * @brief USART2 DeInitialization Function
  * @param None
  * @retval None
  */
static void BSP_DeInit_Serial_1(void)
{
    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart2) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart2);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
}

static void BSP_Init_Serial_0(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 31250;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_ENABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /* Init additional resurces */
    circular_buf_init(&cbuff_uart3, rx_cbuf_uart3, SERIAL_0_CBUF_SIZE);

    /* Enable idle irq */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

static void BSP_DeInit_Serial_0(void)
{
    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart3) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart3);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart3, UART_IT_IDLE);
}

/* Callback ------------------------------------------------------------------*/

void HAL_UART_HandleRxEvent(UART_HandleTypeDef *huart)
{
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial1Handler;
    }
    if (huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial0Handler;
    }

    if (pxSerialHandler != NULL)
    {
        uint32_t u32CurrentPos = pxSerialHandler->u32LowLevelBufferSize - huart->hdmarx->Instance->CNDTR;
        uint8_t * pdata = pxSerialHandler->pu8LowLevelBuffer + pxSerialHandler->u32LowLevelBufferPos;

        /* Detect change on buffer */
        if (u32CurrentPos != pxSerialHandler->u32LowLevelBufferPos)
        {
            uint32_t u32RxData = 0U;

            /* Case without overflow in circular buffer */
            if (u32CurrentPos > pxSerialHandler->u32LowLevelBufferPos)
            {
                u32RxData = u32CurrentPos - pxSerialHandler->u32LowLevelBufferPos;
                while (u32RxData-- != 0U)
                {
                    circular_buf_put2(pxSerialHandler->pxAppCircularBuffer, *pdata++);
                }
            }
            else
            {
                /* Case with overflow in circular buffer */
                u32RxData = pxSerialHandler->u32LowLevelBufferSize - pxSerialHandler->u32LowLevelBufferPos;
                while (u32RxData-- != 0U)
                {
                    circular_buf_put2(pxSerialHandler->pxAppCircularBuffer, *pdata++);
                }

                pdata = pxSerialHandler->pu8LowLevelBuffer;
                u32RxData = u32CurrentPos;
                while (u32RxData-- != 0U)
                {
                    circular_buf_put2(pxSerialHandler->pxAppCircularBuffer, *pdata++);
                }
            }

            if (pxSerialHandler->pxEventCb != NULL)
            {
                pxSerialHandler->pxEventCb(SERIAL_EVENT_RX_IDLE);
            }
        }

        /* Check if end of buffer */
        pxSerialHandler->u32LowLevelBufferPos = u32CurrentPos;
        if (pxSerialHandler->u32LowLevelBufferPos == pxSerialHandler->u32LowLevelBufferSize)
        {
            pxSerialHandler->u32LowLevelBufferPos = 0U;
        }
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial1Handler;
    }
    if (huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial0Handler;
    }

    if (pxSerialHandler != NULL)
    {
        if (pxSerialHandler->pxEventCb != NULL)
        {
            pxSerialHandler->pxEventCb(SERIAL_EVENT_TX_DONE);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance) || 
        (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance))
    {
        HAL_UART_HandleRxEvent(huart);
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if ((huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance) || 
        (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance))
    {
        HAL_UART_HandleRxEvent(huart);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial1Handler;
    }
    if (huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial0Handler;
    }

    if (pxSerialHandler != NULL)
    {
        if (pxSerialHandler->pxEventCb != NULL)
        {
            pxSerialHandler->pxEventCb(SERIAL_EVENT_ERROR);
        }

        pxSerialHandler->u32LowLevelBufferPos = 0U;
        HAL_UART_Receive_DMA(huart,
                            pxSerialHandler->pu8LowLevelBuffer,
                            pxSerialHandler->u32LowLevelBufferSize);
    }
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (huart->Instance == xSerial1Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial1Handler;
    }
    if (huart->Instance == xSerial0Handler.pxHalPeriphHandler->Instance)
    {
        pxSerialHandler = &xSerial0Handler;
    }

    if (pxSerialHandler != NULL)
    {
        pxSerialHandler->u32LowLevelBufferPos = 0U;
        HAL_UART_Receive_DMA(huart,
                            pxSerialHandler->pu8LowLevelBuffer,
                            pxSerialHandler->u32LowLevelBufferSize);
    }
}

/* Public user code ----------------------------------------------------------*/

serial_status_t SERIAL_init(serial_port_t dev, serial_event_cb event_cb)
{
    serial_status_t retval = SERIAL_STATUS_ERROR;
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (dev == SERIAL_0)
    {
        pxSerialHandler = &xSerial0Handler;

        /* Low level init for serial 0 */
        BSP_Init_Serial_0();
    }
    else if (dev == SERIAL_1)
    {
        pxSerialHandler = &xSerial1Handler;

        /* Low level init for serial 1 */
        BSP_Init_Serial_1();
    }

    if (pxSerialHandler != NULL)
    {
        if (event_cb != NULL)
        {
            pxSerialHandler->pxEventCb = event_cb;
        }

        /* Start rx */
        if ( HAL_UART_Receive_DMA(
                                  pxSerialHandler->pxHalPeriphHandler, 
                                  pxSerialHandler->pu8LowLevelBuffer, 
                                  pxSerialHandler->u32LowLevelBufferSize 
                                  ) == HAL_OK )
        {
            retval = SERIAL_STATUS_OK;
        }
    }

    return (retval);
}

serial_status_t SERIAL_deinit(serial_port_t dev)
{
    serial_status_t retval = SERIAL_STATUS_ERROR;

    if (dev == SERIAL_0)
    {
        BSP_DeInit_Serial_0();
        retval = SERIAL_STATUS_OK;
    }
    else if (dev == SERIAL_1)
    {
        BSP_DeInit_Serial_1();
        retval = SERIAL_STATUS_OK;
    }
    else
    {
        /* Nothing to do */
    }

    return (retval);
}

serial_status_t SERIAL_send(serial_port_t dev, uint8_t *pdata, uint16_t len)
{
    serial_status_t retval = SERIAL_STATUS_NODEF;
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (dev == SERIAL_0)
    {
        pxSerialHandler = NULL;
    }
    else if (dev == SERIAL_1)
    {
        pxSerialHandler = &xSerial1Handler;
    }

    if (pxSerialHandler != NULL)
    {
        HAL_StatusTypeDef hal_ret = HAL_UART_Transmit_DMA(pxSerialHandler->pxHalPeriphHandler, pdata, len);

        if (hal_ret == HAL_OK)
        {
            retval = SERIAL_STATUS_OK;
        }
        else if (hal_ret == HAL_BUSY)
        {
            retval = SERIAL_STATUS_BUSY;
        }
        else
        {
            retval = SERIAL_STATUS_ERROR;
        }
    }

    return retval;
}

uint16_t SERIAL_read(serial_port_t dev, uint8_t *pdata, uint16_t max_len)
{
    ERR_ASSERT(pdata != NULL);

    uint16_t n_read = 0;
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (dev == SERIAL_0)
    {
        pxSerialHandler = &xSerial0Handler;
    }
    else if (dev == SERIAL_1)
    {
        pxSerialHandler = &xSerial1Handler;
    }

    if (pxSerialHandler != NULL)
    {
        for (uint32_t i_data = 0; i_data < max_len; i_data++)
        {
            if (circular_buf_get(pxSerialHandler->pxAppCircularBuffer, pdata + i_data) == 0)
            {
                n_read++;
            }
            else
            {
                break;
            }
        }
    }

    return n_read;
}

uint16_t SERIAL_get_read_count(serial_port_t dev)
{
    uint16_t n_read = 0;
    SerialDevHandler_t * pxSerialHandler = NULL;

    if (dev == SERIAL_0)
    {
        pxSerialHandler = &xSerial0Handler;
    }
    else if (dev == SERIAL_1)
    {
        pxSerialHandler = &xSerial1Handler;
    }

    if (pxSerialHandler != NULL)
    {
        n_read = circular_buf_size(pxSerialHandler->pxAppCircularBuffer);
    }

    return n_read;
}

/*****END OF FILE****/
