#include <Adafruit_ADS1X15.h>
#include <Wire.h>

unsigned long nextRead = 0;
const unsigned long readInterval = 2000; // microseconds (2 ms)

Adafruit_ADS1115 ads1115;

void setup() {

  Serial.begin(500000);

  ads1115.begin();
  ads1115.setGain(GAIN_SIXTEEN);
  ads1115.setDataRate(RATE_ADS1115_860SPS);

  Wire.setClock(400000);
  nextRead = micros() + readInterval;
}


void loop() {

  unsigned long now = micros();

  if ((long)(now - nextRead) >= 0) {

    nextRead += readInterval;

    int16_t adc = ads1115.readADC_SingleEnded(0);

    // Send timestamp (4 bytes)
    Serial.write((uint8_t*)&now, 4);

    // Send ADC value (2 bytes)
    Serial.write((uint8_t*)&adc, 2);
  }
}