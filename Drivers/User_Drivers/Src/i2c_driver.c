/**
  ******************************************************************************
  * @file           : i2c_driver.c
  * @brief          : Driver to handle i2c peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "i2c_driver.h"
#include "error.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Buffer size */
#define TX_BUFF_LEN_DEV0    (64U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* HAL peripheral handlers */
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;
DMA_HandleTypeDef hdma_i2c1_tx;

/* Event callback handler */
static i2c_event_cb i2c1_event_cb = NULL;

/* Device buffers */
static uint8_t u8BuffTxDev0[TX_BUFF_LEN_DEV0] = { 0U };

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void __I2C1_Init(void);

/**
  * @brief I2C1 Deinitialization Function
  * @param None
  * @retval None
  */
static void __I2C1_Deinit(void);

/* Private user code ---------------------------------------------------------*/

static void __I2C1_Init(void)
{
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x00300B29;     // 1MHz
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** Configure Analogue filter 
     */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** Configure Digital filter 
     */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }

    /** I2C Fast mode Plus enable
    */
    HAL_I2CEx_EnableFastModePlus(I2C_FASTMODEPLUS_I2C1);
}

static void __I2C1_Deinit(void)
{
    if (HAL_I2C_DeInit(&hi2c1) != HAL_OK)
    {
        ERR_ASSERT(0U);
    }
}

/* Callback ------------------------------------------------------------------*/

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(I2C_EVENT_MASTER_TX_DONE);
        }
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(I2C_EVENT_MASTER_RX_DONE);
        }
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(I2C_EVENT_ERROR);
        }
    }
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb(I2C_EVENT_ERROR);
        }
    }
}

/* Public user code ----------------------------------------------------------*/

i2c_status_t I2C_init(i2c_port_t dev, i2c_event_cb event_cb)
{
    i2c_status_t xRetval = I2C_STATUS_NOTDEF;

    if (dev == I2C_0)
    {
        __I2C1_Init();

        if (event_cb != NULL)
        {
            i2c1_event_cb = event_cb;
        }

        xRetval = I2C_STATUS_OK;
    }

    return xRetval;
}

i2c_status_t I2C_deinit(i2c_port_t dev)
{
    i2c_status_t xRetval = I2C_STATUS_NOTDEF;

    if (dev == I2C_0)
    {
        __I2C1_Deinit();

        if (i2c1_event_cb != NULL)
        {
            i2c1_event_cb = NULL;
        }

        xRetval = I2C_STATUS_OK;
    }

    return xRetval;
}

i2c_status_t I2C_master_send(i2c_port_t dev, uint16_t i2c_addr, uint8_t *pdata, uint16_t len)
{
    ERR_ASSERT(pdata != NULL);

    i2c_status_t xRetval = I2C_STATUS_NOTDEF;

    if (dev == I2C_0)
    {
        ERR_ASSERT(len < TX_BUFF_LEN_DEV0);

        if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY)
        {
            uint16_t u16LenData = len;
            uint8_t *pu8Data = u8BuffTxDev0;

            while (u16LenData-- != 0U)
            {
                *pu8Data++ = *pdata++;
            }

            /* Send data */
            if (HAL_I2C_Master_Transmit_DMA(&hi2c1, i2c_addr, u8BuffTxDev0, len) != HAL_OK)
            {
                xRetval = I2C_STATUS_ERROR;
            }
            else
            {
                xRetval = I2C_STATUS_OK;
            }
        }
        else
        {
            xRetval = I2C_STATUS_BUSY;
        }
    }

    return xRetval;
}

i2c_status_t I2C_master_read(i2c_port_t dev, uint16_t i2c_addr, uint8_t *pdata, uint16_t len)
{
    ERR_ASSERT(pdata != NULL);

    i2c_status_t xRetval = I2C_STATUS_NOTDEF;

    if (dev == I2C_0)
    {
        if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY)
        {
            /* Set read data */
            if (HAL_I2C_Master_Receive_DMA(&hi2c1, i2c_addr, pdata, len) != HAL_OK)
            {
                xRetval = I2C_STATUS_ERROR;
            }
            else
            {
                xRetval = I2C_STATUS_OK;
            }
        }
        else
        {
            xRetval = I2C_STATUS_BUSY;
        }
    }

    return xRetval;
}

/*****END OF FILE****/
