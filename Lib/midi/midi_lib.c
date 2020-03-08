/**
 * @file    midi_lib.c
 * @author  Sebastian Del Moral
 * @brief   Library for handling and parsing midi messages.
 *
 */

/* Includes ------------------------------------------------------------------*/

#include "midi_lib.h"

/* Private typedef -----------------------------------------------------------*/

/** List of defined states used in rx_fsm */
typedef enum
{
    wait_byte_init = 0,
    wait_byte_first_data,
    wait_byte_second_data,
    wait_byte_sys_ex,
    wait_byte_data,
    dispatch_status,
    handle_1_data_msg,
    handle_2_data_msg,
    handle_sys_ex_msg,
    handle_rt_msg,
} midi_rx_state_t;

/* Private define ------------------------------------------------------------*/

#define MIDI_STATUS_MASK (1 << 7)
#define MIDI_STATUS_RT_MASK (0xF8)

#define MIDI_IS_STATUS(data) (((data)&MIDI_STATUS_MASK) == MIDI_STATUS_MASK)
//#define MIDI_IS_RT(data)				(((data) & MIDI_STATUS_RT_MASK) == MIDI_STATUS_RT_MASK)
#define MIDI_IS_RT(data) ((((data)&MIDI_STATUS_RT_MASK) == MIDI_STATUS_RT_MASK) && ((data) != MIDI_STATUS_SYS_EX_START) && ((data) != MIDI_STATUS_SYS_EX_END))
#define MIDI_STATUS_GET_CH(status) (((status)&MIDI_STATUS_CH_MASK) >> 0)
#define MIDI_STATUS_GET_CMD(status) (((status)&MIDI_STATUS_CMD_MASK) >> 4)

/* Private variables ---------------------------------------------------------*/

/* Cb pointer for handle sys_ex functions */
midi_cb_msg_sys_ex_t midi_cb_sys_ex = NULL;

/* Cb pointer for handle msg data 1 functions */
midi_cb_msg_data1_t midi_cb_msg_data_1 = NULL;

/* Cb pointer for handle msg data 2 functions */
midi_cb_msg_data2_t midi_cb_msg_data_2 = NULL;

/* Cb pointer for handle msg rt functions */
midi_cb_msg_rt_t midi_cb_msg_rt = NULL;

/* Current system state control var */
static midi_rx_state_t fsm_rx_state = wait_byte_init;

/* Tmp var for running status */
static uint8_t midi_running_status = 0;
static uint8_t midi_tmp_data_1 = 0;
static uint8_t midi_tmp_data_2 = 0;
static uint32_t i_data_sys_ex = 0;
static uint8_t sys_ex_buff_data[SYS_EX_BUFF_SIZE] = {0};

/* Private function prototypes -----------------------------------------------*/

