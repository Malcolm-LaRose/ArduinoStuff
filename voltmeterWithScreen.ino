#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS   10
#define TFT_DC    8
#define TFT_RST   9

Adafruit_ADS1115 ads;
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Set up the display first, so it can report an ADC problem
  tft.init(170, 320);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 15);
  tft.println("ADS1115 0.256V 16bit Voltmeter");
  tft.setTextWrap(false);

  if (!ads.begin(0x48)) {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 65);
    tft.println("ADS1115 not found!");

    while (true) {
      delay(10);
    }
  }

  // +/-0.256 V range; 7.8125 uV per count
  ads.setGain(GAIN_SIXTEEN);
}

void loop() {
  static int16_t previousRaw = -32768;

  int16_t raw = ads.readADC_SingleEnded(0);

  // Do not redraw the screen unless the ADC reading changed
  if (raw == previousRaw) {
    delay(25);
    return;
  }

  previousRaw = raw;

  float volts = ads.computeVolts(raw);

  Serial.print("A0: ");
  Serial.print(volts, 5);
  Serial.println(" V");

  // Black background erases the old text without clearing the screen.
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 65);
  tft.print(volts, 5);
  tft.print(" V");

  delay(25);
}