/**
 * @file app_lfs.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2021-01-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* Includes ------------------------------------------------------------------*/

#include "app_lfs.h"
#include "lfs.h"
#include "error.h"
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define LFS_BLOCK_SIZE          ( FLASH_PAGE_SIZE )
#define LFS_NUM_BLOCKS          ( 10U ) // This value should fit with init of memory block FLASH_LFS in linker file
#define LFS_PAGE_INIT           ( FLASH_PAGE_NB - LFS_NUM_BLOCKS )
#define LFS_FLASH_BASE_ADDR     ( FLASH_BASE + (LFS_PAGE_INIT * LFS_BLOCK_SIZE) )
#define LFS_READ_MIN_SIZE       ( 8U )
#define LFS_WRITE_MIN_SIZE      ( 8U )
#define LFS_CACHE_SIZE          ( 8U )
#define LFS_LOOKAHEAD_SIZE      ( 8U )
#define LFS_BLOCK_CYCLES        ( 500U )

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/**
 * Low level init for flash.
 */
int ll_flash_init(void)
{
    return 0U;
}

/**
 * Read a region in a block. Negative error codes are propogated
 * to the user.
 */
int ll_flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    ERR_ASSERT(off  % c->read_size == 0);
    ERR_ASSERT(size % c->read_size == 0);
    ERR_ASSERT(block < c->block_count);

    uint32_t u32FlashAddr = LFS_FLASH_BASE_ADDR + (block * LFS_BLOCK_SIZE) + off;
    uint8_t *pu8FlashData = (uint8_t *)u32FlashAddr;
    uint8_t *pu8OutData = (uint8_t *)buffer;

    while ( size-- != 0U )
    {
        *pu8OutData++ = *pu8FlashData++;
    }

    return 0;
}

/*
 * Program a region in a block. The block must have previously
 * been erased. Negative error codes are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 */
int ll_flash_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    ERR_ASSERT(off  % c->read_size == 0);
    ERR_ASSERT(size % c->read_size == 0);
    ERR_ASSERT(block < c->block_count);

    int iRetval = 0U;
    uint32_t u32FlashAddr = LFS_FLASH_BASE_ADDR + (block * LFS_BLOCK_SIZE) + off;
    uint64_t *pu64FlashData = (uint64_t *)u32FlashAddr;
    uint64_t *pu64WriteData = (uint64_t *)buffer;
    uint32_t u32NumTransfer = size / sizeof(uint64_t);

    HAL_FLASH_Unlock();

    while ( u32NumTransfer-- != 0 )
    {
        if ( HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, (uint32_t)pu64FlashData++, *pu64WriteData++) != HAL_OK )
        {
            iRetval = -1;
            break;
        }
    }

    HAL_FLASH_Lock();

    return iRetval;
}

/*
 * Erase a block. A block must be erased before being programmed.
 * The state of an erased block is undefined. Negative error codes
 * are propogated to the user.
 * May return LFS_ERR_CORRUPT if the block should be considered bad.
 */
int ll_flash_erase(const struct lfs_config *c, lfs_block_t block)
{
    ERR_ASSERT(block < c->block_count);

    int iRetval = 0U;
    uint32_t u32PageNumber = LFS_PAGE_INIT + block;
    uint32_t u32PageError = 0U;

    /* Fill EraseInit structure */
    FLASH_EraseInitTypeDef EraseInitStruct = {0};
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Page = u32PageNumber;
    EraseInitStruct.NbPages = 1;

    HAL_FLASH_Unlock();

    if ( HAL_FLASHEx_Erase(&EraseInitStruct, &u32PageError) != HAL_OK )
    {
        iRetval = -1;
    }

    HAL_FLASH_Lock();

    return iRetval;
}

/*
 * Sync the state of the underlying block device. Negative error codes
 * are propogated to the user.
 */
int ll_flash_sync(const struct lfs_config *c)
{
    return 0;
}

/* Private variables ---------------------------------------------------------*/

/** File system handler */
lfs_t xLfs;

/** File handler */
lfs_file_t xFile;

/** File system config */
const struct lfs_config xLfsCfg = {
    // block device operations
    .read  = ll_flash_read,
    .prog  = ll_flash_prog,
    .erase = ll_flash_erase,
    .sync  = ll_flash_sync,

    // block device configuration
    .read_size = LFS_READ_MIN_SIZE,
    .prog_size = LFS_WRITE_MIN_SIZE,
    .block_size = LFS_BLOCK_SIZE,
    .block_count = LFS_NUM_BLOCKS,
    .cache_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_cycles = LFS_BLOCK_CYCLES,
};

/** Midi cfg filename */
const char lfs_midi_cfg_filename[] = "midi_cfg";

/** YM cfg filenames */
const char *lfs_ym_cfg_filename[LFS_YM_SLOT_NUM] = {
    "ym_cfg_0",
    "ym_cfg_1",
    "ym_cfg_2",
    "ym_cfg_3",
    "ym_cfg_4",
};

/* Callback ------------------------------------------------------------------*/
/* Public user code ----------------------------------------------------------*/

