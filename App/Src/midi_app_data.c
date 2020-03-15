/**
 * @file midi_app_data.c
 * @author Sebastian Del Moral Gallardo.
 * @brief Basic library to handle flash constants.
 *
 */

/* Includes -----------------------------------------------------------------*/

#include "midi_app_data.h"

/* Private defines ----------------------------------------------------------*/
/* Private constants  -------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

/* Control structure */
app_data_layout_t xMidiAppData = {0};

/* Private functions definitions --------------------------------------------*/
/* Private functions declaration --------------------------------------------*/
/* HAL callbacks ------------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/

bool bMIDI_APP_DATA_init(void)
{
    bool bRetVal = false;
    
    /* Init control data */
    xMidiAppData.u32InitPage = MIDI_APP_DATA_INIT_PAGE;
    xMidiAppData.u32NumPages = MIDI_APP_DATA_NUM_PAGES;
    xMidiAppData.u32ElementSize = sizeof(midi_app_data_t);

    app_data_status_t xInitStatus = xAPP_DATA_init_layout(&xMidiAppData);

    if (xInitStatus == APP_DATA_NOT_INIT)
    {
        /* Clear page and write a default CFG */
        if (xAPP_DATA_clear_layout(&xMidiAppData) == APP_DATA_OK)
        {
            midi_app_data_t xMidiDefaultCfg = {0};

            /* Base address */
            xMidiDefaultCfg.u8Mode = MIDI_APP_DATA_DEFAULT_MODE;
            xMidiDefaultCfg.u8BaseChannel = MIDI_APP_DATA_DEFAULT_CH;
            xMidiDefaultCfg.u8Program = MIDI_APP_DATA_DEFAULT_PROG;

            if (xAPP_DATA_save_element(&xMidiAppData, (void *)&xMidiDefaultCfg) == APP_DATA_OK)
            {
                bRetVal = true;
            }
        }
    }
    else if (xInitStatus == APP_DATA_OK)
    {
        bRetVal = true;
    }

    return bRetVal;
}

bool bMIDI_APP_DATA_write(midi_app_data_t * pxMidiData)
{
    bool bRetVal = false;

    if (xAPP_DATA_save_element(&xMidiAppData, (void *)pxMidiData) == APP_DATA_OK)
    {
        bRetVal = true;
    }

    return bRetVal;
}

bool bMIDI_APP_DATA_read(const midi_app_data_t ** pxMidiData)
{
    bool bRetVal = false;

    *pxMidiData = pvAPP_DATA_get_element(&xMidiAppData);

    if (*pxMidiData != NULL)
    {
        bRetVal = true;
    }

    return bRetVal;
}

/* EOF */
