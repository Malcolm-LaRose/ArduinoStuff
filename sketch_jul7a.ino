#include <Adafruit_ADS1X15.h>
#include <Wire.h>

unsigned long nextRead = 0;
const int readInterval = 1800; // microseconds (1163 theoretical minimum for this board, practical ~1800)

// If you notice the calculated sampling rate =/= programmed sampling rate, slow down or investigate bottleneck
// Bottleneck probably ADC or python, not really sure

Adafruit_ADS1115 ads1115;

void setup() {
  Serial.begin(1000000); // Trying this speed, previous stable was 500000
  ads1115.begin();
  ads1115.setGain(GAIN_SIXTEEN);
  ads1115.setDataRate(RATE_ADS1115_860SPS);
  Wire.setClock(400000); // Try increasing this later
  nextRead = micros() + readInterval; // micros() has minimum 4us resolution, use absolute timing for further precision but this is stable and avg to readInterval over large times
}

void loop() {
  unsigned long now = micros();

  if ((long)(now - nextRead) >= 0) {
     nextRead += readInterval;

    int16_t adc = ads1115.readADC_SingleEnded(0);

    Serial.write((uint8_t*)&adc, 2);
  }
}