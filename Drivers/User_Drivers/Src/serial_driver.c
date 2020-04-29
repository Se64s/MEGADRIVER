/**
  ******************************************************************************
  * @file           : serial_driver.c
  * @brief          : Driver to handle serial peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "serial_driver.h"
#include "circular_buffer.h"
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Serial 0 resources */
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
static circular_buf_t cbuff_uart3;
static uint8_t rx_buf_uart3[SERIAL_0_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart3[SERIAL_0_CBUF_SIZE] = {0};
static serial_event_cb uart3_event_cb = NULL;

/* Serial 1 resources */
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
static circular_buf_t cbuff_uart2;
static uint8_t rx_buf_uart2[SERIAL_1_RX_SIZE] = {0};
static uint8_t rx_cbuf_uart2[SERIAL_1_CBUF_SIZE] = {0};
static serial_event_cb uart2_event_cb = NULL;

/* Private function prototypes -----------------------------------------------*/

static void _error_handler(void);

static void BSP_USART2_UART_Init(void);
static void BSP_USART2_UART_Deinit(void);

static void BSP_USART3_UART_Init(void);
static void BSP_USART3_UART_Deinit(void);

/* Private user code ---------------------------------------------------------*/

/**
  * @brief Error Handler for this module
  * @param None.
  * @retval None.
  */
static void _error_handler(void)
{
    ERR_ASSERT(0U);
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void BSP_USART2_UART_Init(void)
{
    /* DMA1_Channel2_3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Ch4_7_DMAMUX1_OVR_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Ch4_7_DMAMUX1_OVR_IRQn);

    /* DMA1_Channel2_3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

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
        _error_handler();
    }
    if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        _error_handler();
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        _error_handler();
    }
    if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
    {
        _error_handler();
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
static void BSP_USART2_UART_Deinit(void)
{
    /* Disable associated IRQ */
    HAL_NVIC_DisableIRQ(USART2_IRQn);

    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart2) != HAL_OK)
    {
        _error_handler();
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart2);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart2, UART_IT_IDLE);
}

