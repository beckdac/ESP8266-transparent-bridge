ESP8266-transparent-bridge
==========================

Absolutely transparent bridge for the ESP8266

This is really basic firmware for the ESP that creates a totally transparent TCP socket to ESP UART0 bridge. Characters come in on one interface and go out the other. The totally transparent bridge mode is something that has been missing and is available on higher priced boards.

**Pros: **

* It works. Do you already have a serial project to which you just want to add WiFi? This is your ticket. No more dealing with the AT command set.
* You can program your Arduino over WiFi. Just hit the reset button and upload your sketch using avrdude's socket port, e.g.
```
avrdude -c avrisp -p m328p -P net:192.168.4.1:23 -F -U flash:w:mySketch.hex:i
```
* Optional by compile time defines:
 * Static configuration each time the unit boots from values defined in user/config.h.  Uncomment the following line in user/config.h:
```
#define CONFIG_STATIC
```
 * Dynamic configuration by remote telnet using +++AT prefixed commands. Enabled by default.  To disable, comment the following line in user/config.h:
```
#define CONFIG_DYNAMIC
```
Telnet into the module and issue commands prefixed by +++AT to escape each command from bridge mode.  The dynamic configuration commands are:
```
+++AT                                    # do nothing, print OK
+++AT MODE                               # print current opmode 
+++AT MODE <mode: 1= STA, 2= AP, 3=both> # set current opmode
+++AT STA                                # print current ssid and password connected to
+++AT STA <ssid> <password>              # set ssid and password to connect to
+++AT AP                                 # print the current soft ap settings
+++AT AP <ssid>                          # set the AP as open with specified ssid
+++AT AP <ssid> <password>               # set the AP as WPA with password
+++AT BAUD                               # print current UART baud rate
+++AT BAUD <baud>                        # set currrent UART baud rate
+++AT PORT                               # print current incoming TCP socket port
+++AT PORT <port>                        # set current incoming TCP socket port (restarts ESP)
+++AT RESET                              # software reset the unit
```
Upon success, all commands send back "OK" as their final output.  Note that passwords may not contain spaces.  For the softAP, the mode is fixed to AUTH_WPA_PSK.
Example session:
```
user@host:~$ telnet 192.168.1.197
Trying 192.168.1.197...
Connected to 192.168.1.197.
Escape character is '^]'.
+++AT MODE
MODE=3
OK
+++AT AP 
SSID=ESP_9E2EA6 PASSWORD= AUTHMODE=0 CHANNEL=3
OK
+++AT AP newSSID password
OK
+++AT AP
SSID=newSSID PASSWORD=password AUTHMODE=2 CHANNEL=3
OK
+++AT AP ESP_9E2EA6
OK
+++AT AP
SSID=ESP_9E2EA6 PASSWORD= AUTHMODE=0 CHANNEL=3
OK
^]c

telnet> c
Connection closed.
```
In order, this gets the current opmode. Good, it is 3 for STA + AP. Next, the current AP settings are retrieved. Next, the AP ssid is changed to newSSID and the authmode set to WPA and a password set. The AP settings are retrieved again to verify. Finally, the AP SSID is changed back to the original and by not using a password, the authmode is set to OPEN.

**Cons:** 

* Unbuffered TCP writes. Each incoming UART character gets sent as a separate TCP packet. This could potentially impact performance, however, in my hands that hasn't been an issue.


Parts of this firmware are from the stock AT firmware and the esphttpd project.
Enjoy.

Flash command, e.g. w/ locations:
```
/opt/Espressif/esptool-py/esptool.py --port /dev/tty.usbserial-A603HRFF write_flash 0x00000 eagle.app.v6.flash.bin 0x40000 eagle.app.v6.irom0text.bin
```
