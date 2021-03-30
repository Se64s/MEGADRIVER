/**
 * @file    user_error.c
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Toolset of functions and macro to control errors.
 *
 */

/* Includes -----------------------------------------------------------------*/
#include "user_error.h"

/* Private defines ----------------------------------------------------------*/

/** Disable IRQ */
#define DISABLE_IRQ()   __asm volatile ( " cpsid i " ::: "memory" )

/** Index address PC */
#define MSG_INDEX_PC    30U

/** Index address LK */
#define MSG_INDEX_LK    46U

/* Private macro  -----------------------------------------------------------*/
/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

/** Error print function defined by user */
static err_print_data vUserPrint = NULL;

/** Error message array */
static char pcErrorMsg[] = "\r\n\r\n----- ASSERT -----\r\nPC: 0x00000000\r\nLK: 0x00000000\r\n------------------\r\n\r\n";

/** Structure to hold assert information */
sAssertInfo_t xGlobalAssert = {0};

/* Private functions declaration --------------------------------------------*/
/* Private functions definitions --------------------------------------------*/

static const char dumpHexDigit(uint8_t nibble)
{
    static const char hexToASCII[] = "0123456789ABCDEF";
    char cRetVal = 0;

    cRetVal = hexToASCII[nibble];

    return cRetVal;
}

static void dumpByteAsHex(char byte, char *dst)
{
    *dst++ = dumpHexDigit(byte >> 4);
    *dst = dumpHexDigit(byte & 0xF);
}

/* Public functions declaration ---------------------------------------------*/

void vErrorInit(err_print_data vPrintError)
{
    if (vPrintError != NULL)
    {
        vUserPrint = vPrintError;
    }
}

void vErrorAssert(const void *pc, const void *lr)
{
    DISABLE_IRQ();

    xGlobalAssert.pc = (uint32_t)pc;
    xGlobalAssert.lr = (uint32_t)lr;

    for (uint32_t i = 0; i < sizeof(uint32_t); i++)
    {
        dumpByteAsHex(xGlobalAssert.pc >> ((3U - i) * 8U), &pcErrorMsg[MSG_INDEX_PC + (2U * i)]);
        dumpByteAsHex(xGlobalAssert.lr >> ((3U - i) * 8U), &pcErrorMsg[MSG_INDEX_LK + (2U * i)]);
    }

    /* Print data if handler initiated */
    if (vUserPrint != NULL)
    {
        vUserPrint(pcErrorMsg, sizeof(pcErrorMsg));
    }

    /* Stop app */
    // __asm("bkpt 0");
    while (1);
}

/* EOF */
