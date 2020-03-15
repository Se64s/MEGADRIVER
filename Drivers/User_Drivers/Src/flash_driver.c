/**
 * @file 	flash_driver.h
 * @author  Sebastian Del Moral Gallardo
 * @brief	Basic funtionality for use flash write/read.
 *
 */

/* Includes -----------------------------------------------------------------*/

#include "flash_driver.h"

/* Private defines ----------------------------------------------------------*/
/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/* Private functions definitions --------------------------------------------*/
/* Private functions declaration --------------------------------------------*/
/* HAL callbacks ------------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

flash_status_t xFLASH_init(void)
{
    flash_status_t xRetVal = FLASH_DRIVER_ERROR;

    /* TODO: Init actions here */
    xRetVal = FLASH_DRIVER_OK;

    return xRetVal;
}

flash_status_t xFLASH_deini(void)
{
    flash_status_t xRetVal = FLASH_DRIVER_ERROR;

    /* TODO: Deinit actions here */
    xRetVal = FLASH_DRIVER_OK;

    return xRetVal;
}

flash_status_t xFLASH_ErasePage(uint32_t u32PageNumber)
{
    flash_status_t xRetVal = FLASH_DRIVER_ERROR;

    if (u32PageNumber < FLASH_PAGE_NB)
    {
        uint32_t u32PageError = 0;

        HAL_FLASH_Unlock();

        /* Fill EraseInit structure*/
        FLASH_EraseInitTypeDef EraseInitStruct = {0};
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.Page = u32PageNumber;
        EraseInitStruct.NbPages = 1;

        if (HAL_FLASHEx_Erase(&EraseInitStruct, &u32PageError) == HAL_OK)
        {
            xRetVal = FLASH_DRIVER_OK;
        }

        HAL_FLASH_Lock();
    }

    return xRetVal;
}

flash_status_t xFLASH_WriteDoubleWord(uint32_t u32Addr, uint64_t u64Data)
{
    flash_status_t xRetVal = FLASH_DRIVER_ERROR;

    HAL_FLASH_Unlock();

    if (HAL_OK == HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, u32Addr, u64Data))
    {
        xRetVal = FLASH_DRIVER_OK;
    }

    HAL_FLASH_Lock();

    return xRetVal;
}

/* EOF */
