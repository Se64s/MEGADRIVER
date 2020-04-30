/**
 * @file    synth_app_data.c
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Basic library to handle synth app data layout.
 *
 */

/* Includes -----------------------------------------------------------------*/

#include "app_data_handler.h"
#ifdef APP_DATA_USE_RTOS
#include "FreeRTOS.h"
#include "semphr.h"
#endif

/* Private defines ----------------------------------------------------------*/

/** Address for app data */
#define APP_DATA_BASE_ADDR          ((uint32_t)(FLASH_BASE + (FLASH_PAGE_NB - APP_DATA_NUM_PAGES) * FLASH_PAGE_SIZE))

/* Private macro  -----------------------------------------------------------*/

/** Get address for a defined page */
#define GET_PAGE_ADDR(PAGE_NUMBER)  ((uint32_t)(FLASH_BASE + ((PAGE_NUMBER) * FLASH_PAGE_SIZE)))

/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

#ifdef APP_DATA_USE_RTOS
volatile SemaphoreHandle_t xAppDataMutex = NULL;
#endif

/* Private functions definitions --------------------------------------------*/

/**
  * @brief Check if element has been init.
  * @param u32ElementAddr Adress of element to check.
  * @param u32Size Number of bytes to check.
  * @retval true if element init, false if element not init.
  */
static bool bCheckEmptyElement(uint32_t u32ElementAddr, uint32_t u32Size);

/**
  * @brief Check if layout has been initiated.
  * @param pxLayout Layout parameters to check.
  * @retval true layout already init, false layout not init.
  */
static bool bCheckLayout(app_data_layout_t * pxLayout);

/**
  * @brief Get last position with valid data.
  * @param pxLayout Control structure with layout parameters.
  * @retval NULL is no position found, EOC if a position with data found.
  */
static const void * pvGetElement(app_data_layout_t * pxLayout);

/**
  * @brief Save element.
  * @param pxLayout Control structure with layout parameters.
  * @param pvElement Pointer to element to save.
  * @retval true element saved, false element not saved.
  */
bool bSaveElement(app_data_layout_t * pxLayout, void * pvElement);

/**
  * @brief Save element on flash.
  * @param u32FlashAddr Flash address.
  * @param u32ElementSize Element size.
  * @param pvElement Pointer to element to write.
  * @retval true element saved, false element not saved.
  */
bool bWriteElementFlash(uint32_t u32FlashAddr, void * pvElement, uint32_t u32ElementSize);

/**
  * @brief Mark element with KNOWN pattern.
  * @param u32FlashAddr Flash address.
  * @param u32ElementSize Element size.
  * @retval true element saved, false element not saved.
  */
bool bClearElementFlash(uint32_t u32FlashAddr, uint32_t u32ElementSize);

/* Private functions declaration --------------------------------------------*/

static bool bCheckEmptyElement(uint32_t u32ElementAddr, uint32_t u32Size)
{
    bool bRetVal = false;

    if (u32ElementAddr != 0U)
    {
        const uint8_t * pu8Element = (uint8_t *)u32ElementAddr;
        uint32_t u32InitBytesCount = 0;

        for (uint32_t u32ElementIndex = 0U; u32ElementIndex < u32Size; u32ElementIndex++)
        {
            if (pu8Element[u32ElementIndex] == APP_DATA_8_NO_INIT)
            {
                u32InitBytesCount++;
            }
            else
            {
                break;
            }
        }

        if (u32InitBytesCount == u32Size)
        {
            bRetVal = true;
        }
    }

    return bRetVal;
}

static bool bCheckLayout(app_data_layout_t * pxLayout)
{
    bool bRetVal = false;

    if (pxLayout != NULL)
    {
        bool bEndOfSearch = false;
        uint32_t u32TmpPage = 0U;
        uint32_t u32TmpElement = 0U;
        uint32_t u32NumPageElements = FLASH_PAGE_SIZE / pxLayout->u32ElementSize;

        /* Loop over all defined pages */
        for (uint32_t u32PageIndex = 0U; u32PageIndex < pxLayout->u32NumPages; u32PageIndex++)
        {
            /* Loop over all page elements */
            for (uint32_t u32ElementIndex = 0U; u32ElementIndex < u32NumPageElements; u32ElementIndex++)
            {
                uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage + u32PageIndex);
                u32ElementAddr += u32ElementIndex * pxLayout->u32ElementSize;

                /* Check if element has been initited */
                if (bCheckEmptyElement(u32ElementAddr, pxLayout->u32ElementSize) == true)
                {
                    bEndOfSearch = true;
                    break;
                }
                else
                {
                    u32TmpElement = u32ElementIndex;
                    u32TmpPage = u32PageIndex;
                    bRetVal = true;
                }
            }

            /* Last element already found */
            if (bEndOfSearch)
            {
                break;
            }
        }

        /* Init layout with read param */
        pxLayout->u32PageIndex = u32TmpPage;
        pxLayout->u32ElementIndex = u32TmpElement;
    }

    return bRetVal;
}

