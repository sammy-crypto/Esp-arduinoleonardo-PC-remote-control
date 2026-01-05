# Esp-arduinoleonardo-PC-remote-control
This device allows users to plug it into any target pc and remotely control it with any wifi capable device using HID mouse/keystroke emulation. This requires and ESP8266 and Arduino Leonardo. The ESP controller acts as a network access point, browsing to the default gateway/router address opens up the remote control UI. Enjoy!

Wiring:

PC - Leo - ESP rx - 1k - Leo rx + 2k - ESP GND
Leo rx - ESP tx
Leo 5V - ESP vin
Common gnd controllers
