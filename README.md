ESP8266-transparent-bridge
==========================

I've noticed the jeelabs/esp-link fails in most new projects for wireless uploading to AVRs.  From scoping the problem, I suspect the GPIO reset time is not correct for most of the RESET cap setups.
~~## Note, this firmware is largely unsupported.  For new projects, I would recommend this firmware: [https://github.com/jeelabs/esp-link](https://github.com/jeelabs/esp-link).  It also supports a transparent bridge, but has additional features like a configuration web server, uController programming ports, and additional features for the esp12 class boards.~~

[![Join the chat at https://gitter.im/beckdac/ESP8266-transparent-bridge](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/beckdac/ESP8266-transparent-bridge?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

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
+++AT AP <ssid> <pw> [<authmode> [hide-ssid [<ch>]]]]
                                         # set the AP ssid and password, authmode:1= WEP,2= WPA,3= WPA2,4= WPA+WPA2, 
                                         # hide-ssid:1-hide, 0-show(not hide), channel: 1..13
+++AT BAUD                               # print current UART settings
+++AT BAUD <baud> [data [parity [stop]]] # set current UART baud rate and optional data bits = 5/6/7/8 , parity = N/E/O, stop bits = 1/1.5/2
+++AT PORT                               # print current incoming TCP socket port
+++AT PORT <port>                        # set current incoming TCP socket port (restarts ESP)
+++AT FLASH                              # print current flash settings
+++AT FLASH <1|0>                        # 1: The changed UART settings (++AT BAUD ...) are saved ( Default after boot), 0= no save to flash.
+++AT RESET                              # software reset the unit
+++AT GPIO2 <0|1|2 100>                  # 1: pull GPIO2 pin up (HIGH) 0: pull GPIO2 pin down (LOW) 2: reset GPIO2, where 100 is optional to specify reset delay time in ms (default 100ms)
+++AT SHOWIP							 # Display Station IP Address, gateway and netmask
+++AT SHOWMAC							 # Display Station MAC.
+++AT SCAN								 # Display available networks around

Upon success, all commands send back "OK" as their final output.  Note that passwords may not contain spaces.

The settings are saved after the commands
+++AT PORT <port>
+++AT BAUD <baud> ...

After +++AT FLASH 0 the parameter of command +++AT BAUD <baud> ... are  NOT saved to the flash memory.
The new settings are applied to the UART and saved only in RAM.
But a following +++AT PORT <port>  need to flash the settings for the necessary reboot. Then also the changed UART setting are saved to flash.

The disable of flash the settings is for devices with baud rate changes to avoid permanently flash of the setting sector.
Some electric meter start conversion with 300 baud and accept a command to change to 9600.

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

* limited buffered TCP writes. The first buffer is the UART FIFO. The second buffer is to collect new uart chars until the previous packet is sent.
From SDK 0.9.4 the next espconn_sent must after espconn_sent_callback of the pre-packet.
All incoming UART characters in the FIFO gets sent immediately via the tx-buffer. The resulting TCP packet has only some bytes.

This could potentially impact performance, however, in my hands that hasn't been an issue.


Parts of this firmware are from the stock AT firmware and the esphttpd project.
Enjoy.

Flash command, e.g. w/ locations:
```
/opt/Espressif/esptool-py/esptool.py --port /dev/tty.usbserial-A603HRFF write_flash 0x00000 0x00000.bin 0x40000 0x40000.bin
...
or use ESP8266Flasher.exe from https://github.com/nodemcu/nodemcu-flasher with
0x00000.bin at 0x00000
0x40000.bin at 0x40000
```

#Visual Studio 2013 Integration
see "New Windows terminal/flasher apps & Visual Studio" http://www.esp8266.com/viewtopic.php?f=9&t=911#p5113 to setup Visual Studio 2013
Please install in a folder i.e. c:\Projects\Espressif\
```
ESP8266-transparent-bridge/              #this project
esp_iot_sdk_v0.9.5/                      #http://bbs.espressif.com/download/file.php?id=189
xtensa-lx106-elf/                        #pre-built compiler, see http://www.esp8266.com/viewtopic.php?f=9&t=911#p5113 ,
                                         #I used xtensa-lx106-elf-141114.7z from  https://drive.google.com/uc?export=download&confirm=XHSI&id=0BzWyTGWIwcYQallNcTlxek1qNTQ
esptool-py.py                            #http://www.esp8266.com/download/file.php?id=321
```

The files used by Visual Studio are:
```
ESP8266-transparent-bridge.sln           #solution file
ESP8266-transparent-bridge.vcxproj       #project file, with IncludePath to xtensa-lx106-elf, sdk for intellisense and "Go To Definition" F12
espmake.cmd                              #batch file called by build, rebuild, clean command, which set the path and call make with Makefile_VS2013
Makefile_VS2013                          #the makefile called by the Visual Studio NMake project via espmake.cmd
```
The Debug config is used for compile, Release for compile & flash with esptool-py.py  

The following absolute path names and COM Port number are expected:
```
C:\MinGW\bin;C:\MinGW\msys\1.0\bin       in espmake.cmd
C:\Python27\python                       in Makefile_VS2013 for flash
COM5                                     in Makefile_VS2013 for flash
```
