/**
 * @file cli_cmd.c
 * @author Sebastián Del Moral Gallardo.
 * @brief CLI commands implemented in system.
 *
 */

/* Includes ------------------------------------------------------------------*/

#include "cli_cmd.h"
#include "printf.h"
#include "FreeRTOS.h"
#include "FreeRTOS_CLI.h"

#include <stdlib.h>
#include "stm32g0xx_hal.h"
#include "YM2612_driver.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Force reset device.
 * @param  pcWriteBuffer
 * @param  xWriteBufferLen
 * @param  pcCommandString
 * @retval pdFALSE, pdTRUE
 */
static BaseType_t devReset(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/**
 * @brief  Send write command to YM2612.
 * @param  pcWriteBuffer
 * @param  xWriteBufferLen
 * @param  pcCommandString
 * @retval pdFALSE, pdTRUE
 */
static BaseType_t YM2612Write(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/* Private variables ---------------------------------------------------------*/

static const CLI_Command_Definition_t xDevReset = {
    "reset",
    "reset: Force device reset",
    devReset,
    0
};

static const CLI_Command_Definition_t xWriteReg = {
    "writeReg",
    "writeReg: Write register into YM2612, use: writeReg <addr 0-255> <data 0-255> <bank 0-1>",
    YM2612Write,
    3
};

/* Callbacks -----------------------------------------------------------------*/
/* Private application code --------------------------------------------------*/

static BaseType_t devReset(char *pcWriteBuffer,
        size_t xWriteBufferLen,
        const char *pcCommandString)
{
    vCliPrintf("CLI", "OK");
    (void)HAL_NVIC_SystemReset();
    return pdFALSE;
}

static BaseType_t YM2612Write(char *pcWriteBuffer,
        size_t xWriteBufferLen,
        const char *pcCommandString)
{
    uint8_t u8regAddr;
    uint8_t u8regData;
    uint8_t u8bankSel;
    char *pcParameter1;
    char *pcParameter2;
    char *pcParameter3;
    BaseType_t xParameter1StringLength;
    BaseType_t xParameter2StringLength;
    BaseType_t xParameter3StringLength;

    /* Get cmd parameters */
    pcParameter1 = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 1, &xParameter1StringLength);
    pcParameter2 = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 2, &xParameter2StringLength);
    pcParameter3 = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 3, &xParameter3StringLength);
    pcParameter1[xParameter1StringLength] = 0x00;
    pcParameter2[xParameter2StringLength] = 0x00;
    pcParameter3[xParameter3StringLength] = 0x00;
    u8regAddr = (uint8_t)atoi(pcParameter1);
    u8regData = (uint8_t)atoi(pcParameter2);
    u8bankSel = (uint8_t)atoi(pcParameter3);

    /* Check parameters */
    if ((u8bankSel == YM2612_BANK_0) || u8bankSel == YM2612_BANK_1)
    {
        YM2612_bank_t xbankValue = (u8bankSel == YM2612_BANK_0)?YM2612_BANK_0:YM2612_BANK_1;
        
        vYM2612_write_reg(u8regAddr, u8regData, xbankValue);
        
        (void)snprintf(pcWriteBuffer, xWriteBufferLen, "OK");
    }
    else
    {
        (void)snprintf(pcWriteBuffer, xWriteBufferLen, "ERROR: Bank value not valid");
    }
    return pdFALSE;
}

/* Public application code ---------------------------------------------------*/

void cli_cmd_init(void)
{
    (void)FreeRTOS_CLIRegisterCommand(&xDevReset);
    (void)FreeRTOS_CLIRegisterCommand(&xWriteReg);
}

/* EOF */