static midiStatus_t state_handler_wait_byte_init(uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_first_data(uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_second_data(uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_sys_ex(uint8_t rx_byte);
static midiStatus_t state_handler_wait_byte_data(uint8_t rx_byte);
static midiStatus_t state_handler_dispatch_status(uint8_t rx_byte);

/* Private function ----------------------------------------------------------*/

static midiStatus_t state_handler_wait_byte_init(uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (midi_cb_msg_rt != NULL)
            {
                midi_cb_msg_rt(rx_byte);
            }
        }
        else
        {
            fsm_rx_state = dispatch_status;
            retval = midiHandleNewState;
        }
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_first_data(uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (midi_cb_msg_rt != NULL)
            {
                midi_cb_msg_rt(rx_byte);
            }
        }
        else
        {
            fsm_rx_state = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        midi_tmp_data_1 = rx_byte;
        fsm_rx_state = wait_byte_second_data;
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_second_data(uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (midi_cb_msg_rt != NULL)
            {
                midi_cb_msg_rt(rx_byte);
            }
        }
        else
        {
            fsm_rx_state = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        midi_tmp_data_2 = rx_byte;
        if (midi_cb_msg_data_2 != NULL)
        {
            midi_cb_msg_data_2(midi_running_status, midi_tmp_data_1, midi_tmp_data_2);
        }
        fsm_rx_state = wait_byte_first_data;
        midi_tmp_data_1 = 0;
        midi_tmp_data_2 = 0;
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_sys_ex(uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (midi_cb_msg_rt != NULL)
            {
                midi_cb_msg_rt(rx_byte);
            }
        }
        else
        {
            if (rx_byte == MIDI_STATUS_SYS_EX_END)
            {
                /* Handle end of sys_ex */
                if (midi_cb_sys_ex != NULL)
                {
                    midi_cb_sys_ex(sys_ex_buff_data, i_data_sys_ex);
                    //i_data_sys_ex = 0;
                }
            }
            fsm_rx_state = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        /* Handle data byte for sys_ex */
        if (i_data_sys_ex < SYS_EX_BUFF_SIZE)
        {
            sys_ex_buff_data[i_data_sys_ex++] = rx_byte;
        }
        else
        {
            i_data_sys_ex = 0;
            fsm_rx_state = wait_byte_init;
            retval = midiSysExcBuffFull;
        }
    }

    return retval;
}

static midiStatus_t state_handler_wait_byte_data(uint8_t rx_byte)
{
    midiStatus_t retval = midiOk;

    if (MIDI_IS_STATUS(rx_byte))
    {
        if (MIDI_IS_RT(rx_byte))
        {
            if (midi_cb_msg_rt != NULL)
            {
                midi_cb_msg_rt(rx_byte);
            }
        }
        else
        {
            fsm_rx_state = dispatch_status;
            retval = midiHandleNewState;
        }
    }
    else
    {
        if (midi_cb_msg_data_1 != NULL)
        {
            midi_cb_msg_data_1(midi_running_status, rx_byte);
        }
    }

    return retval;
}

static midiStatus_t state_handler_dispatch_status(uint8_t rx_byte)
{
    midiStatus_t retval = midiError;

    /* Check if 1 data cmd */
    if (((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_PROG_CHANGE) ||
        ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_CH_PRESS) ||
        (rx_byte == MIDI_STATUS_TIME_CODE) ||
        (rx_byte == MIDI_STATUS_SONG_SELECT))
    {
        fsm_rx_state = wait_byte_data;
        retval = midiOk;
    }
    /* Check if 2 data cmd */
    else if (((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_OFF) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_NOTE_ON) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_POLY_PRESS) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_CC) ||
             ((rx_byte & MIDI_STATUS_CMD_MASK) == MIDI_STATUS_PITCH_BEND) ||
             (rx_byte == MIDI_STATUS_SONG_POS))
    {
        fsm_rx_state = wait_byte_first_data;
        retval = midiOk;
    }
    /* Check if sys ex cmd */
    else if (rx_byte == MIDI_STATUS_SYS_EX_START)
    {
        i_data_sys_ex = 0;
        fsm_rx_state = wait_byte_sys_ex;
        retval = midiOk;
    }
    /* If not defined, jump to start */
    else
    {
        fsm_rx_state = wait_byte_init;
    }

    /* Save running status */
    if (retval == midiOk)
    {
        midi_running_status = rx_byte;
    }

    return retval;
}

/* ---------------------------------------------------------------------------*/

/** State Handler definitions */
midiStatus_t (*rx_state_table[])(uint8_t rx_byte) = {
    state_handler_wait_byte_init,
    state_handler_wait_byte_first_data,
    state_handler_wait_byte_second_data,
    state_handler_wait_byte_sys_ex,
    state_handler_wait_byte_data,
    state_handler_dispatch_status};

/* Public function -----------------------------------------------------------*/

/* Init library FSM and setup callback functions for rx parsing */
midiStatus_t midi_init(midi_cb_msg_sys_ex_t cb_sys_ex,
                       midi_cb_msg_data1_t cb_msg_data1,
                       midi_cb_msg_data2_t cb_msg_data2,
                       midi_cb_msg_rt_t cb_msg_rt)
{
    midiStatus_t retval = midiOk;

    /* Init private cb handlers */
    if (cb_sys_ex != NULL)
    {
        midi_cb_sys_ex = cb_sys_ex;
    }
    if (cb_msg_data1 != NULL)
    {
        midi_cb_msg_data_1 = cb_msg_data1;
    }
    if (cb_msg_data2 != NULL)
    {
        midi_cb_msg_data_2 = cb_msg_data2;
    }
    if (cb_msg_rt != NULL)
    {
        midi_cb_msg_rt = cb_msg_rt;
    }

    fsm_rx_state = wait_byte_init;
    midi_running_status = 0;

    return retval;
}

/* Update midi rx_fsm */
midiStatus_t midi_update_fsm(uint8_t data_rx)
{
    midiStatus_t retval = midiError;

    do
    {
        retval = rx_state_table[fsm_rx_state](data_rx);
    } while (retval == midiHandleNewState);

    return retval;
}

/* Set rx_fsm into reset state */
midiStatus_t midi_reset_fsm(void)
{
    midiStatus_t retval = midiOk;

    fsm_rx_state = wait_byte_init;
    midi_running_status = 0;
    midi_tmp_data_1 = 0;
    midi_tmp_data_2 = 0;

    return retval;
}

midiStatus_t midi_get_sysex_data(uint8_t ** pu8Data, uint32_t * pu32LenData)
{
    midiStatus_t xRetval = midiError;

    if ((pu8Data != NULL) && (pu32LenData != NULL))
    {
        *pu8Data = sys_ex_buff_data;
        *pu32LenData = i_data_sys_ex;
        xRetval = midiOk;
    }

    return xRetval;
}

/* EOF */
