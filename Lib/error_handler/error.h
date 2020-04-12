/**
 * @file    error.h
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Toolset of functions and macro to conrol errors.
 *
 */

#ifndef __ERROR_H
#define __ERROR_H

#ifdef  __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

/* Exported defines ----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/** Control structure with fields to print in assert */
typedef struct sAssertInfo
{
    uint32_t pc;
    uint32_t lr;
    uint32_t line;
} sAssertInfo_t;

/** Function definition to print out assert data */
typedef void (* err_print_data)(const char *Format, ...);

/* Exported macro ------------------------------------------------------------*/

/** Macro to capture LR reg value */
#define ERR_GET_LR()        __builtin_return_address(0)

/** Macro to capture PC reg value */
#define ERR_GET_PC(_a)      __asm volatile ("mov %0, pc" : "=r" (_a))

/** Capture assert data */
#define ERR_ASSERT_RECORD()         \
do{                                 \
    void *pc;                       \
    ERR_GET_PC(pc);                 \
    const void *lr = ERR_GET_LR();  \
    vErrorAssert(pc, lr, __LINE__); \
} while (0)

/** Execute assert */
#define ERR_ASSERT(exp)     \
do {                        \
    if (!(exp)) {           \
      ERR_ASSERT_RECORD();  \
    }                       \
} while (0)

/* Exported variables --------------------------------------------------------*/

extern sAssertInfo_t xGlobalAssert;

/* Exported functions --------------------------------------------------------*/

/**
  * @brief Setup output funtion used in assert.
  * @param vPrintError pointer to function used to print error.
  * @retval None.
  */
void vErrorInit(err_print_data vPrintError);

/**
  * @brief Funtion to output assert information.
  * @param pc program counter register value.
  * @param lr link register value.
  * @param line line of error.
  * @retval None.
  */
void vErrorAssert(const void *pc, const void *lr, uint32_t line);

#ifdef  __cplusplus
}
#endif

#endif /* __ERROR_H */
