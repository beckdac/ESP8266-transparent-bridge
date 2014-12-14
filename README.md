ESP8266-transparent-bridge
==========================

Absolutely transparent bridge for the ESP8266

Parts of this firmware are from the stock AT firmware and the esphttpd project.

Pros: totally transparent bridge from UART to serial.  Have a serial project that you just want to add WiFi too?  This will work.

Cons: zero configuration for the ESP8266 is provided.  The best way to handle this, currently, is to configure the board with the stock AT firmware so that the settings get pushed to flash, then burn this firmware and go.

Enjoy.
