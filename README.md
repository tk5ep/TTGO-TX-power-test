# TTGO-TX-power-test
A small software to be able to measure the output power of LoRa modules
Tested on Lilygo T3 and Lilygo T-beam modules.

# Why ?
It's difficult to measure the real power of a broad RF signal like used with LoRa APRS.
It's easier to do that with a continuous carrier (CW).
This software does produce a CW carrier at 2 different levels set to +20dBm and +10dBm during 10s.

# How ?
- Connect a Wattmeter (with a dummy load) to the antenna connector.
- Load the software on Visual Studio Code with PlatformIO compiler.
- Change to settings to fit your board. 
- Load the software
It will start immediately to send a carrier at +20dBm during 10 s on 433.775 MHz. These are the default values that can be changed

# Settings
----

  //#define tbeam
  #define T3
  // comment out if using SSD1306 driver. 1.3" inch uses SH11106
  #define ssd1306;  // defines the OLED driver type
  
  int TXdelay   = 10000;      // TX delay in ms at each power
  double TXfreq = 433.775;    // frequency in MHz
  int TXpowerHi = 20;
  int TXpowerLo = 10;
