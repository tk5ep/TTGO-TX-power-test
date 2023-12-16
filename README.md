# TTGO-TX-power-test
A small software to be able to measure the output power of LoRa modules
Tested on Lilygo T3 and Lilygo T-beam modules.

# Why ?
It's difficult to measure the real power of a broad RF signal like used with LoRa APRS.
It's easier to do that with a continuous carrier (CW).
This software does produce a CW carrier at 2 different levels set to +20dBm and +10dBm during 10s.

# Usage
- Connect a Wattmeter (with a dummy load) to the antenna connector.
- Load the software on Visual Studio Code with PlatformIO compiler.
- Change to settings to fit your need
- Load the software
It will start immediately to send a carrier at +20dBm
