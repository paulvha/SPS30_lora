# BME280 and Sensirion SPS30 connecting to TTN & Luftdaten

## ===========================================================

A program to set instructions and get information from an SPS30 and BME280
and sent to TheThingsNetwerk (TTN).
It has been tested to run with I2C communcation on Feather Lora 32U4.

## Getting Started
As part of a larger project I am looking at analyzing and understanding the air quality.
I have done a number of projects on air-sensors. The SPS30 sensor is a new kid on the block
that looks interesting. This is the first version of a working driver + examples.
More work is happening to create examples and compare against other sensors.

This first is a cut down version of 1.3.6 for Arduino Feather LORA.

For detailed information, please read the feather_lora.odt in the extras folder

## Prerequisites
LMIC  : https://github.com/matthijskooijman/arduino-lmic

If you plan to use the standard library:
<br> BME   : Adafruit_BME280 and Adafruit_Sensor
<br> SPS30 : sps (https://github.com/paulvha/sps30)

## Software installation
Obtain the zip and install like any other

## Program usage
### Program options
Please see the description in the top of the sketch and read the documentation (odt)

## Versioning

### version lora / October 2019
 * Initial version optimized for Arduino Feather lora 32U4

### version Lora / 1.0.2 / November 2019
 * added forwarder between TTN and Luftdaten (see documentation in luftdaten folder)
 * forwarder tested on Ubuntu, Raspberry Pi and Windows 10
 * Forwarder can also save the data in a file in local directory or share
 * small change in the sketch in the data order

## Author
 * Paul van Haastrecht (paulvha@hotmail.com)

## License
This project is licensed under the GNU GENERAL PUBLIC LICENSE 3.0

## Acknowledgements
Make sure to read the datasheet from Sensirion. While draft it does provide good starting point.

