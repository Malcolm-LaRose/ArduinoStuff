# ArduinoStuff

Simple measurements of voltage from an Adafruit ADS1115 breakout using an Arduino Uno R3.

jul7a is simplest best in my opinion --> use corresponding ino

jul18 high speed test is fastest and similarly simple

timestamps for checking timing precision (should be 2ms +- 4us) --> use jul7b

liveplot for fourier fun --> use jul7b

livetest for the same with multithreading --> use jul7b

Highest observed stable timing ~555Hz with jul7a pipeline

Highest capable stable timing from jul18 high speed test (860SPS on ADS1115)



Future stuff:

ino and python for max speed ads1115 reading and timing

measure stability of direct adc 860SPS output

consider using higher speed arduino timer

similar stuff for the 24bit adc

write tft screen library for my purposes --> Which includes graphical elements that scale appropriately with each other and can be placed modularly on screen with function calls

better names
