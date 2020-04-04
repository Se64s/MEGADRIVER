/**
 * @file    synth_app_data.c
 * @author  Sebastian Del Moral Gallardo.
 * @brief   Basic library to handle synth app data layout.
 *
 */

/* Includes -----------------------------------------------------------------*/

#include "synth_app_data.h"

/* Private defines ----------------------------------------------------------*/
/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

/* Control structure */
app_data_layout_t pxSynthAppData[SYNTH_APP_DATA_NUM_PRESETS] = {0U};

/* Private functions definitions --------------------------------------------*/
/* Private functions declaration --------------------------------------------*/
/* HAL callbacks ------------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

bool bSYNTH_APP_DATA_init(void)
{
    bool bRetVal = false;

    for (uint32_t u32CfgIndex = 0U; u32CfgIndex < SYNTH_APP_DATA_NUM_PRESETS; u32CfgIndex++)
    {
        pxSynthAppData[u32CfgIndex].u32InitPage = SYNTH_APP_DATA_PRESET_INIT_PAGE + u32CfgIndex;
        pxSynthAppData[u32CfgIndex].u32NumPages = SYNTH_APP_PAGE_PER_PRESET;
        pxSynthAppData[u32CfgIndex].u32ElementSize = sizeof(synth_app_data_t);

        app_data_status_t xInitStatus = xAPP_DATA_init_layout(&pxSynthAppData[u32CfgIndex]);

        if (xInitStatus == APP_DATA_NOT_INIT)
        {
            /* Init actions */
            break;
        }
        else if (xInitStatus == APP_DATA_OK)
        {
            bRetVal = true;
        }
    }

    return bRetVal;
}

bool bSYNTH_APP_DATA_write(uint8_t u8PresetId, synth_app_data_t * pxPresetData)
{
    bool bRetVal = false;

    if ((u8PresetId < SYNTH_APP_DATA_NUM_PRESETS) && (pxPresetData != NULL))
    {
        if (xAPP_DATA_save_element(&pxSynthAppData[u8PresetId], (void *)pxPresetData) == APP_DATA_OK)
        {
            bRetVal = true;
        }
    }

    return bRetVal;
}

bool bSYNTH_APP_DATA_read(uint8_t u8PresetId, const synth_app_data_t ** pxPresetData)
{
    bool bRetVal = false;

    if ((u8PresetId < SYNTH_APP_DATA_NUM_PRESETS) && (pxPresetData != NULL))
    {
        *pxPresetData = pvAPP_DATA_get_element(&pxSynthAppData[u8PresetId]);

        if (*pxPresetData != NULL)
        {
            /* Check if valid data */
            uint32_t u32NotValidDataCount = 0U;
            uint8_t * pu8Data = (uint8_t *)*pxPresetData;
            for (uint32_t u32DataIndex = 0; u32DataIndex < pxSynthAppData[u8PresetId].u32ElementSize; u32DataIndex++)
            {
                uint8_t u8Data = pu8Data[u32DataIndex];
                if (u8Data == APP_DATA_8_NO_INIT || u8Data == 0xAB)
                {
                    u32NotValidDataCount++;
                }
                else
                {
                    break;
                }
            }

            if (u32NotValidDataCount != pxSynthAppData[u8PresetId].u32ElementSize)
            {
                bRetVal = true;
            }
        }
    }

    return bRetVal;
}

/* EOF */
