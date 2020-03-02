#! /usr/bin/env python
"""
Get table of base frecuencies to use by YM2613 chip
"""
from __future__ import print_function
import sys

def main():
    # C4-B4 -> Octave 4
    base_note_freq = [
        4938, 5232, 5543, 5872,
        6222, 6591, 6983, 7399,
        7839, 8305, 8799, 9322
    ]

    f_num_list = []
    block_num = 4
    for note in base_note_freq:
        f_num = note / pow(2, block_num - 1)
        f_num_list.append(int(f_num))
    
    print("f_num: ", f_num_list, "block: ", block_num)

if __name__ == "__main__":
    main()
    sys.exit(0)