static const void * pvGetElement(app_data_layout_t * pxLayout)
{
    const void * pvElement = NULL;

    if (pxLayout != NULL)
    {
        uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage + pxLayout->u32PageIndex);
        u32ElementAddr += pxLayout->u32ElementIndex * pxLayout->u32ElementSize;
        pvElement = (void *)u32ElementAddr;
    }

    return pvElement;
}

bool bWriteElementFlash(uint32_t u32FlashAddr, void * pvElement, uint32_t u32ElementSize)
{
    bool bRetVal = false;

    if (pvElement != NULL)
    {
        /* Element to save must be 8B alligned */
        if (u32ElementSize % 8U == 0U)
        {
            bRetVal = true;
            uint32_t u32NumOperations = u32ElementSize / 8U;
            uint64_t * pu64Data = (uint64_t *)pvElement;

            /* Write loop */
            for (uint32_t u32WriteIndex = 0U; u32WriteIndex < u32NumOperations; u32WriteIndex++)
            {
                if (xFLASH_WriteDoubleWord(u32FlashAddr, *pu64Data++) == FLASH_DRIVER_OK)
                {
                    u32FlashAddr += sizeof(uint64_t);
                }
                else
                {
                    bRetVal = false;
                    break;
                }
            }
        }
    }

    return bRetVal;
}

bool bClearElementFlash(uint32_t u32FlashAddr, uint32_t u32ElementSize)
{
    bool bRetVal = false;

    /* Element to save must be 8B alligned */
    if (u32ElementSize % 8U == 0U)
    {
        bRetVal = true;
        uint32_t u32NumOperations = u32ElementSize / 8U;
        uint64_t pu64Data = 0xABABABABABABABAB;

        /* Write loop */
        for (uint32_t u32WriteIndex = 0U; u32WriteIndex < u32NumOperations; u32WriteIndex++)
        {
            if (xFLASH_WriteDoubleWord(u32FlashAddr, pu64Data) == FLASH_DRIVER_OK)
            {
                u32FlashAddr += sizeof(uint64_t);
            }
            else
            {
                bRetVal = false;
                break;
            }
        }
    }

    return bRetVal;
}

bool bSaveElement(app_data_layout_t * pxLayout, void * pvElement)
{
    bool bRetVal = false;

    if (pxLayout != NULL)
    {
        uint32_t u32NumPageElements = FLASH_PAGE_SIZE / pxLayout->u32ElementSize;

        /* Element fits on current page */
        if (pxLayout->u32ElementIndex < (u32NumPageElements - 1U))
        {
            uint32_t u32NextElement = pxLayout->u32ElementIndex + 1U;

            uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage + pxLayout->u32PageIndex);
            u32ElementAddr += u32NextElement * pxLayout->u32ElementSize;

            if (bWriteElementFlash(u32ElementAddr, pvElement, pxLayout->u32ElementSize) == true)
            {
                pxLayout->u32ElementIndex = u32NextElement;
                bRetVal = true;
            }
        }
        /* Element fits on next page */
        else if (pxLayout->u32PageIndex < (pxLayout->u32NumPages - 1U))
        {
            uint32_t u32NextPage = pxLayout->u32PageIndex + 1U;

            if (xFLASH_ErasePage(u32NextPage + pxLayout->u32InitPage) == FLASH_DRIVER_OK)
            {
                uint32_t u32NextElement = 0U;
                uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage + pxLayout->u32PageIndex);
                u32ElementAddr += u32NextElement * pxLayout->u32ElementSize;

                if (bWriteElementFlash(u32ElementAddr, pvElement, pxLayout->u32ElementSize) == true)
                {
                    pxLayout->u32PageIndex = u32NextPage;
                    pxLayout->u32ElementIndex = u32NextElement;
                    bRetVal = true;
                }
            }
        }
        /* Page rollout, start from the begining */
        else
        {
            uint32_t u32NextPage = 0U;

            if (xFLASH_ErasePage(u32NextPage + pxLayout->u32InitPage) == FLASH_DRIVER_OK)
            {
                uint32_t u32NextElement = 0U;
                uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage + pxLayout->u32PageIndex);
                u32ElementAddr += u32NextElement * pxLayout->u32ElementSize;

                if (bWriteElementFlash(u32ElementAddr, pvElement, pxLayout->u32ElementSize) == true)
                {
                    pxLayout->u32PageIndex = u32NextPage;
                    pxLayout->u32ElementIndex = u32NextElement;
                    bRetVal = true;
                }
            }
        }
    }

    return bRetVal;
}

