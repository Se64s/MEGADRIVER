/**
  ******************************************************************************
  * @file           : spi_driver.c
  * @brief          : Driver to handle spi peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "spi_driver.h"
#include "user_error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/** SPI port handler */
typedef struct SpiHandler
{
    spi_port_t ePort;
    SPI_HandleTypeDef *pxHalPeriphHandler;
    uint8_t *pu8BufferTx;
    uint16_t u16BufferTxSize;
    uint8_t *pu8BufferRx;
    uint16_t u16BufferRxSize;
    spi_event_cb pxEventCb;
} SpiHandler_t;

/* Private define ------------------------------------------------------------*/

#define SPI1_TX_LEN         ( 16U )
#define SPI1_RX_LEN         ( 16U )

#define SPI2_TX_LEN         ( 2U )
#define SPI2_RX_LEN         ( 0U )

/* Private macro -------------------------------------------------------------*/

#define SPI0_CS_RISE()      ( GPIOA->BSRR = (uint32_t)GPIO_PIN_4 )
#define SPI0_CS_FALL()      ( GPIOA->BRR = (uint32_t)GPIO_PIN_4 )

#define SPI1_CS_RISE()      ( GPIOB->BSRR = (uint32_t)GPIO_PIN_12 )
#define SPI1_CS_FALL()      ( GPIOB->BRR = (uint32_t)GPIO_PIN_12 )

/* Private variables ---------------------------------------------------------*/

/* HAL peripheral handlers */

/* SPI1 Handler */
SPI_HandleTypeDef hspi1;
uint8_t u8Spi1BufferTx[SPI1_TX_LEN] = { 0U };
uint8_t u8Spi1BufferRx[SPI1_RX_LEN] = { 0U };

SpiHandler_t xSpi0Handler = {
    .ePort = SPI_0,
    .pxHalPeriphHandler = &hspi1,
    .pu8BufferTx = u8Spi1BufferTx,
    .u16BufferTxSize = SPI1_TX_LEN,
    .pu8BufferRx = u8Spi1BufferRx,
    .u16BufferRxSize = SPI1_RX_LEN,
    .pxEventCb = NULL,
};

/* SPI2 Handler */
SPI_HandleTypeDef hspi2;
DMA_HandleTypeDef hdma_spi2_tx;
uint8_t u8Spi2BufferTx[SPI2_TX_LEN] = { 0U };

SpiHandler_t xSpi1Handler = {
    .ePort = SPI_1,
    .pxHalPeriphHandler = &hspi2,
    .pu8BufferTx = u8Spi2BufferTx,
    .u16BufferTxSize = SPI2_TX_LEN,
    .pu8BufferRx = NULL,
    .u16BufferRxSize = 0U,
    .pxEventCb = NULL,
};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief SPI0 BSP init
  * @param None
  * @retval None
  */
static void BSP_SPI0_Init(void);

/**
  * @brief SPI0 BSP deinit
  * @param None
  * @retval None
  */
static void BSP_SPI0_Deinit(void);

/**
  * @brief SPI1 BSP init
  * @param None
  * @retval None
  */
static void BSP_SPI1_Init(void);

/**
  * @brief SPI1 BSP deinit
  * @param None
  * @retval None
  */
static void BSP_SPI1_Deinit(void);

/* Private user code ---------------------------------------------------------*/

static void BSP_SPI0_Init(void)
{
}

static void BSP_SPI0_Deinit(void)
{
}

