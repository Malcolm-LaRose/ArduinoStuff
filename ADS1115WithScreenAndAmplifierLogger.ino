#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS 10
#define TFT_RST 9
#define TFT_DC 8

#define EIGENWHITE 0xFFBB
#define EIGENGRAU 0x10A3
#define CREAM 0xFD4F
#define PINK 0xFB53

Adafruit_ADS1115 ads1115;
Adafruit_ST7789 tft(TFT_CS,TFT_DC,TFT_RST);

adsGain_t ampSignalPinGain = GAIN_ONE;

unsigned long nextRead = 0;
unsigned long nextDisplay = 0;
const unsigned long readInterval = 2000; // Microseconds
const unsigned long displayInterval = 20000; // Microseconds
float countsToVoltsConversionFactor = 1.0; // Set by choice of gain

void configureRawSignalPin() {
  ads1115.setGain(GAIN_SIXTEEN);
  ads1115.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, true);
  countsToVoltsConversionFactor = 0.0000078125;
}

void configureAmpSignalPin() {
  ads1115.setGain(GAIN_ONE);
  ads1115.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, true);
  countsToVoltsConversionFactor = 0.125;
}

// We will want the loop to switch between these two pins for measurement
// Single shot might be better for that purpose, but precision timing is more difficult
// Data should be shoveled down the serial bus at full speed but can be displayed on the screen at a more sedate pace

void setup() {
  Serial.begin(1000000);
  Wire.begin();

  tft.init(170,320);
  tft.setRotation(3);
  tft.fillScreen(EIGENGRAU);
  tft.setTextWrap(true);

  if (!ads1115.begin(0x48)) {
    tft.setTextColor(ST77XX_RED, EIGENGRAU);
    tft.setTextSize(2);
    tft.setCursor(20, 65);
    tft.println("ADS1115 not found!");

    while (true) {
      delay(10);
    }
  }

  ads1115.setDataRate(RATE_ADS1115_860SPS);
  configureRawSignalPin();
  nextRead = micros() + readInterval;

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 15);
  tft.print("ADS1115 "); 
  tft.print(ads1115.getFsRange(),3); 
  tft.print("V\n 16bit ");
  tft.print(ads1115.getFsRange()/.32768,2);
  tft.println(" uV/count");
  tft.setTextWrap(false);
}

void loop() {
  unsigned long now = micros();
  static int16_t previousRaw = 0;
  static int16_t raw = 1;

  // Part of loop that reads the ADC and writes data to the computer (or SD in the future)
  if ((long)(now - nextRead) >= 0) {
    nextRead += readInterval;

    raw = ads1115.getLastConversionResults();

    Serial.write((uint8_t*)&raw, 2);
  }

  // Part of loop that prints to screen
  if ((long)(now - nextDisplay) >= 0) {
    nextDisplay += displayInterval;
    if (raw != previousRaw) {
      previousRaw = raw;
      float volts = (float)(raw*countsToVoltsConversionFactor);

      tft.setTextColor(EIGENWHITE, EIGENGRAU);
      tft.setTextSize(3);
      tft.setCursor(20, 65);
      tft.print(volts, 5);
      tft.print(" V");
    }
  }
}
