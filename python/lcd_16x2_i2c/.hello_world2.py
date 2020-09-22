import RPi_I2C_driver
from time import *

mylcd = RPi_I2C_driver.lcd()

mylcd.lcd_display_string("Hello World!", 1)