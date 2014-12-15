ESP8266-transparent-bridge
==========================

Absolutely transparent bridge for the ESP8266

This is really basic firmware for the ESP that creates a totally transparent TCP socket to ESP UART0 bridge. Characters come in on one interface and go out the other. The totally transparent bridge mode is something that has been missing and is available on higher priced boards.

Pros: 
It works. Do you already have a serial project to which you just want to add WiFi? This is your ticket. No more dealing with the AT command set.
You can program your Arduino over WiFi. Just hit the reset button and upload your sketch using avrdude's socket port, e.g.
```
avrdude -c avrisp -p m328p -P net:169.254.4.1:23 -F -U flash:w:mySketch.hex:i
```

Cons: 
Zero run-time configuration support at this time. In order to use this firmware, you have to have already configured the module using the AT firmware, including what AP to connect to, etc.. When you install this firmware, it uses those settings.
Unbuffered TCP writes. Each incoming UART character gets sent as a separate TCP packet. This could potentially impact performance, however, in my hands that hasn't been an issue.


Parts of this firmware are from the stock AT firmware and the esphttpd project.
Enjoy.

Flash command, e.g. w/ locations:
```
/opt/Espressif/esptool-py/esptool.py --port /dev/tty.usbserial-A603HRFF write_flash 0x00000 eagle.app.v6.flash.bin 0x40000 eagle.app.v6.irom0text.bin
```
