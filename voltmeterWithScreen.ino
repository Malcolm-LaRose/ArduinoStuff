#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS   10
#define TFT_RST   9
#define TFT_DC    8

#define EIGENWHITE 0xFFBB
#define CREAM 0xFD4F
#define PINK 0xFB53

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
  ads.setDataRate(32); // Lower sampling rate reduces noise significantly - don't go below 20/s for gradifrac purposes 32/64/128 are good

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 15);
  tft.print("ADS1115 "); 
  tft.print(ads.getFsRange(),3); 
  tft.print("V\n 16bit ");
  tft.print(ads.getFsRange()/.32768,2);
  tft.println(" uV/count");
  tft.setTextWrap(false);

  // Serial.print(ads.getFsRange(),3);
  // Serial.println(" Full scall range");
}

void drawSpinner(byte frame) {
  const int8_t xOffset[8] = {0, 4, 6, 4, 0, -4, -6, -4};
  const int8_t yOffset[8] = {-6, -4, 0, 4, 6, 4, 0, -4};

  const int16_t centerX = 300;
  const int16_t centerY = 18;

  tft.fillRect(290, 8, 21, 21, ST77XX_BLACK);

  for (byte dot = 0; dot < 8; dot++) {
    int16_t x = centerX + xOffset[dot];
    int16_t y = centerY + yOffset[dot];

    // How far this dot trails behind the moving head.
    byte trailPosition = (frame + 8 - dot) % 8;

    if (trailPosition == 0) {
      // Head: radius 2
      tft.fillCircle(x, y, 2, PINK);

    } else {
      // Remaining dots: single pixels
      tft.drawPixel(x, y, EIGENWHITE);
    }
  }
}

void loop() {
  static int16_t previousRaw = 0;
  static byte spinnerFrame = 0;

  int16_t raw = ads.readADC_SingleEnded(1);

  // Update the voltage only when its reading changes.
  if (raw != previousRaw) {
    previousRaw = raw;

    float volts = ads.computeVolts(raw);

   // Serial.print("A0: ");
   // Serial.print(volts, 5);
   // Serial.println(" V");

    tft.setTextColor(EIGENWHITE, ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(20, 65);
    tft.print(volts, 5);
    tft.print(" V");
    
  }

  // Update the indicator every frame, even with an unchanged voltage.
  drawSpinner(spinnerFrame);
  spinnerFrame = (spinnerFrame + 1) % 8;

  delay(20);
}