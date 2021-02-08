/**
  ******************************************************************************
  * @file           : display_driver.c
  * @brief          : Driver to handle display peripheral
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "display_driver.h"
#include "i2c_driver.h"
#include "error.h"
#include "printf.h"
#include "main.h"

#ifdef DISPLAY_USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Display 0 address */
#define DISPLAY_ADDRESS     0x78U

/* Number of ticks per microsec */
#define DISPLAY_TICKS_USEC  15U

/* Maximun len display string */
#define DISPLAY_MAX_LEN     16U

/* Display initial msg  */
#define DISPLAY_INIT_0      "MEGADRIVER"
#define DISPLAY_INIT_1      "Build v"

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief Require hardware init
  * @param None
  * @retval None
  */
static void __HardwareInit(void);

/**
  * @brief Require hardware de-init
  * @param None
  * @retval None
  */
static void __HardwareDeInit(void);

/**
  * @brief Delay implementation us
  * @param u32UsCount number of us to wait
  * @retval None
  */
static void __UsDelay(uint32_t u32UsCount);

/* Callbacks required by u8g2 lib */
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

/* Private user code ---------------------------------------------------------*/

static void __HardwareInit(void)
{
    if (I2C_init(I2C_0, NULL) != I2C_STATUS_OK)
    {
        ERR_ASSERT(0U);
    }
}

static void __HardwareDeInit(void)
{
    if (I2C_deinit(I2C_0) != I2C_STATUS_OK)
    {
        ERR_ASSERT(0U);
    }
}

static void __UsDelay(uint32_t u32UsCount)
{
#ifdef DISPLAY_USE_RTOS
    // Use RTOS interface for implement delays
    vTaskDelay(10U);
#else
    uint32_t tick_count = DISPLAY_TICKS_USEC * u32UsCount;
    while (tick_count-- != 0)
    {
        __NOP();
    }
#endif
}

/* Public user code ----------------------------------------------------------*/

display_status_t DISPLAY_init(display_port_t dev, u8g2_t * pxDisplayHandler)
{
    display_status_t xRetval = DISPLAY_STATUS_NOTDEF;

    if (dev == DISPLAY_0)
    {
        __HardwareInit();

        // Set display I2C addr
        u8g2_SetI2CAddress(pxDisplayHandler, DISPLAY_ADDRESS);

        // Init grapfic library
        u8g2_Setup_ssd1306_i2c_128x64_noname_2(pxDisplayHandler, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);

        // Init sequence to display
        u8g2_InitDisplay(pxDisplayHandler);
        u8g2_SetPowerSave(pxDisplayHandler, 0U);

        xRetval = DISPLAY_STATUS_OK;
    }

    return xRetval;
}

display_status_t DISPLAY_deinit(display_port_t dev)
{
    display_status_t xRetval = DISPLAY_STATUS_NOTDEF;

    if (dev == DISPLAY_0)
    {
        __HardwareDeInit();

        xRetval = DISPLAY_STATUS_OK;
    }

    return xRetval;
}

display_status_t DISPLAY_update(display_port_t dev, u8g2_t * pxDisplayHandler)
{
    display_status_t xRetval = DISPLAY_STATUS_NOTDEF;

    if (dev == DISPLAY_0)
    {
        char pcInitMsg0[DISPLAY_MAX_LEN] = {0};
        char pcInitMsg1[DISPLAY_MAX_LEN] = {0};
        char pcInitMsg2[DISPLAY_MAX_LEN] = {0};

        (void)snprintf(pcInitMsg0, DISPLAY_MAX_LEN, "%s", DISPLAY_INIT_0);
        (void)snprintf(pcInitMsg1, DISPLAY_MAX_LEN, "Build   %s", MAIN_APP_VERSION);
        (void)snprintf(pcInitMsg2, DISPLAY_MAX_LEN, "Rev     %x", GIT_REVISION);

        // Update display data
        u8g2_FirstPage(pxDisplayHandler);
        do {
            u8g2_SetFont(pxDisplayHandler, u8g2_font_amstrad_cpc_extended_8r);
            u8g2_DrawStr(pxDisplayHandler, 0, 10, pcInitMsg0);
            u8g2_DrawStr(pxDisplayHandler, 0, 25, pcInitMsg1);
            u8g2_DrawStr(pxDisplayHandler, 0, 40, pcInitMsg2);
        } while (u8g2_NextPage(pxDisplayHandler));

        xRetval = DISPLAY_STATUS_OK;
    }

    return xRetval;
}

/* Callback ------------------------------------------------------------------*/

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
    static uint8_t u8Buffer[32U];
    static uint8_t u8BufIdx = 0U;
    uint8_t *pu8Data;
    uint8_t u8RetVal = 1U;

    switch (msg)
    {
        case U8X8_MSG_BYTE_SEND:
            pu8Data = (uint8_t *)arg_ptr;
            while(arg_int > 0)
            {
                u8Buffer[u8BufIdx++] = *pu8Data;
                pu8Data++;
                arg_int--;
            }
            break;

        case U8X8_MSG_BYTE_INIT:
            break;

        case U8X8_MSG_BYTE_SET_DC:
            /* ignored for i2c */
            break;

        case U8X8_MSG_BYTE_START_TRANSFER:
            u8BufIdx = 0;
            __UsDelay(350U);
            break;

        case U8X8_MSG_BYTE_END_TRANSFER:
            {
                uint16_t u16DevAddr = u8g2_GetI2CAddress(u8x8);
                (void)I2C_master_send(I2C_0, u16DevAddr, u8Buffer, u8BufIdx);
            }
            break;

        default:
            u8RetVal = 0U;
            break;
    }

    return u8RetVal;
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    uint8_t u8RetVal = 1U;

    switch(msg)
    {
        // Initialize I2C peripheral
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            /* HAL initialization contains all what we need so we can skip this part. */
            __UsDelay(1000U);
            break;

        // Function which implements a delay, arg_int contains the amount of ms
        case U8X8_MSG_DELAY_MILLI:
            {
                uint32_t u32CastMsToUs = 1000U * (uint32_t)arg_int;
                __UsDelay(u32CastMsToUs);
            }
            break;

        // Function which delays 10us
        case U8X8_MSG_DELAY_10MICRO:
            __UsDelay(10U);
            break;

        // Function which delays 100ns
        case U8X8_MSG_DELAY_100NANO:
            __NOP();
            break;

        case U8X8_MSG_DELAY_I2C:
            break;

        default:
            u8RetVal = 0U;
            break;
    }

    return u8RetVal;
}

/*****END OF FILE****/
