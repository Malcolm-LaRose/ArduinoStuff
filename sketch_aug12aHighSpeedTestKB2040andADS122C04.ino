#include <Wire.h>
#include <SPI.h>

#include <Adafruit_NeoPixel.h>

#include <Adafruit_ADS122C04.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// KB2040 pretty pixel control
#define NEOPIXEL_PIN 17

// TFT control pins
#define TFT_CS   A0
#define TFT_DC   A1
#define TFT_RST  D10

// ADC control pins
#define ADC_SDA A2
#define ADC_SCL A3

#define EIGENWHITE 0xFFBB

Adafruit_NeoPixel pixel(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_ADS122C04 ads;

Adafruit_ST7789 tft = Adafruit_ST7789(
  TFT_CS,
  TFT_DC,
  TFT_RST
);


// ============================================================
// SAMPLING + TIMING
// ============================================================

unsigned long nextRead = 0;
unsigned long rainbowTime = 0;
unsigned long drawTimer = 0;

const unsigned int drawEveryXFrames = 200;

// Current position in the rainbow.
// 0-255 gives one complete rainbow cycle.
byte rainbowPosition = 0;

// 1163 us corresponds to ~860 samples/sec
const unsigned long readInterval = 1000; // us, 1000 samples/sec




void setup() {

  pixel.begin();
  pixel.setBrightness(77);

  Serial.begin(1000000);

  // ----------------------------------------------------------
  // I2C
  // ----------------------------------------------------------

  Wire.setSDA(A2);
  Wire.setSCL(A3);
  Wire.begin();

  // 1 MHz I2C
  Wire.setClock(1000000);

  // ----------------------------------------------------------
  // TFT
  // ----------------------------------------------------------

  tft.init(170, 320);

  tft.setRotation(1);

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
  ads.setTurboMode(false);  // Doubles sampling rate
  ads.setMux(ADS122C04_MUX_AIN1_AIN0); 
  ads.setGain(ADS122C04_GAIN_8); // +/-0.256 V range; 30.5 nV per count
  ads.setDataRate(ADS122C04_RATE_1000SPS); // Lower sampling rate reduces noise significantly - don't go below 50SPS for gradifrac purposes (20ms precision) - Higher speeds have lower precision

  ads.startSync();

  nextRead = micros() + readInterval;
}

uint32_t colorWheel(byte pos) {
  pos = 255 - pos;

  if (pos < 85) {
    return pixel.Color(255 - pos * 3, 0, pos * 3);
  }
  else if (pos < 170) {
    pos -= 85;
    return pixel.Color(0, pos * 3, 255 - pos * 3);
  }
  else {
    pos -= 170;
    return pixel.Color(pos * 3, 255 - pos * 3, 0);
  }
}

void rainbowStep() {

  // Set the LED to the current rainbow position
  pixel.setPixelColor(
    0,
    colorWheel(rainbowPosition)
  );

  pixel.show();

  // Advance exactly ONE step.
  rainbowPosition++;
}



float convertToVoltage(int32_t rawData) {
  static const float effectiveGain = ads.getEffectiveGain();
  static constexpr float refVoltage = 2.048; // Internal reference, consider external for more precision(?)
  
  float voltage = (float)rawData * refVoltage / (effectiveGain * 8388608.0f);

  return voltage;

} // Need to compare the timing of this vs. the onboard version of this function

void loop() {

  int32_t raw = ads.readData(); // Needs at least 24 bits of precision to meet or exceed ADC precision

  float volts = convertToVoltage(raw); // Float has 23 bits of precision in the mantissa, perfect for signed 24 bit data

  // Precision data transfer, hopefully
  unsigned long now = micros();

  if ((long)(now - nextRead) >= 0) {

    nextRead += readInterval;

    Serial.write((uint8_t*)&raw, 4);
    //Serial.println(2.048000000 / (ads.getEffectiveGain() * 8388608.0f),24);

  }

  
  // Printing to screen goes here
  if ((long)(drawTimer % drawEveryXFrames) == 0) {

    tft.setTextColor(EIGENWHITE, ST77XX_BLACK);
    tft.setTextSize(3);
    tft.setCursor(20, 65);
    tft.print(volts, 7);
    tft.print("  V");

    rainbowStep();

  }

  drawTimer++;
 
}