#! /usr/bin/env python
"""
Set a register preset to test different configurations on YM2612 chip
"""
from __future__ import print_function
import sys
import time
import serial


class YM2612Chip:
    """
    Class to handle and store information regading to FM synth chip
    """
    def __init__(self, 
            com="COM3", 
            baudrate="115200", 
            lfo_on=0, 
            lfo_freq=0):
        # Serial port handler
        self.ser = serial.Serial(com, baudrate, timeout=1)
        # Chip general register
        self.lfo_on = lfo_on
        self.lfo_freq = lfo_freq
        # Channel definition
        self.channel = {
            0: self.__YMChannel(channel_id=0),
            1: self.__YMChannel(channel_id=1),
            2: self.__YMChannel(channel_id=2),
            3: self.__YMChannel(channel_id=3),
            4: self.__YMChannel(channel_id=4),
            5: self.__YMChannel(channel_id=5)
        }

    def __send_cmd(self, cli_cmd):
        self.ser.write(cli_cmd)
        retval = False
        rx_buff = ""
        while True:
            byte_rx = self.ser.read()
            if len(byte_rx) == 0:
                break
            rx_buff += byte_rx.decode("utf-8")
            if "OK" in rx_buff:
                retval = True
                break
        if retval:
            print("CMD: OK")
            time.sleep(0.01)
        else:
            print("CMD: ERR")
        return retval

    def __write_reg(self, addr, data, bank):
        reg_addr = int(addr)
        reg_data = int(data)
        reg_bank = int(bank)
        print("CMD: writeReg: %02X-%02X-%02X" % (reg_addr, reg_data, reg_bank))
        cli_cmd = b'writeReg %d %d %d\n' % (reg_addr, reg_data, reg_bank)
        return self.__send_cmd(cli_cmd)

    def reset_board(self):
        print("CMD: reset")
        cli_cmd = b'reset\n'
        if self.__send_cmd(cli_cmd):
            time.sleep(2)
            return True
        else:
            return False

    def setup_channel(self, channel_id):
        if (channel_id < 6):
            channel_offset = channel_id % 3
            reg_sel = channel_id / 3
            # Set feedback and algorithm
            reg = self.channel[channel_id].get_reg_FBALG()
            self.__write_reg(0xB0 + channel_offset, reg, reg_sel)
            # Set Audio out and lfo destination
            reg = self.channel[channel_id].get_reg_LRAMSPMS()
            self.__write_reg(0xB7 + channel_offset, reg, reg_sel)
        else:
            print("CH: id not valid")

    def setup_operator(self, channel_id, operator_id):
        if (channel_id < 6) and (operator_id < 4):
            operator_offset = operator_id * 4
            channel_offset = channel_id % 3
            reg_sel = channel_id / 3
            # Set DT/MUL
            reg = self.channel[channel_id].operator[operator_id].get_reg_DETMUL()
            self.__write_reg(0x30 + channel_offset + operator_offset, reg, reg_sel)
            # Set TL, 0dB-96dB
            reg = self.channel[channel_id].operator[operator_id].get_reg_TL()
            self.__write_reg(0x40 + channel_offset + operator_offset, reg, reg_sel)
            # Set KS/AR
            reg = self.channel[channel_id].operator[operator_id].get_reg_KSAR()
            self.__write_reg(0x50 + channel_offset + operator_offset, reg, reg_sel)
            # Set AM/DR
            reg = self.channel[channel_id].operator[operator_id].get_reg_AMDR()
            self.__write_reg(0x60 + channel_offset + operator_offset, reg, reg_sel)
            # Set SR
            reg = self.channel[channel_id].operator[operator_id].get_reg_SR()
            self.__write_reg(0x70 + channel_offset + operator_offset, reg, reg_sel)
            # Set SL/RL
            reg = self.channel[channel_id].operator[operator_id].get_reg_SLRL()
            self.__write_reg(0x80 + channel_offset + operator_offset, reg, reg_sel)
            # Set SSG-EG
            reg = self.channel[channel_id].operator[operator_id].get_reg_SSGEG()
            self.__write_reg(0x90 + channel_offset + operator_offset, reg, reg_sel)
        else:
            print("OP: id not valid")

    def setup_LFO(self):
        reg = self.__get_LFO()
        self.__write_reg(0x22, reg, 0)

    def __get_LFO(self):
        reg = ((self.lfo_on & 0x01) << 3) | (self.lfo_freq & 0x07)
        return reg

    def set_reg_values(self):
        """
        Set current values
        """
        print("SETCFG: Start")
        # Set board registers
        self.setup_LFO()
        for voice in range(6):
            self.setup_channel(voice)
            for operator in range(4):
                self.setup_operator(voice, operator)
        print("SETCFG: End")

    def set_preset(self, preset_id):
        """
        Set preset:
            0-Bell
            1-Piano
            2-Eorgan
            3-Brass
            4-String
            5-Vibrphn
        """
        # Bell
        if preset_id == 0:
            print("PRESET: Bell")
            # Reset board
            self.reset_board()
            # Set board registers
            self.lfo_on = 1
            self.lfo_freq = 0
            self.setup_LFO()
            for voice in range(6):
                print("PRESET: Setup voice", voice)
                # Setup voice 0
                self.channel[voice].op_algorithm = 4
                self.channel[voice].feedback = 3
                self.channel[voice].audio_out = 3
                self.channel[voice].phase_mod_sens = 0
                self.channel[voice].amp_mod_sens = 2
                self.setup_channel(voice)
                # Setup operator 0
                self.channel[voice].operator[0].total_level = 0x28 # 30
                self.channel[voice].operator[0].multiple = 15
                self.channel[voice].operator[0].detune = 3
                self.channel[voice].operator[0].attack_rate = 31
                self.channel[voice].operator[0].decay_rate = 4
                self.channel[voice].operator[0].sustain_level = 1
                self.channel[voice].operator[0].sustain_rate = 10
                self.channel[voice].operator[0].release_rate = 3
                self.channel[voice].operator[0].key_scale = 1
                self.channel[voice].operator[0].amp_mod_on = 1
                self.channel[voice].operator[0].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 0)
                # Setup operator 1
                self.channel[voice].operator[1].total_level = 0x08 # 6
                self.channel[voice].operator[1].multiple = 3
                self.channel[voice].operator[1].detune = 5 # -1
                self.channel[voice].operator[1].attack_rate = 30
                self.channel[voice].operator[1].decay_rate = 8
                self.channel[voice].operator[1].sustain_level = 0x03 # 9
                self.channel[voice].operator[1].sustain_rate = 6
                self.channel[voice].operator[1].release_rate = 3
                self.channel[voice].operator[1].key_scale = 1
                self.channel[voice].operator[1].amp_mod_on = 0
                self.channel[voice].operator[1].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 1)
                # Setup operator 2
                self.channel[voice].operator[2].total_level = 0x0C # 19
                self.channel[voice].operator[2].multiple = 7
                self.channel[voice].operator[2].detune = 5 # -1
                self.channel[voice].operator[2].attack_rate = 31
                self.channel[voice].operator[2].decay_rate = 4
                self.channel[voice].operator[2].sustain_level = 0x03 # 9
                self.channel[voice].operator[2].sustain_rate = 17
                self.channel[voice].operator[2].release_rate = 1
                self.channel[voice].operator[2].key_scale = 1
                self.channel[voice].operator[2].amp_mod_on = 0
                self.channel[voice].operator[2].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 2)
                # Setup operator 3
                self.channel[voice].operator[3].total_level = 0x04 # 3
                self.channel[voice].operator[3].multiple = 2
                self.channel[voice].operator[3].detune = 4 # 0
                self.channel[voice].operator[3].attack_rate = 31
                self.channel[voice].operator[3].decay_rate = 5
                self.channel[voice].operator[3].sustain_level = 0x02 # 6
                self.channel[voice].operator[3].sustain_rate = 12
                self.channel[voice].operator[3].release_rate = 3
                self.channel[voice].operator[3].key_scale = 1
                self.channel[voice].operator[3].amp_mod_on = 0
                self.channel[voice].operator[3].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 3)
        # Piano
        elif preset_id == 1:
            print("PRESET: Piano")
            # Reset board
            self.reset_board()
            # Set board registers
            self.lfo_on = 0
            self.lfo_freq = 0
            self.setup_LFO()
            for voice in range(6):
                print("PRESET: Setup voice", voice)
                # Setup voice 0
                self.channel[voice].op_algorithm = 4
                self.channel[voice].feedback = 2
                self.channel[voice].audio_out = 3
                self.channel[voice].phase_mod_sens = 0
                self.channel[voice].amp_mod_sens = 1
                self.setup_channel(voice)
                # Setup operator 0
                self.channel[voice].operator[0].total_level = 0x30 # 36
                self.channel[voice].operator[0].multiple = 1
                self.channel[voice].operator[0].detune = 0
                self.channel[voice].operator[0].attack_rate = 31
                self.channel[voice].operator[0].decay_rate = 0
                self.channel[voice].operator[0].sustain_level = 1 # 3
                self.channel[voice].operator[0].sustain_rate = 8
                self.channel[voice].operator[0].release_rate = 0
                self.channel[voice].operator[0].key_scale = 3
                self.channel[voice].operator[0].amp_mod_on = 0
                self.channel[voice].operator[0].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 0)
                # Setup operator 1
                self.channel[voice].operator[1].total_level = 0x01 # 3
                self.channel[voice].operator[1].multiple = 0
                self.channel[voice].operator[1].detune = 0 # 0
                self.channel[voice].operator[1].attack_rate = 25
                self.channel[voice].operator[1].decay_rate = 7
                self.channel[voice].operator[1].sustain_level = 0x01 # 3
                self.channel[voice].operator[1].sustain_rate = 6
                self.channel[voice].operator[1].release_rate = 7
                self.channel[voice].operator[1].key_scale = 2
                self.channel[voice].operator[1].amp_mod_on = 0
                self.channel[voice].operator[1].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 1)
                # Setup operator 2
                self.channel[voice].operator[2].total_level = 0x30 # 36
                self.channel[voice].operator[2].multiple = 2
                self.channel[voice].operator[2].detune = 0
                self.channel[voice].operator[2].attack_rate = 31
                self.channel[voice].operator[2].decay_rate = 0
                self.channel[voice].operator[2].sustain_level = 0x01 # 3
                self.channel[voice].operator[2].sustain_rate = 8
                self.channel[voice].operator[2].release_rate = 0
                self.channel[voice].operator[2].key_scale = 3
                self.channel[voice].operator[2].amp_mod_on = 0
                self.channel[voice].operator[2].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 2)
                # Setup operator 3
                self.channel[voice].operator[3].total_level = 0x01 # 3
                self.channel[voice].operator[3].multiple = 1
                self.channel[voice].operator[3].detune = 0 # 0
                self.channel[voice].operator[3].attack_rate = 27
                self.channel[voice].operator[3].decay_rate = 7
                self.channel[voice].operator[3].sustain_level = 0x01 # 3
                self.channel[voice].operator[3].sustain_rate = 6
                self.channel[voice].operator[3].release_rate = 7
                self.channel[voice].operator[3].key_scale = 2
                self.channel[voice].operator[3].amp_mod_on = 0
                self.channel[voice].operator[3].ssg_envelope = 0x00 # OFF
                self.setup_operator(voice, 3)
        else:
            print("PRESET: Id not found")
        print("PRESET: End")

    class __YMChannel:
        """
        Class to handle YM2612 voice
        """
        def __init__(self, 
                    channel_id=0, 
                    feedback=0, 
                    op_algorithm=0, 
                    audio_out=0, 
                    phase_mod_sens=0, 
                    amp_mod_sens=0):
            # Channel attributes
            self.channel_id = channel_id
            self.feedback = feedback
            self.op_algorithm = op_algorithm
            self.audio_out = audio_out
            self.phase_mod_sens = phase_mod_sens
            self.amp_mod_sens = amp_mod_sens
            # Channel operators
            self.operator = {
                0: self.__YMOperator(op_id=0),
                1: self.__YMOperator(op_id=1),
                2: self.__YMOperator(op_id=2),
                3: self.__YMOperator(op_id=3),
            }

        def get_reg_LRAMSPMS(self):
            reg = ((self.audio_out & 0x03) << 6) | ((self.amp_mod_sens & 0x03) << 4) | (self.phase_mod_sens & 0x07)
            return reg

        def get_reg_FBALG(self):
            reg = ((self.feedback & 0x03) << 3) | (self.op_algorithm & 0x07)
            return reg

        class __YMOperator:
            """
            Class with operator data
            """
            def __init__(self, 
                    op_id=0,
                    detune=0, 
                    multiple=0, 
                    total_level=0, 
                    key_scale=0, 
                    attack_rate=0, 
                    amp_mod_on=0,
                    decay_rate=0,
                    sustain_rate=0,
                    sustain_level=0,
                    release_rate=0,
                    ssg_envelope=0):
                # Operator attributes
                self.detune = detune
                self.multiple = multiple
                self.total_level = total_level
                self.key_scale = key_scale
                self.attack_rate = attack_rate
                self.amp_mod_on = amp_mod_on
                self.decay_rate = decay_rate
                self.sustain_rate = sustain_rate
                self.sustain_level = sustain_level
                self.release_rate = release_rate
                self.ssg_envelope = ssg_envelope

            def get_reg_DETMUL(self):
                reg = ((self.detune << 4) & 0x70) | (self.multiple & 0x0F)
                return reg

            def get_reg_TL(self):
                reg = self.total_level & 0x7F
                return reg

            def get_reg_KSAR(self):
                reg = ((self.key_scale & 0x03) << 6) | (self.attack_rate & 0x1F)
                return reg

            def get_reg_AMDR(self):
                reg = ((self.amp_mod_on & 0x01) << 7) | (self.decay_rate & 0x1F)
                return reg

            def get_reg_SR(self):
                reg = (self.sustain_rate & 0x1F)
                return reg

            def get_reg_SLRL(self):
                reg = ((self.sustain_level & 0x0F) << 4) | (self.release_rate & 0x0F)
                return reg

            def get_reg_SSGEG(self):
                reg = self.ssg_envelope & 0x0F
                return reg

def main():
    print("\r\nYM2612: Preset TEST\r\n")
    synth = YM2612Chip()
    synth.set_preset(1)

if __name__ == "__main__":
    main()
    sys.exit(0)
