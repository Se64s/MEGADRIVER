#! /usr/bin/env python
"""
Utility to extract YM2612 reg info from VGI files
"""
import argparse
import struct
import YM2612
import time
import sys

def read_byte(file_handler):
    return struct.unpack('B', file_handler.read(1))[0]

def load_vgi_file(file_path, ym_handler):
    retval = False
    print ("VGI: LOAD FILE", file_path)
    with open(file_path, 'rb') as byte_reader:
        byte_data = byte_reader.read()
        if len(byte_data) == 43:
            byte_reader.seek(0)
            # Read algorithm
            ym_handler.op_algorithm = read_byte(byte_reader)
            ym_handler.feedback = read_byte(byte_reader)
            reg = read_byte(byte_reader)
            ym_handler.audio_out = (reg >> 6) & 0x03
            ym_handler.amp_mod_sens = (reg >> 4) & 0x03
            ym_handler.phase_mod_sens = (reg >> 0) & 0x07
            # Operator CFG
            for operator_id in range(4):
                ym_handler.channel[0].operator[operator_id].multiple = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].detune = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].total_level = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].key_scale = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].attack_rate = read_byte(byte_reader)
                reg = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].amp_mod_on = (reg >> 7) & 0x01
                ym_handler.channel[0].operator[operator_id].decay_rate = (reg >> 0) & 0x1F
                ym_handler.channel[0].operator[operator_id].sustain_rate = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].release_rate = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].sustain_level = read_byte(byte_reader)
                ym_handler.channel[0].operator[operator_id].ssg_envelope = read_byte(byte_reader)
            # Copy channel 1 in other channels
            for channel_id in range(5):
                ym_handler.channel[channel_id + 1] = ym_handler.channel[0]
                ym_handler.channel[channel_id + 1].channel_id = channel_id + 1
            retval = True
        else:
            print("VGI: Error on file size (%d/43)" % len(byte_data))
    # Show operation result
    if retval == False:
        print ("VGI: ERR")
    else:
        print ("VGI: OK")
    return retval

def set_reg_values(ym_handler):
    """Set current register values"""
    retval = False
    print ("VGI: SET REGISTER")
    if ym_handler.ser_com:
        ym_handler.set_reg_values()
        retval = True
    if ym_handler.midi_com:
        ym_handler.midi_set_reg_values()
        retval = True
    return retval

def load_preset(ym_handler, preset_pos):
    """Load preset into defined position"""
    retval = False
    if preset_pos in range(YM2612.YM_MAX_NUM_USER_PRESETS):
        print ("VGI: LOAD PRESET", preset_pos)
        ym_handler.midi_load_preset(preset_pos)
        retval = True
    else:
        print ("VGI: NOT VALID PRESETS ID")
    return retval

def load_default_preset(ym_handler, preset_pos):
    """Load default preset into defined position"""
    retval = False
    if preset_pos in range(YM2612.YM_MAX_NUM_DEFAULT_PRESETS):
        print ("VGI: LOAD DEFAULT PRESET", preset_pos)
        ym_handler.midi_load_default_preset(preset_pos)
        retval = True
    else:
        print ("VGI: NOT VALID PRESETS ID")
    return retval

def save_preset(ym_handler, preset_pos):
    """Save preset into defined position"""
    retval = False
    print ("VGI: SAVE PRESET", preset_pos)
    ym_handler.set_reg_values()
    if preset_pos in range(YM2612.YM_MAX_NUM_USER_PRESETS):
        print ("VGI: SAVE PRESET", preset_pos)
        preset_name = "User preset %d" % preset_pos
        ym_handler.midi_save_preset(preset_pos, preset_name)
        retval = True
    else:
        print ("VGI: NOT VALID PRESETS ID")
    return retval

def main():
    # Get parameters
    parser = argparse.ArgumentParser()
    parser.add_argument('-f','--file', type=str, required=False, help="path to vgi file to process")
    parser.add_argument('-s','--serial', type=str, required=False, help="serial port to use")
    parser.add_argument('-m', '--midi', type=int, required=False, help="midi port to use")
    parser.add_argument('-sr', '--set-register', action='store_true', required=False, help="set register values on device")
    parser.add_argument('-dp', '--default-preset', type=int, required=False, help="load default preset")
    parser.add_argument('-lp', '--load-preset', type=int, required=False, help="load user preset")
    parser.add_argument('-sp', '--save-preset', type=int, required=False, help="save preset into a slot")
    args = parser.parse_args()
    vgi_file_path = args.file
    # Create handler for YM chip
    fm_chip = YM2612.YM2612Chip(ser_com=args.serial, midi_com=args.midi)
    # Try to get register values from file
    if args.file:
        load_vgi_file(vgi_file_path, fm_chip)
    # Load register values
    if args.set_register:
        set_reg_values(fm_chip)
    # Execute load vendor preset action
    elif args.default_preset != None:
        load_default_preset(fm_chip, args.default_preset)
    # Execute save preset action
    elif args.save_preset != None:
        save_preset(fm_chip, args.save_preset)
    # Execute load preset action
    elif args.load_preset != None:
        load_preset(fm_chip, args.load_preset)

if __name__ == "__main__":
    main()
    sys.exit(0)