/**
 * @file sys_ll_serial.h
 * @author Sebastián Del Moral (sebmorgal@gmail.com)
 * @brief System support packet to handle serial interfaces using low level features.
 * @version 0.1
 * @date 2020-09-27
 * 
 * @copyright Copyright (c) 2020
 * 
 */

/* Define to prevent recursive inclusion ------------------------------------*/
#ifndef __SYS_LL_SERIAL_H
#define __SYS_LL_SERIAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported includes --------------------------------------------------------*/
#include <stdint.h>

/* Exported defines ---------------------------------------------------------*/
/* Exported types -----------------------------------------------------------*/

/** List of serial devices*/
typedef enum
{
    SYS_LL_SERIAL_0 = 0U,
    SYS_LL_SERIAL_NODEF = 0xFFU,
} sys_ll_serial_port_t;

/** Operation status */
typedef enum
{
    SYS_LL_SERIAL_STATUS_ERROR =   0U,
    SYS_LL_SERIAL_STATUS_OK =      1U,
    SYS_LL_SERIAL_STATUS_NODEF =   0xFFU,
} sys_ll_serial_status_t;

/* Exported macro -----------------------------------------------------------*/
/* Exported functions prototypes --------------------------------------------*/

/**
 * @brief Init low level uart
 * 
 * @param dev defined serial interface
 * @return sys_ll_serial_status_t 
 */
sys_ll_serial_status_t SYS_LL_UartInit(sys_ll_serial_port_t dev);

/**
 * @brief Blocking send action over defined serial interface
 * 
 * @param dev defined serial interface
 * @param buf buffer data to send
 * @param len number of bytes to send
 * @return sys_ll_serial_status_t 
 */
sys_ll_serial_status_t SYS_LL_UartSend(sys_ll_serial_port_t dev, uint8_t* buf, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_LL_SERIAL_H */

/*EOF*/