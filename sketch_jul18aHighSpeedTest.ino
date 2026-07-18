#include <Adafruit_ADS1X15.h>
#include <Wire.h>

unsigned long nextRead = 0;
const unsigned long readInterval = 1800;

Adafruit_ADS1115 ads1115;

void setup() {

  Serial.begin(1000000);

  Wire.begin();
  Wire.setClock(400000);

  ads1115.begin();

  ads1115.setGain(GAIN_SIXTEEN);
  ads1115.setDataRate(RATE_ADS1115_860SPS);

  // Start continuous conversion mode
  ads1115.startADCReading(
      ADS1X15_REG_CONFIG_MUX_SINGLE_0,
      true
  );

  nextRead = micros() + readInterval;
}


void loop() {

  unsigned long now = micros();

  if ((long)(now - nextRead) >= 0) {

    nextRead += readInterval;

    int16_t adc = ads1115.getLastConversionResults();

    Serial.write((uint8_t*)&adc, 2);
  }
}