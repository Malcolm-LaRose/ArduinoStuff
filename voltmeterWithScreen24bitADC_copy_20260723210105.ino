#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS122C04.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS   10
#define TFT_RST   9
#define TFT_DC    8

#define EIGENWHITE 0xFFBB
#define CREAM 0xFD4F
#define PINK 0xFB53

const float GAUGE_MIN_VOLTS = 0.000;
const float GAUGE_MAX_VOLTS = 0.256;
const byte GAUGE_STEPS = 8;
bool printGauge = true;

Adafruit_ADS122C04 ads;
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);


void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setWireTimeout(25000,true);

  // Set up the display first, so it can report an ADC problem
  tft.init(170, 320);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 15);
  tft.println("ADS122C04 0.256V\n 24bit Voltmeter");
  tft.setTextWrap(false);

  if (!ads.begin()) {
    tft.setTextColor(ST77XX_RED, ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 65);
    tft.println("ADS122C04 not found!");

    while (true) {
      delay(10);
    }
  }

  ads.setContinuousMode(true);
  ads.setMux(ADS122C04_MUX_AIN3);
  // +/-0.256 V range; 30.5 nV per count
  ads.setGain(ADS122C04_GAIN_8);
  ads.setDataRate(ADS122C04_RATE_90SPS); // Lower sampling rate reduces noise significantly - don't go below 20/s for gradifrac purposes
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

    } else if (trailPosition <= 3) {
      // Three following dots: radius 1
      tft.fillCircle(x, y, 1, CREAM);

    } else {
      // Remaining dots: single pixels
      tft.drawPixel(x, y, EIGENWHITE);
    }
  }
}

void loop() {
  static byte spinnerFrame = 0;

  ads.startSync();

  while (!ads.isDataReady()) {
      delay(1);
    }

  int32_t raw = ads.readData();

  double volts = ads.convertToVoltage(raw);

  //Serial.print("A3: ");
  //Serial.print(volts, 7);
  //Serial.println(" V");

  tft.setTextColor(EIGENWHITE, ST77XX_BLACK);
  tft.setTextSize(3);
  tft.setCursor(20, 65);
  tft.print(volts, 7);
  tft.print(" V");

  drawSpinner(spinnerFrame);
  spinnerFrame = (spinnerFrame + 1) % 8;

  delay(20);
}