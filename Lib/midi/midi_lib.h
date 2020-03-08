/**
 * @file    midi_lib.h
 * @author  Sebastian Del Moral
 * @brief   Library for handling and parsing midi messages.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MIDI_LIB_H
#define __MIDI_LIB_H

#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

  /* Exported types ------------------------------------------------------------*/

/** Midi modes */
typedef enum
{
    /** Omni-on Poly */
    MidiMode1 = 0x00,
    /** Omni-on Mono */
    MidiMode2,
    /** Omni-off Poly */
    MidiMode3,
    /** Omni-off Mono */
    MidiMode4,
    /** Number of defined modes */
    MidiModeNum,
} midiMode_t;

  /** Defined midi lib status */
  typedef enum
  {
    midiOk = 0x00,
    midiSysExcBuffFull,
    midiHandleNewState,
    midiError,
  } midiStatus_t;

  /** CB for handle sys_ex */
  typedef void (*midi_cb_msg_sys_ex_t)(uint8_t *pdata, uint32_t len_data);

  /** CB for handle 1 data message */
  typedef void (*midi_cb_msg_data1_t)(uint8_t cmd, uint8_t data);

  /** CB for handle 2 data message */
  typedef void (*midi_cb_msg_data2_t)(uint8_t cmd, uint8_t data0, uint8_t data1);

  /** CB for handle rt message */
  typedef void (*midi_cb_msg_rt_t)(uint8_t rt_data);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/

/* MIDI status cmd with 1 bytes */
#define MIDI_STATUS_PROG_CHANGE 0xC0
#define MIDI_STATUS_CH_PRESS 0xD0
#define MIDI_STATUS_TIME_CODE 0xF1
#define MIDI_STATUS_SONG_SELECT 0xF3

/* MIDI status cmd with 2 bytes */
#define MIDI_STATUS_NOTE_OFF 0x80
#define MIDI_STATUS_NOTE_ON 0x90
#define MIDI_STATUS_POLY_PRESS 0xA0
#define MIDI_STATUS_CC 0xB0
#define MIDI_STATUS_PITCH_BEND 0xE0
#define MIDI_STATUS_SONG_POS 0xF2

/* Mask to get channel from status messages */
#define MIDI_STATUS_CH_MASK 0x0F
#define MIDI_STATUS_CMD_MASK 0xF0

/* CC types */
#define MIDI_CC_MOD 0x01
#define MIDI_CC_BRI 0x4A
#define MIDI_CC_HAR 0x47
#define MIDI_CC_ATT 0x49
#define MIDI_CC_C15 0x15
#define MIDI_CC_C16 0x16
#define MIDI_CC_C17 0x17
#define MIDI_CC_C18 0x18
#define MIDI_CC_C4B 0x4B
#define MIDI_CC_NOTE_OFF 0x7B

/* MIDI RT */
#define MIDI_RT_MASK 0xF8
#define MIDI_RT_CLK 0xF8
#define MIDI_RT_START 0xFA
#define MIDI_RT_CONT 0xFB
#define MIDI_RT_STOP 0xFC
#define MIDI_RT_RESET 0xFF

/* MIDI SYS_EX */
#define MIDI_STATUS_SYS_EX_START 0xF0
#define MIDI_STATUS_SYS_EX_END 0xF7

/* Sys ex max buffer size */
#define SYS_EX_BUFF_SIZE 310

  /* Exported functions prototypes ---------------------------------------------*/

  /**
 * @brief  Init library FSM and setup callback functions for rx parsing.
 * @param  cb_sys_ex: callback function for sysEx messages.
 * @param  cb_msg_data1: callback function for 1 data messages.
 * @param  cb_msg_data2: callback function for 2 data messages.
 * @param  cb_msg_rt: callback function for real time messages
 * @retval Operation status.
 */
  midiStatus_t midi_init(
      midi_cb_msg_sys_ex_t cb_sys_ex,
      midi_cb_msg_data1_t cb_msg_data1,
      midi_cb_msg_data2_t cb_msg_data2,
      midi_cb_msg_rt_t cb_msg_rt);

  /**
 * @brief  Update midi rx_fsm.
 * @param  data_rx: input byte to update midi_rx fsm.
 * @retval Operation result.
 */
  midiStatus_t midi_update_fsm(uint8_t data_rx);

  /**
 * @brief  Set rx_fsm into reset state.
 * @param  None.
 * @retval Operation result.
 */
  midiStatus_t midi_reset_fsm(void);

  /**
 * @brief  Get buffer for SysEx data.
 * @param  pu8Data pointer to store buffer used for sysex data
 * @param  pu32LenData pointer to store the len of the current sysex data.
 * @retval Operation result.
 */
  midiStatus_t midi_get_sysex_data(uint8_t ** pu8Data, uint32_t * pu32LenData);

#ifdef __cplusplus
}
#endif

#endif /* __MIDI_LIB_H */

/* EOF */
