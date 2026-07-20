#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS   10
#define TFT_RST   9
#define TFT_DC    8

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
  tft.println("ADS1115 0.256V\n 16bit Voltmeter");
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

void drawSpinner(byte frame) {
  const int8_t xOffset[8] = {0, 4, 6, 4, 0, -4, -6, -4};
  const int8_t yOffset[8] = {-6, -4, 0, 4, 6, 4, 0, -4};

  const int16_t centerX = 300;
  const int16_t centerY = 18;

  // Erase only the tiny spinner area.
  tft.fillRect(290, 8, 21, 21, ST77XX_BLACK);

  // A tiny eight-dot rotating "loading" sparkle.
  for (byte dot = 0; dot < 8; dot++) {
    uint16_t color = (dot == frame) ? ST77XX_YELLOW : ST77XX_BLUE;

    tft.fillCircle(
      centerX + xOffset[dot],
      centerY + yOffset[dot],
      (dot == frame) ? 2 : 1,
      color
    );
  }
}

void loop() {
  static int16_t previousRaw = 0;
  static byte spinnerFrame = 0;

  int16_t raw = ads.readADC_SingleEnded(0);

  // Update the voltage only when its reading changes.
  if (raw != previousRaw) {
    previousRaw = raw;

    float volts = ads.computeVolts(raw);

    Serial.print("A0: ");
    Serial.print(volts, 5);
    Serial.println(" V");

    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(20, 65);
    tft.print(volts, 5);
    tft.print(" V");
  }

  // Update the indicator every frame, even with an unchanged voltage.
  drawSpinner(spinnerFrame);
  spinnerFrame = (spinnerFrame + 1) % 8;

  delay(17);
}