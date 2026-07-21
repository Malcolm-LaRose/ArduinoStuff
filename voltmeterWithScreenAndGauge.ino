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

const float GAUGE_MIN_VOLTS = 0.000;
const float GAUGE_MAX_VOLTS = 0.256;
const byte GAUGE_STEPS = 8;
bool printGauge = true;

Adafruit_ADS1115 ads;
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void drawGaugeFrame() {
  const int16_t gaugeWidth = 4;
  const int16_t gaugeY = 38;
  const int16_t gaugeHeight = tft.height() - gaugeY - 12;
  const int16_t gaugeX = tft.width() - gaugeWidth - 8;

  // Labels move left so the right-side arrow cannot overwrite them.
  const int16_t labelX = gaugeX - 34;

  tft.setTextColor(EIGENWHITE, ST77XX_BLACK);
  tft.setTextSize(1);

  tft.setCursor(labelX, gaugeY - 7);
  tft.print("0.256");

  tft.setCursor(labelX, gaugeY + gaugeHeight);
  tft.print("0.000");
}

void drawGauge(float volts) {
  const int16_t gaugeWidth = 4;
  const int16_t gaugeY = 38;
  const int16_t gaugeHeight = tft.height() - gaugeY - 12;
  const int16_t gaugeX = tft.width() - gaugeWidth - 8;

  float fraction = (volts - GAUGE_MIN_VOLTS) /
                   (GAUGE_MAX_VOLTS - GAUGE_MIN_VOLTS);

  if (fraction < 0.0) fraction = 0.0;
  if (fraction > 1.0) fraction = 1.0;

  int16_t pointerY = gaugeY + gaugeHeight - 2 -
                     (int16_t)(fraction * (gaugeHeight - 3));

  // Clear the bar and its right-side arrow.
  tft.fillRect(
    gaugeX, gaugeY - 2,
    gaugeWidth + 9, gaugeHeight + 4,
    ST77XX_BLACK
  );

  const int16_t innerHeight = gaugeHeight - 2;
  const int16_t blockHeight =
    (innerHeight - (GAUGE_STEPS - 1)) / GAUGE_STEPS;
  const int16_t innerBottom = gaugeY + gaugeHeight - 2;

  for (byte step = 0; step < GAUGE_STEPS; step++) {
  int16_t blockY = innerBottom - (step + 1) * blockHeight - step + 1;
  int16_t blockBottom = blockY + blockHeight;

  // Fill each segment only up to the arrow's exact Y position.
  int16_t fillStart = pointerY > blockY ? pointerY : blockY;

  if (fillStart < blockBottom) {
    tft.fillRect(
      gaugeX + 1,
      fillStart,
      gaugeWidth - 2,
      blockBottom - fillStart,
      CREAM
    );
  }

  tft.drawRect(gaugeX, blockY - 1, gaugeWidth, blockHeight + 2, EIGENWHITE);
}

  // Solid red arrow: point touches the right edge of the scale and faces left.
  tft.fillTriangle(
    gaugeX + gaugeWidth - 1, pointerY,
    gaugeX + gaugeWidth + 7, pointerY - 3,
    gaugeX + gaugeWidth + 7, pointerY + 3,
    ST77XX_RED
  );
}

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

  drawGaugeFrame();

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
  ads.setDataRate(64); // Lower sampling rate reduces noise significantly - don't go below 20/s for gradifrac purposes
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
  static int16_t previousRaw = 0;
  static byte spinnerFrame = 0;

  int16_t raw = ads.readADC_SingleEnded(0);

  // Update the voltage only when its reading changes.
  if (raw != previousRaw) {
    previousRaw = raw;

    float volts = ads.computeVolts(raw);

    //Serial.print("A0: ");
    //Serial.print(volts, 5);
    //Serial.println(" V");

    tft.setTextColor(EIGENWHITE, ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(20, 65);
    tft.print(volts, 5);
    tft.print(" V");
    if (printGauge == true) {
    drawGauge(volts);
    printGauge = !printGauge;
    }
    else {
    printGauge = !printGauge;
    }
  }

  // Update the indicator every frame, even with an unchanged voltage.
  drawSpinner(spinnerFrame);
  spinnerFrame = (spinnerFrame + 1) % 8;

  delay(16.67);
}