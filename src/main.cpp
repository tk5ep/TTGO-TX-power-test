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

// PINs mapping
// Lilygo modules have same pining, given for example if other modules 
// Lilygo T-beam
#ifdef tbeam
  // T-beam pins
  // 0.96" OLED with SSD1306. 1.3" with SSD110X
  //#define I2C_SDA         21
  //#define I2C_SCL         22
  #define OLED_RST -1       // shared pin. pin 16 crashes
  // LoRa radio
  #define LORA_SCK        5
  #define LORA_MISO       19
  #define LORA_MOSI       27
  #define LORA_SS         18
  #define LORA_DIO0       26
  #define LORA_DIO1       33  // pin LoRa1 on header not connected to IO pin
  //#define LORA_DIO2       32  // pin LoRa2 on header
  #define LORA_RST        23
#endif
// Lilygo T3
#ifdef T3
  // T3 pins
  // I2C OLED Display works with SSD1306 driver
  //#define OLED_SDA     21
  //#define OLED_SCL     22
  #define OLED_RST      -1
  // SPI LoRa Radio
  #define LORA_SCK      5
  #define LORA_MISO     19
  #define LORA_MOSI     27
  #define LORA_SS       18
  #define LORA_DIO0     26
  #define LORA_DIO1     33  // pin LoRa1 on header not connected to IO pin
  //#define LORA_DIO2     32  // pin LoRa2 on header
  #define LORA_RST      23
#endif

// OLED line variables
String line1 = "";
String line2 = "";
String line3 = "";
String line4 = "";
String line5 = "";
String line6 = "";

// DISPLAY SSD1306
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// instance for LoRa module
SX1278 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);
// instance for OLED
#ifdef ssd1306
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#else
  Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#endif

// instance using the FSK module
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
  // init OLED
  #ifdef ssd1306
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c))
  #else
    display.begin(0x3c, true); // Address 0x3C default
  #endif

  // OLED setting
  display.clearDisplay();
    #ifdef ssd1306
    display.setTextColor(WHITE);
  #else
    display.setTextColor(SH110X_WHITE);
  #endif
  // boot info page on OLED
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
  radio.setOutputPower(TXpowerHi);
  double WattsHi = pow( 10.0, (TXpowerHi - 30.0) / 10.0) * 1000;

  Serial.print(F("Transmitting a CW carrier ... "));
  line1 = "TXing CW carrier";
  line2 = "+" + String(TXpowerHi) +"dBm";
  line3 = String(WattsHi) + " mW";  
  radio.setOutputPower(TXpowerHi);
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(line1);
  display.setTextSize(2);
  display.setCursor(0,30);
  display.print(line2);
  display.setCursor(0,50);
  display.print(line3);
  display.display();

  rtty.idle();
  delay(TXdelay);
  rtty.standby();

  radio.setOutputPower(TXpowerLo);                                // set the output power
  double WattsLo = pow( 10.0, (TXpowerLo - 30.0) / 10.0) * 1000;  // dBm to mw conversion

  display.clearDisplay();
  line1 = "TXing CW carrier";
  line2 = "+" + String(TXpowerLo) + "dBm";
  line3 = String(WattsLo) + " mW";
  display.setCursor(0,10);
  display.setTextSize(1);
  display.print(line1);
  display.setCursor(0,30);
  display.setTextSize(2);
  display.print(line2);
  display.setCursor(0,50);
  display.print(line3);
  display.display();

  rtty.idle();              // transmitting
  delay(TXdelay);           // during this time
  //rtty.standby();         // turn the transmitter off
  display.clearDisplay();

  Serial.println(F("done!"));

  // wait a second 
  //delay(1000);
}
