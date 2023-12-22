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
// If LilyGo T-beam board. Comment out for this board
//#define Lilygo_tbeam
// If LilyGo T3 board. Comment out for this board
#define Lilygo_T3
// defines the OLED driver type comment out if using SSD1306 driver. Comment if 1.3" inch uses SH11106
#define ssd1306
// TX delay in ms at each power
int TXdelay   = 2000;
// TX frequency in MHz
double TXfreq = 433.775;
// audio start frequency in Hz
int TXtonebase = 1000;
// audio step in Hz
int TXtonestep = 100;
// array in which all desired power levels are given in dBm
const int PowerArray [] = {2,4,6,8,10,17,20};

/*****************************************
  ____             __ _       
 / ___|___  _ __  / _(_) __ _ 
| |   / _ \| '_ \| |_| |/ _` |
| |__| (_) | | | |  _| | (_| |
 \____\___/|_| |_|_| |_|\__, |
                        |___/ 
*****************************************/
// include the libraries
#include <Arduino.h>
#include <RadioLib.h>
#ifdef ssd1306
  #include <Adafruit_SSD1306.h>
#else
  #include <Adafruit_SH110X.h>
#endif

String SOFTWARE_DATE = "22.12.23";

// PINs mapping
// Lilygo modules have same pining, given for example if other modules 
// Lilygo T-beam
#ifdef Lilygo_tbeam
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
#ifdef Lilygo_T3
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
#ifdef ssd1306
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#else
  Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
#endif

// init radio instance
SX1278 radio = new Module(LORA_SS, LORA_DIO0, LORA_RST, LORA_DIO1);

// create AFSK client instance using the FSK module
// this requires connection to the module direct
// input pin, here connected to Arduino pin 5
// SX127x/RFM9x:  DIO2
AFSKClient audio(&radio,32);

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

//ledcAttachPin(LORA_DIO1, 0);

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
  line2 = "Sends FM carrier at";
  line3 = "different levels";
  line4 = "";
  line5 = "by TK5EP v" + SOFTWARE_DATE;
  display.setCursor(0, 0);
  display.print(line1);
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(line2);
  display.setCursor(0,30);
  display.print(line3);
  display.setCursor(0, 56);
  display.print(line5);
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
    line2="failed code " + String(state) ;
    line3="Stopping execution";
    display.setCursor(0, 10);
    display.print(line2);
    display.setCursor(0, 30);
    display.print(line3);
    while(true);
  }
      display.display();

  radio.setCurrentLimit(150);           // set current limiter to at least 150mA
  //radio.setFrequencyDeviation(10);     // set FM deviation
  state = radio.setFrequency(TXfreq);   // set the frequency
    if(state == RADIOLIB_ERR_NONE) {
    Serial.println(String(TXfreq,3) + " frequency setting success!");
  } else {
    Serial.print(F("Frequency setting failed, code "));
    Serial.println(state);
    while(true);
  }

  // initialize AFSK client
  Serial.print(F("[AFSK] Initializing ... "));
  display.setCursor(0,30);
  line3="AFSK initializing :";
  display.print(line3);
  display.display();
  state = audio.begin();
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    line4="success";
    display.setCursor(0, 40);
    display.print(line4);
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    line4="failed " + String(state);
    display.setCursor(0, 40);
    display.print(line4);
    while(true);
  }
  display.display();

  state = audio.begin();
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println(F("Audio begin success!"));
  } else {
    Serial.print(F("Audio begin failed, code "));
    Serial.println(state);
    //while(true);
  }

  delay(3000);
  display.clearDisplay();

} // END setup()

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
  int TXtone = TXtonebase;

  for (int TXpower : PowerArray) { // for each element in the array
  display.clearDisplay();
  Serial.println("Generating " + String(TXtone) + "Hz tone ... ");

  audio.tone(TXtone);

  int state = radio.setOutputPower(TXpower);
  if(state == RADIOLIB_ERR_NONE) {
    Serial.println("+" +String(TXpower) + "dBm power setting success!");
  } else {
    Serial.print(F("Power setting failed, code "));
    Serial.println(state);
    //while(true);
  }
  // convert dBm to mW
  double TXWatts = pow(10.0, (TXpower - 30.0) / 10.0) * 1000;

radio.transmitDirect();

  Serial.println("Transmitting +" + String(TXpower) +"dBm FM carrier ...\n ");
  line1 = "TXing FM carrier";
  line2 = "+" + String(TXpower) +" dBm";
  line3 = String(TXWatts) + " mW";  
  display.setTextSize(1);
  display.setCursor(0,10);
  display.print(line1);
  display.setTextSize(2);
  display.setCursor(0,30);
  display.print(line2);
  display.setCursor(0,50);
  display.print(line3);
  display.display();

  delay(TXdelay);
  
  // stop transmitting to be able to change power setting
  radio.standby();
  TXtone = TXtone + TXtonestep;
} // END for loop
} // END loop()