/* Public functions declaration --------------------------------------------*/

app_data_status_t xAPP_DATA_init_layout(app_data_layout_t * pxLayout)
{
    app_data_status_t xRetVal = APP_DATA_ERROR;

    /* Init resources */
#ifdef APP_DATA_USE_RTOS
    if (xAppDataMutex == NULL)
    {
        xAppDataMutex = xSemaphoreCreateMutex();
    }
#endif

    if (pxLayout != NULL)
    {
#ifdef APP_DATA_USE_RTOS
    if (xAppDataMutex != NULL)
    {
        if (xSemaphoreTake(xAppDataMutex, portMAX_DELAY) != pdTRUE)
        {
            while (1U);
        }
#endif
        if (bCheckLayout(pxLayout) != true)
        {
            uint32_t u32ElementAddr = GET_PAGE_ADDR(pxLayout->u32InitPage);
            if (xFLASH_ErasePage(pxLayout->u32InitPage) == FLASH_DRIVER_OK)
            {
                if (bClearElementFlash(u32ElementAddr, pxLayout->u32ElementSize))
                {
                    xRetVal = APP_DATA_NOT_INIT;
                }
            }
        }
        else
        {
            xRetVal = APP_DATA_OK;
        }
#ifdef APP_DATA_USE_RTOS
        if (xSemaphoreGive(xAppDataMutex) != pdTRUE)
        {
            while (1U);
        }
    }
#endif
    }

    return xRetVal;
}

app_data_status_t xAPP_DATA_clear_layout(app_data_layout_t * pxLayout)
{
    app_data_status_t xRetVal = APP_DATA_ERROR;

#ifdef APP_DATA_USE_RTOS
    if (xAppDataMutex != NULL)
    {
        if (xSemaphoreTake(xAppDataMutex, portMAX_DELAY) != pdTRUE)
        {
            while (1U);
        }
#endif

    if (pxLayout != NULL)
    {
        xRetVal = APP_DATA_OK;

        for (uint32_t u32PageIndex = 0U; u32PageIndex < pxLayout->u32NumPages; u32PageIndex++)
        {
            if (FLASH_DRIVER_OK != xFLASH_ErasePage(pxLayout->u32InitPage + u32PageIndex))
            {
                xRetVal = APP_DATA_ERROR;
                break;
            }
        }
    }

#ifdef APP_DATA_USE_RTOS
        if (xSemaphoreGive(xAppDataMutex) != pdTRUE)
        {
            while (1U);
        }
    }
#endif

    return xRetVal;
}

app_data_status_t xAPP_DATA_save_element(app_data_layout_t * pxLayout, void * pvData)
{
    app_data_status_t xRetVal = APP_DATA_ERROR;

#ifdef APP_DATA_USE_RTOS
    if (xAppDataMutex != NULL)
    {
        if (xSemaphoreTake(xAppDataMutex, portMAX_DELAY) != pdTRUE)
        {
            while (1U);
        }
#endif

    if (bSaveElement(pxLayout, pvData) == true)
    {
        xRetVal = APP_DATA_OK;
    }

#ifdef APP_DATA_USE_RTOS
        if (xSemaphoreGive(xAppDataMutex) != pdTRUE)
        {
            while (1U);
        }
    }
#endif

    return xRetVal;
}

const void * pvAPP_DATA_get_element(app_data_layout_t * pxLayout)
{
    const void * pvReturnData = NULL;

#ifdef APP_DATA_USE_RTOS
    if (xAppDataMutex != NULL)
    {
        if (xSemaphoreTake(xAppDataMutex, portMAX_DELAY) != pdTRUE)
        {
            while (1U);
        }
#endif

    if (pxLayout != NULL)
    {
        pvReturnData = pvGetElement(pxLayout);
    }

#ifdef APP_DATA_USE_RTOS
        if (xSemaphoreGive(xAppDataMutex) != pdTRUE)
        {
            while (1U);
        }
    }
#endif

    return pvReturnData;
}

/* EOF */
