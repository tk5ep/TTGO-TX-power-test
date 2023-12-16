/*
Small software to test the output power of LoRa modules like Lilygo T3 and T-beam
It transmits alternatively a carrier at 2 power levels (TXpowerHi & TXpowerLo) during a certain time (TXdelay) on frequency (TXfreq)

written by TK5EP dec 2023
https://egloff.eu
https://github.com/tk5ep

*/


/***************************************
 ____       _   _   _                 
/ ___|  ___| |_| |_(_)_ __   __ _ ___ 
\___ \ / _ \ __| __| | '_ \ / _` / __|
 ___) |  __/ |_| |_| | | | | (_| \__ \
|____/ \___|\__|\__|_|_| |_|\__, |___/
                            |___/ 
****************************************/
#define tbeam               // LilyGo T-beam board. Comment out for this board
//#define T3                 // LilyGo T3 board. Comment out for this board
//#define ssd1306            // defines the OLED driver type comment out if using SSD1306 driver. Comment if 1.3" inch uses SH11106
int TXdelay   = 10000;      // TX delay in ms at each power
double TXfreq = 433.775;    // frequency in MHz
int TXpowerHi = 20;         // High power level in dBm (max 20)
int TXpowerLo = 10;         // Low power level in dBm

/*****************************************
  ____             __ _       
 / ___|___  _ __  / _(_) __ _ 
| |   / _ \| '_ \| |_| |/ _` |
| |__| (_) | | | |  _| | (_| |
 \____\___/|_| |_|_| |_|\__, |
                        |___/ 
*****************************************/

#include <RadioLib.h>
#include <Wire.h>

String SOFTWARE_DATE = "16.12.23";

#ifdef ssd1306
  #include <Adafruit_SSD1306.h>
#else
  #include <Adafruit_SH110X.h>
#endif

#ifdef tbeam
  // T-beam pins
  #define I2C_SDA         21
  #define I2C_SCL         22
  #define OLED_RST -1       // shared pin. pin 16 crashes
  #define LORA_SCK        5
  #define LORA_MISO       19
  #define LORA_MOSI       27
  #define LORA_SS         18
  #define LORA_DIO0       26
  #define LORA_DIO1       33
  #define LORA_DIO2       32
  #define LORA_RST        23
#endif

#ifdef T3
  // I2C OLED Display works with SSD1306 driver
  //#define OLED_SDA 21
  //#define OLED_SCL 22
  #define OLED_RST -1

  // SPI LoRa Radio
  #define LORA_SCK 5        // GPIO5 - SX1276 SCK
  #define LORA_MISO 19     // GPIO19 - SX1276 MISO
  #define LORA_MOSI 27    // GPIO27 - SX1276 MOSI
  #define LORA_SS 18     // GPIO18 - SX1276 CS
  #define LORA_DIO0 26
  #define LORA_DIO1 33
  #define LORA_RST 14   // GPIO14 - SX1276 RST
  #define LORA_IRQ 26  // GPIO26 - SX1276 IRQ (interrupt request)
#endif

String line1 = "";
String line2 = "";
String line3 = "";
String line4 = "";
String line5 = "";
String line6 = "";

//SX1278 radio = new Module(10, 2, 9, 3);
SX1278 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);

// DISPLAY SSD1306

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

#ifdef ssd1306
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#else
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#endif

// create RTTY client instance using the FSK module
RTTYClient rtty(&radio);

/*****************************
 _____      _               
/  ___|    | |              
\ `--.  ___| |_ _   _ _ __  
 `--. \/ _ \ __| | | | '_ \ 
/\__/ /  __/ |_| |_| | |_) |
\____/ \___|\__|\__,_| .__/ 
                     | |    
                     |_|    
******************************/
void setup() {
  Serial.begin(115200);
  
  
  #ifdef ssd1306
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))
  #else
  display.begin(0x3c, true); // Address 0x3C default
  #endif

  // boot info page on OLED
  display.clearDisplay();
  
  #ifdef ssd1306
  display.setTextColor(WHITE);
  #else
  display.setTextColor(SH110X_WHITE);
  #endif

  display.setTextSize(2);
  display.clearDisplay();             // mandatory to remove Adafruit splash
  line1 = "TTGO TX";
  line2 = "RF power test";
  line3 = "@ +" + String(TXpowerHi) +"dBm & +" + String(TXpowerLo) +"dBm";
  line4 = "by TK5EP v" + SOFTWARE_DATE;
  display.setCursor(0, 0);
  display.print(line1);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(line2);

  
  display.setCursor(0,40);
  display.print(line3);
  display.setCursor(0, 56);
  
  display.print(line4);
  display.display();
  delay(5000);

  display.clearDisplay();
  //display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  // initialize SX1278 with default settings
  Serial.print(F("[SX1278] Initializing ... "));
  line1 = "SX1278 initializing :";
  display.print(line1);
  display.display();

  int state = radio.beginFSK();

  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    display.setCursor(0, 10);
    line2="success";
    display.print(line2);
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    display.setCursor(0, 10);
    line2="failed code " + state ;
        while(true);
  }

  display.display();

  //radio.setOutputPower(20);

  // initialize RTTY client
  Serial.print(F("[RTTY] Initializing ... "));
  
  display.setCursor(0,30);
  line3="RTTY initializing :";
  display.print(line3);
  display.display();

  state = rtty.begin(TXfreq, 850, 45);  // shift 850 Hz à 45 bauds (not used here)
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    line4="success";
    display.setCursor(0, 40);
    display.print(line4);
    display.display();
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    line4="failed " + state;
    display.setCursor(0, 40);
    display.print(line4);
    display.display();
    while(true);
  }
  delay(3000);

  display.clearDisplay();
}

/*****************************
 _                       
| |                      
| |     ___   ___  _ __  
| |    / _ \ / _ \| '_ \ 
| |___| (_) | (_) | |_) |
\_____/\___/ \___/| .__/ 
                  | |    
                  |_|    
*****************************/
void loop() {
  Serial.print(F("Transmitting a CW carrier ... "));
  line1 = "TXing CW carrier";
  line2 = "+" + String(TXpowerHi) +"dBm";
  radio.setOutputPower(TXpowerHi);
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(line1);
  display.setTextSize(2);
  display.setCursor(0,30);
  display.print(line2);
  display.display();

  rtty.idle();
  delay(TXdelay);
  rtty.standby();

  radio.setOutputPower(TXpowerLo);

  display.clearDisplay();
  line1 = "TXing CW carrier";
  line2 = "+" + String(TXpowerLo) + "dBm";
  display.setCursor(0,10);
  display.setTextSize(1);
  display.print(line1);
  display.setCursor(0,30);
  display.setTextSize(2);
  display.print(line2);
  display.display();

  rtty.idle();
  delay(TXdelay);
  rtty.standby();
  display.clearDisplay();

  // turn the transmitter off
  //rtty.standby();

  Serial.println(F("done!"));

  // wait a second
  delay(1000);
}