static void BSP_USART3_UART_Init(void)
{
    /* DMA1_Channel2_3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    /* DMA1_Channel2_3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(USART3_4_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART3_4_IRQn);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 31250;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        _error_handler();
    }

    /* Init additional resurces */
    circular_buf_init(&cbuff_uart3, rx_cbuf_uart3, SERIAL_0_CBUF_SIZE);

    /* Enable idle irq */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

static void BSP_USART3_UART_Deinit(void)
{
    /* Disable associated IRQ */
    HAL_NVIC_DisableIRQ(USART3_4_IRQn);

    /* Deinit peripheral */
    if (HAL_UART_DeInit(&huart3) != HAL_OK)
    {
        _error_handler();
    }

    /* Init additional resurces */
    circular_buf_free(&cbuff_uart3);

    /* Disable idle irq */
    __HAL_UART_DISABLE_IT(&huart3, UART_IT_IDLE);
}

/* Callback ------------------------------------------------------------------*/

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SERIAL_EVENT_TX_DONE);
        }
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uint8_t * pdata = rx_buf_uart2;
        uint32_t rx_size = SERIAL_1_RX_SIZE;
        while (rx_size-- != 0)
        {
            if (circular_buf_put2(&cbuff_uart2, *pdata++) != 0)
            {
                if (uart2_event_cb != NULL)
                {
                    uart2_event_cb(SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_1_RX_SIZE);
    }
    else if (huart->Instance == USART3)
    {
        uint8_t * pdata = rx_buf_uart3;
        uint32_t rx_size = SERIAL_0_RX_SIZE;
        while (rx_size-- != 0)
        {
            if (circular_buf_put2(&cbuff_uart3, *pdata++) != 0)
            {
                if (uart3_event_cb != NULL)
                {
                    uart3_event_cb(SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_0_RX_SIZE);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SERIAL_EVENT_ERROR);
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_1_RX_SIZE);
    }
    else if (huart->Instance == USART3)
    {
        if (uart3_event_cb != NULL)
        {
            uart3_event_cb(SERIAL_EVENT_ERROR);
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_0_RX_SIZE);
    }
}

void HAL_UART_AbortCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        uint8_t * pdata = rx_buf_uart2;
        uint32_t rx_size = SERIAL_1_RX_SIZE - huart->hdmarx->Instance->CNDTR;
        while (rx_size != 0)
        {
            if (circular_buf_put2(&cbuff_uart2, *pdata++) != 0)
            {
                if (uart2_event_cb != NULL)
                {
                    uart2_event_cb(SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
            rx_size--;
        }
        HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_1_RX_SIZE);

        if (uart2_event_cb != NULL)
        {
            uart2_event_cb(SERIAL_EVENT_RX_IDLE);
        }
    }
    else if (huart->Instance == USART3)
    {
        uint8_t * pdata = rx_buf_uart3;
        uint32_t rx_size = SERIAL_0_RX_SIZE - huart->hdmarx->Instance->CNDTR;
        while (rx_size != 0)
        {
            if (circular_buf_put2(&cbuff_uart3, *pdata++) != 0)
            {
                if (uart3_event_cb != NULL)
                {
                    uart3_event_cb(SERIAL_EVENT_RX_BUF_FULL);
                }
                break;
            }
            rx_size--;
        }
        HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_0_RX_SIZE);

        if (uart3_event_cb != NULL)
        {
            uart3_event_cb(SERIAL_EVENT_RX_IDLE);
        }
    }
}

/* Public user code ----------------------------------------------------------*/

serial_status_t SERIAL_init(serial_port_t dev, serial_event_cb event_cb)
{
    serial_status_t retval = SERIAL_STATUS_ERROR;

    if (dev == SERIAL_0)
    {
        /* Init hardware */
        BSP_USART3_UART_Init();

        if (event_cb != NULL)
        {
            uart3_event_cb = event_cb;
        }

        /* Enable reading */
        if (HAL_UART_Receive_DMA(&huart3, rx_buf_uart3, SERIAL_0_RX_SIZE) == HAL_OK)
        {
            retval = SERIAL_STATUS_OK;
        }
    }
    else if (dev == SERIAL_1)
    {
        /* Init hardware */
        BSP_USART2_UART_Init();

        if (event_cb != NULL)
        {
            uart2_event_cb = event_cb;
        }

        /* Enable reading */
        if (HAL_UART_Receive_DMA(&huart2, rx_buf_uart2, SERIAL_1_RX_SIZE) == HAL_OK)
        {
            retval = SERIAL_STATUS_OK;
        }
    }
    else
    {
        /* Nothing to do */
    }

    return (retval);
}

serial_status_t SERIAL_deinit(serial_port_t dev)
{
    serial_status_t retval = SERIAL_STATUS_ERROR;

    if (dev == SERIAL_0)
    {
        BSP_USART3_UART_Deinit();
        retval = SERIAL_STATUS_OK;
    }
    else if (dev == SERIAL_1)
    {
        BSP_USART2_UART_Deinit();
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
    UART_HandleTypeDef * phuart = NULL;

    /* Get serial handler */
    if (dev == SERIAL_0)
    {
        /* code */
    }
    else if (dev == SERIAL_1)
    {
        phuart = &huart2;
    }
    else
    {
        /* Nothing to do */
    }

    /* If handler defined, process data */
    if (phuart != NULL)
    {
        HAL_StatusTypeDef hal_ret = HAL_UART_Transmit_DMA(phuart, pdata, len);

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

    return (retval);
}

uint16_t SERIAL_read(serial_port_t dev, uint8_t *pdata, uint16_t max_len)
{
    uint16_t n_read = 0;

    /* Get serial handler */
    if (dev == SERIAL_0)
    {
        HAL_NVIC_DisableIRQ(USART3_4_IRQn);
        for (uint32_t i_data = 0; i_data < max_len; i_data++)
        {
            if (circular_buf_get(&cbuff_uart3, &pdata[i_data]) == 0)
            {
                n_read++;
            }
            else
            {
                break;
            }
        }
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
    else if (dev == SERIAL_1)
    {
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        for (uint32_t i_data = 0; i_data < max_len; i_data++)
        {
            if (circular_buf_get(&cbuff_uart2, &pdata[i_data]) == 0)
            {
                n_read++;
            }
            else
            {
                break;
            }
        }
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else
    {
        /* Nothing to do */
    }

    return (n_read);
}

uint16_t SERIAL_get_read_count(serial_port_t dev)
{
    uint16_t n_read = 0;

    /* Get serial handler */
    if (dev == SERIAL_0)
    {
        HAL_NVIC_DisableIRQ(USART3_4_IRQn);
        n_read = circular_buf_size(&cbuff_uart3);
        HAL_NVIC_EnableIRQ(USART3_4_IRQn);
    }
    else if (dev == SERIAL_1)
    {
        HAL_NVIC_DisableIRQ(USART2_IRQn);
        n_read = circular_buf_size(&cbuff_uart2);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else
    {
        /* Nothing to do */
    }

    return (n_read);
}

/*****END OF FILE****/