static void BSP_SPI1_Init(void)
{
    xSpi1Handler.pxHalPeriphHandler->Instance = SPI2;
    xSpi1Handler.pxHalPeriphHandler->Init.Mode = SPI_MODE_MASTER;
    xSpi1Handler.pxHalPeriphHandler->Init.Direction = SPI_DIRECTION_2LINES;
    xSpi1Handler.pxHalPeriphHandler->Init.DataSize = SPI_DATASIZE_16BIT;
    xSpi1Handler.pxHalPeriphHandler->Init.CLKPolarity = SPI_POLARITY_LOW;
    xSpi1Handler.pxHalPeriphHandler->Init.CLKPhase = SPI_PHASE_1EDGE;
    xSpi1Handler.pxHalPeriphHandler->Init.NSS = SPI_NSS_HARD_OUTPUT;
    xSpi1Handler.pxHalPeriphHandler->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    xSpi1Handler.pxHalPeriphHandler->Init.FirstBit = SPI_FIRSTBIT_MSB;
    xSpi1Handler.pxHalPeriphHandler->Init.TIMode = SPI_TIMODE_DISABLE;
    xSpi1Handler.pxHalPeriphHandler->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    xSpi1Handler.pxHalPeriphHandler->Init.CRCPolynomial = 7;
    xSpi1Handler.pxHalPeriphHandler->Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    xSpi1Handler.pxHalPeriphHandler->Init.NSSPMode = SPI_NSS_PULSE_ENABLE;

    if ( HAL_SPI_Init( xSpi1Handler.pxHalPeriphHandler ) != HAL_OK )
    {
        ERR_ASSERT(0U);
    }
}

static void BSP_SPI1_Deinit(void)
{
    if ( HAL_SPI_DeInit( xSpi1Handler.pxHalPeriphHandler ) != HAL_OK )
    {
        ERR_ASSERT(0U);
    }
}

/* Callback ------------------------------------------------------------------*/

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    SpiHandler_t *pxSpiHandler = NULL;

    if ( hspi->Instance == SPI1 )
    {
        pxSpiHandler = &xSpi0Handler;
    }
    else if ( hspi->Instance == SPI2 )
    {
        pxSpiHandler = &xSpi1Handler;
    }
    else
    {
        /* Not valid value */
    }

    if ( pxSpiHandler != NULL )
    {
        if ( pxSpiHandler->pxEventCb != NULL )
        {
            pxSpiHandler->pxEventCb( SPI_EVENT_TX_DONE );
        }
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    SpiHandler_t *pxSpiHandler = NULL;

    if ( hspi->Instance == SPI1 )
    {
        pxSpiHandler = &xSpi0Handler;
    }
    else if ( hspi->Instance == SPI2 )
    {
        pxSpiHandler = &xSpi1Handler;
    }
    else
    {
        /* Not valid value */
    }

    if ( pxSpiHandler != NULL )
    {
        if ( pxSpiHandler->pxEventCb != NULL )
        {
            pxSpiHandler->pxEventCb( SPI_EVENT_ERROR );
        }
    }
}

/* Public user code ----------------------------------------------------------*/

spi_status_t SPI_init(spi_port_t dev, spi_event_cb event_cb)
{
    spi_status_t retval = SPI_STATUS_NOTDEF;
    
    if (dev == SPI_0)
    {
        BSP_SPI0_Init();

        if ( event_cb != NULL )
        {
            xSpi0Handler.pxEventCb = event_cb;
        }

        retval = SPI_STATUS_OK;
    }
    else if (dev == SPI_1)
    {
        BSP_SPI1_Init();

        if ( event_cb != NULL )
        {
            xSpi1Handler.pxEventCb = event_cb;
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
        BSP_SPI0_Deinit();

        xSpi0Handler.pxEventCb = NULL;

        retval = SPI_STATUS_OK;
    }
    else if (dev == SPI_1)
    {
        BSP_SPI1_Deinit();

        xSpi1Handler.pxEventCb = NULL;

        retval = SPI_STATUS_OK;
    }

    return(retval);
}

spi_status_t SPI_send(spi_port_t dev, uint8_t *pdata, uint16_t len)
{
    spi_status_t retval = SPI_STATUS_NOTDEF;

    if (dev == SPI_0)
    {
        // None
    }
    else if (dev == SPI_1)
    {
        if ( len < xSpi1Handler.u16BufferTxSize )
        {
            while ( HAL_SPI_GetState(xSpi1Handler.pxHalPeriphHandler) != HAL_SPI_STATE_READY );

            for ( uint16_t u16i = 0U; u16i < len; u16i++ )
            {
                xSpi1Handler.pu8BufferTx[u16i] = pdata[u16i];
            }

            retval = ( HAL_SPI_Transmit_DMA(xSpi1Handler.pxHalPeriphHandler, xSpi1Handler.pu8BufferTx, len) == HAL_OK ) ? SPI_STATUS_OK : SPI_STATUS_ERROR;
        }
    }

    return(retval);
}

/* EOF */
