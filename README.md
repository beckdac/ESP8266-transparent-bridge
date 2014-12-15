ESP8266-transparent-bridge
==========================

Absolutely transparent bridge for the ESP8266

Parts of this firmware are from the stock AT firmware and the esphttpd project.

Pros: totally transparent bridge from UART to serial.  Have a serial project that you just want to add WiFi?  This will work.

Cons: zero configuration for the ESP8266 is provided.  The best way to handle this, currently, is to configure the board with the stock AT firmware so that the settings get pushed to flash, then burn this firmware and go.

Baud rate for the UART is fixed to 115200.  Change in user/user_main.c

Enjoy.

Flash command, e.g. w/ locations:
```
/opt/Espressif/esptool-py/esptool.py --port /dev/tty.usbserial-A603HRFF write_flash 0x00000 eagle.app.v6.flash.bin 0x40000 eagle.app.v6.irom0text.bin
```
