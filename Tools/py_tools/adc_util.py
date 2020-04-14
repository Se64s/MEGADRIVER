#! /usr/bin/env python
"""
Utility to calculate adc range and tolenrances
"""
# Range in mV
v_range = 10000
adc_count = pow(2,12)
# adc_resolution = v_range / adc_count
# print("ADC resolution:", v_range / adc_count, "mV")
# # v_oct values
# note_voltage_step = 1000 / 12
# print("Number steps per note:", note_voltage_step / adc_resolution)

def voltage_to_count(voltage):
    a = -2048.0 / 5000.0
    b = 2048.0
    retval = round(voltage * a + b)
    return retval

def get_param_value(max_param_value, voltage_in):
    param_scale = max_param_value / adc_count
    param_count = adc_count - voltage_to_count(voltage_in)
    out_value = round(param_scale * param_count)
    print("Vin", voltage_in, "mV", "Out", out_value)
    return out_value

# Compute range scales
max_value_range = 8
print("Test ADC")
for Vin in range(-5000, 5000, 333):
    get_param_value(max_value_range, Vin)