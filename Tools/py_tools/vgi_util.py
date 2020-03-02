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

def process_file(file_path, ym_handler):
    with open(file_path, 'rb') as byte_reader:
        byte_data = byte_reader.read()
        if len(byte_data) == 43:
            byte_reader.seek(0)
            # Read algorithm - 3
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
            # Set all channels
            ym_handler.set_reg_values()
            return True
        else:
            print("VGI: Error on file size (%d/43)" % len(byte_data))
            return False

def main():
    # Get parameters
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', type=str, required=True, help="path to vgi file to process")
    parser.add_argument('-s', type=str, required=True, help="serial port to use")
    args = parser.parse_args()
    vgi_file_path = args.f
    serial_com = args.s
    # Create handler for YM chip
    fm_chip = YM2612.YM2612Chip(com=serial_com)
    fm_chip.reset_board()
    # Try to get register values from file
    if process_file(vgi_file_path, fm_chip):
        print ("VGI: OK")
    else:
        print ("VGI: ERR")

if __name__ == "__main__":
    main()
    sys.exit(0)