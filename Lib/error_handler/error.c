/**
 * @file    error.c
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Toolset of functions and macro to conrol errors.
 *
 */

/* Includes -----------------------------------------------------------------*/

#include "error.h"

/* Private defines ----------------------------------------------------------*/
/* Private macro  -----------------------------------------------------------*/

/** Error message format */
#define ERR_MSG     "\r\n\r\n ----- ASSERT -----\r\n\r\n PC:\t0x%X\r\n LK:\t0x%X\r\n LINE:\t%d\r\n\r\n"

/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

/** Error print function defined by user */
static err_print_data vUserPrint = NULL;

/** Structure to hold assert information */
sAssertInfo_t xGlobalAssert = {0};

/* Private functions definitions --------------------------------------------*/
/* Private functions declaration --------------------------------------------*/
/* Public functions declaration ---------------------------------------------*/

void vErrorInit(err_print_data vPrintError)
{
    if (vPrintError != NULL)
    {
        vUserPrint = vPrintError;
    }
}

void vErrorAssert(const void *pc, const void *lr, uint32_t line)
{
    xGlobalAssert.pc = (uint32_t)pc;
    xGlobalAssert.lr = (uint32_t)lr;
    xGlobalAssert.line = line;

    /* Print data if handler initiated */
    if (vUserPrint != NULL)
    {
        vUserPrint(ERR_MSG, xGlobalAssert.pc, xGlobalAssert.lr, xGlobalAssert.line);
    }

    /* Stop app */
    __asm("bkpt 0");
}

/* EOF */