lfs_status_t LFS_init(void)
{
    lfs_status_t eRetval = LFS_OK;

    if ( ll_flash_init() != 0U )
    {
        eRetval = LFS_ERROR;
    }
    else
    {
        // mount the filesystem
        int err = lfs_mount(&xLfs, &xLfsCfg);

        // reformat if we can't mount the filesystem, this should only happen on the first boot
        if ( err )
        {
            ERR_ASSERT( lfs_format(&xLfs, &xLfsCfg) == LFS_ERR_OK );
            ERR_ASSERT( lfs_mount(&xLfs, &xLfsCfg) == LFS_ERR_OK );

            // Init midi data
            {
                lfs_midi_data_t xMidiCfg = {
                    .u8Mode = MidiMode3,
                    .u8BaseChannel = 0U,
                    .u8Program = 0U,
                    .u8Bank = 0U
                };

                // Save file with config
                err = lfs_file_open( &xLfs, &xFile, lfs_midi_cfg_filename, LFS_O_RDWR | LFS_O_CREAT );
                ERR_ASSERT( err == LFS_ERR_OK );

                err = lfs_file_rewind( &xLfs, &xFile );
                ERR_ASSERT( err == LFS_ERR_OK );

                err = lfs_file_write( &xLfs, &xFile, (void*)&xMidiCfg, sizeof(lfs_midi_data_t) );
                ERR_ASSERT( err == sizeof(lfs_midi_data_t) );

                err = lfs_file_close( &xLfs, &xFile );
                ERR_ASSERT( err == LFS_ERR_OK );
            }

            // Init YM data
            {
                lfs_ym_data_t xYmCfg = { 0U };
                const char* pcName = pxSYNTH_APP_DATA_CONST_get_name(0U);
                const xFmDevice_t * pxInitPreset = pxSYNTH_APP_DATA_CONST_get(0U);
                xYmCfg.xPresetData = *pxInitPreset;
                strcpy((char*)xYmCfg.pu8Name, pcName);

                for ( uint32_t u32SlotIndex = 0U; u32SlotIndex < LFS_YM_SLOT_NUM; u32SlotIndex++ )
                {
                    err = lfs_file_open( &xLfs, &xFile, lfs_ym_cfg_filename[u32SlotIndex], LFS_O_WRONLY | LFS_O_CREAT );
                    ERR_ASSERT( err == LFS_ERR_OK );

                    err = lfs_file_rewind( &xLfs, &xFile );
                    ERR_ASSERT( err == LFS_ERR_OK );

                    err = lfs_file_write( &xLfs, &xFile, (void*)&xYmCfg, sizeof(lfs_ym_data_t) );
                    ERR_ASSERT( err == sizeof(lfs_ym_data_t) );

                    err = lfs_file_close( &xLfs, &xFile );
                    ERR_ASSERT( err == LFS_ERR_OK );
                }
            }
        }
    }

    return eRetval;
}

lfs_status_t LFS_read_midi_data(lfs_midi_data_t *pxData)
{
    if ( lfs_file_open(&xLfs, &xFile, lfs_midi_cfg_filename, LFS_O_RDONLY) == LFS_ERR_OK )
    {
        int err = lfs_file_rewind( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }

        err = lfs_file_read( &xLfs, &xFile, pxData, sizeof(lfs_midi_data_t) );
        if (err != sizeof(lfs_midi_data_t))
        {
            return LFS_ERROR;
        }

        err = lfs_file_close( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }
    }

    return LFS_OK;
}

lfs_status_t LFS_write_midi_data(lfs_midi_data_t *pxData)
{
    if ( lfs_file_open(&xLfs, &xFile, lfs_midi_cfg_filename, LFS_O_RDWR) == LFS_ERR_OK )
    {
        int err = lfs_file_rewind( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }

        err = lfs_file_write( &xLfs, &xFile, pxData, sizeof(lfs_midi_data_t) );
        if (err != sizeof(lfs_midi_data_t))
        {
            return LFS_ERROR;
        }

        err = lfs_file_close( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }
    }

    return LFS_OK;
}

lfs_status_t LFS_read_ym_data(uint8_t u8Slot, lfs_ym_data_t *pxData)
{
    ERR_ASSERT( u8Slot < (uint8_t)LFS_YM_SLOT_NUM );

    int err = lfs_file_open(&xLfs, &xFile, lfs_ym_cfg_filename[u8Slot], LFS_O_RDWR);

    if ( err == LFS_ERR_OK )
    {
        int err = lfs_file_rewind( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }

        err = lfs_file_read( &xLfs, &xFile, pxData, sizeof(lfs_ym_data_t) );
        if (err != sizeof(lfs_ym_data_t))
        {
            return LFS_ERROR;
        }

        err = lfs_file_close( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }
    }

    return LFS_OK;
}

lfs_status_t LFS_write_ym_data(uint8_t u8Slot, lfs_ym_data_t *pxData)
{
    ERR_ASSERT( u8Slot < (uint8_t)LFS_YM_SLOT_NUM );

    int err = lfs_file_open(&xLfs, &xFile, lfs_ym_cfg_filename[u8Slot], LFS_O_RDWR);

    if ( err == LFS_ERR_OK )
    {
        int err = lfs_file_rewind( &xLfs, &xFile );
        if (err != LFS_ERR_OK)
        {
            return LFS_ERROR;
        }

        err = lfs_file_write( &xLfs, &xFile, pxData, sizeof(lfs_ym_data_t) );
        if ( err != sizeof(lfs_ym_data_t) )
        {
            return LFS_ERROR;
        }

        err = lfs_file_close( &xLfs, &xFile );
        if ( err != LFS_ERR_OK )
        {
            return LFS_ERROR;
        }
    }

    return LFS_OK;
}

/* EOF */
