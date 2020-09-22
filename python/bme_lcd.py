import smbus2
import bme280
from lcd_16x2_i2c import RPi_I2C_driver
import time
import sys

porta_i2c = 1
endereco = 0x76
bus = smbus2.SMBus(porta_i2c)

calibracao_paramentros = bme280.load_calibration_params(bus, endereco)

lcd = RPi_I2C_driver.lcd()

lcd.lcd_display_string(f"Joao Rossi", 1)

while True:
    try:
        dado = bme280.sample(bus, endereco, calibracao_paramentros)
        l1 = f"T:{str(round(dado.temperature, 2))} U:{str(round(dado.humidity, 2))}"
        l2 = f"P:{str(round(dado.pressure, 2))}"
        print(l1)
        print(l2)
        lcd.lcd_display_string(f"{l1}", 1)
        lcd.lcd_display_string(f"{l2}", 2)
        time.sleep(1)
    except KeyboardInterrupt:
        print("\nKilling program")
        sys.exit(0)

