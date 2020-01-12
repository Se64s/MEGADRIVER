/**
  ******************************************************************************
  * @file           : spi_driver.c
  * @brief          : Driver to handle spi peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "spi_driver.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#define SPI1_CS_RISE()      (GPIOA->BSRR = (uint32_t)GPIO_PIN_4)
#define SPI1_CS_FALL()      (GPIOA->BRR = (uint32_t)GPIO_PIN_4)

/* Private variables ---------------------------------------------------------*/

/* HAL peripheral handlers */
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

/* Event callback handler */
static spi_event_cb spi1_event_cb = NULL;

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void);

/**
  * @brief SPI1 Deinitialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Deinit(void);

/* Private user code ---------------------------------------------------------*/

static void MX_SPI1_Init(void)
{
    /* SPI1 parameter configuration*/
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_HARD_OUTPUT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        while(1);
    }

    /* DMA1_Channel2_3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

    /* SPI1 interrupt Init */
    HAL_NVIC_SetPriority(SPI1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
}

static void MX_SPI1_Deinit(void)
{
    /* SPI1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(SPI1_IRQn);

    if (HAL_SPI_DeInit(&hspi1) != HAL_OK)
    {
        while(1);
    }
}

/* Callback ------------------------------------------------------------------*/

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        if (spi1_event_cb != NULL)
        {
            spi1_event_cb(SPI_EVENT_ERROR);
        }
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        if (spi1_event_cb != NULL)
        {
            spi1_event_cb(SPI_EVENT_ERROR);
        }
    }
}

/* Public user code ----------------------------------------------------------*/

spi_status_t SPI_init(spi_port_t dev, spi_event_cb event_cb)
{
    spi_status_t retval = SPI_STATUS_NOTDEF;
    
    if (dev == SPI_0)
    {
        MX_SPI1_Init();

        if (event_cb != NULL)
        {
            spi1_event_cb = event_cb;
        }

        retval = SPI_STATUS_OK;
    }

    return(retval);
}

spi_status_t SPI_deinit(spi_port_t dev)
{
    spi_status_t retval = SPI_STATUS_NOTDEF;
    
    if (dev == SPI_0)
    {
        MX_SPI1_Deinit();
        spi1_event_cb = NULL;
        retval = SPI_STATUS_OK;
    }
    
    return(retval);
}

spi_status_t SPI_send(spi_port_t dev, uint8_t *pdata, uint16_t len)
{
    spi_status_t retval = SPI_STATUS_NOTDEF;

    if (dev == SPI_0)
    {
        HAL_StatusTypeDef op_status;
        
        /* Wait until periph ready */
        while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);

        op_status = HAL_SPI_Transmit_DMA(&hspi1, pdata, len);

        if (op_status == HAL_OK)
        {
            retval = SPI_STATUS_OK;
        }
        else
        {
            retval = SPI_STATUS_ERROR;
        }
    }

    return(retval);
}

/*****END OF FILE****/
