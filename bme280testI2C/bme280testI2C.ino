/*
 * Adapatation done by D. Ray for BME280 on ESP8266
 * Connected to
 * SDA GPIO0
 * SCL GPIO2
 * Modification done directly in the Librairy
 */

/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
// #include <SPI.h> // Remove not used for I2C
#include <Adafruit_Sensor.h>
#include <DR_BME280.h> // Modification on Adafruit_BME280.h with SDA on GPIO0 & SCL GPIO2

#define BME_SCK 13  // Only for SPI
#define BME_MISO 12 // Only for SPI
#define BME_MOSI 11 // Only for SPI
#define BME_CS 10 // Only for SPI

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;
// I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 test with I2 on ESP8266"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");

    Serial.println();
    delay(2000);
}
