#! /usr/bin/env python
"""
Utility to calculate adc range and tolenrances
"""
# Range in mV
v_range = 10000
adc_count = pow(2,12)
adc_resolution = v_range / adc_count
print("ADC resolution:", v_range / adc_count, "mV")
# v_oct values
note_voltage_step = 1000 / 12
print("Number steps per note:", note_voltage_step / adc_resolution)