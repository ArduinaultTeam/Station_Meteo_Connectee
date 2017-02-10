//#include <SPI.h>

/***************************************************************************
  Development done by David Ray

 Usage of the BME280 with the ESP 8266 and protocol I2C

 Feb 2017
 
 ***************************************************************************/

#include <Wire.h>

/* Connection information */
#define SDA_PIN 0
#define SCL_PIN 2
#define BME280_ADDRESS (0x76)

#define SEALEVELPRESSURE_HPA (1013.25)
/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
    enum
    {
      BME280_REGISTER_DIG_T1              = 0x88,
      BME280_REGISTER_DIG_T2              = 0x8A,
      BME280_REGISTER_DIG_T3              = 0x8C,

      BME280_REGISTER_DIG_P1              = 0x8E,
      BME280_REGISTER_DIG_P2              = 0x90,
      BME280_REGISTER_DIG_P3              = 0x92,
      BME280_REGISTER_DIG_P4              = 0x94,
      BME280_REGISTER_DIG_P5              = 0x96,
      BME280_REGISTER_DIG_P6              = 0x98,
      BME280_REGISTER_DIG_P7              = 0x9A,
      BME280_REGISTER_DIG_P8              = 0x9C,
      BME280_REGISTER_DIG_P9              = 0x9E,

      BME280_REGISTER_DIG_H1              = 0xA1,
      BME280_REGISTER_DIG_H2              = 0xE1,
      BME280_REGISTER_DIG_H3              = 0xE3,
      BME280_REGISTER_DIG_H4              = 0xE4,
      BME280_REGISTER_DIG_H5              = 0xE5,
      BME280_REGISTER_DIG_H6              = 0xE7,

      BME280_REGISTER_CHIPID             = 0xD0,
      BME280_REGISTER_VERSION            = 0xD1,
      BME280_REGISTER_SOFTRESET          = 0xE0,

      BME280_REGISTER_CAL26              = 0xE1,  // R calibration stored in 0xE1-0xF0

      BME280_REGISTER_CONTROLHUMID       = 0xF2,
      BME280_REGISTER_CONTROL            = 0xF4,
      BME280_REGISTER_CONFIG             = 0xF5,
      BME280_REGISTER_PRESSUREDATA       = 0xF7,
      BME280_REGISTER_TEMPDATA           = 0xFA,
      BME280_REGISTER_HUMIDDATA          = 0xFD,
    };

/*=========================================================================*/

/*=========================================================================
    CALIBRATION DATA
    -----------------------------------------------------------------------*/
    typedef struct
    {
      uint16_t dig_T1;
      int16_t  dig_T2;
      int16_t  dig_T3;

      uint16_t dig_P1;
      int16_t  dig_P2;
      int16_t  dig_P3;
      int16_t  dig_P4;
      int16_t  dig_P5;
      int16_t  dig_P6;
      int16_t  dig_P7;
      int16_t  dig_P8;
      int16_t  dig_P9;

      uint8_t  dig_H1;
      int16_t  dig_H2;
      uint8_t  dig_H3;
      int16_t  dig_H4;
      int16_t  dig_H5;
      int8_t   dig_H6;
    } bme280_calib_data;
/* =========================================================================*/
bme280_calib_data bme280_calib;
signed short t_fine;

unsigned char read8 (byte i2caddr,byte reg)
{
  unsigned char value;
  Wire.beginTransmission((unsigned char) i2caddr);
  Wire.write((byte)reg);
  Wire.endTransmission();
  Wire.requestFrom((unsigned char)i2caddr,(byte)1);
  value=Wire.read();
return value;
}

// Procedure lecture sur 16 bits
unsigned short read16 (byte i2caddr,byte reg)
{
  unsigned short value;
  Wire.beginTransmission((unsigned char) i2caddr);
  Wire.write((unsigned char)reg);
  Wire.endTransmission();
  Wire.requestFrom((unsigned char)i2caddr,(byte)2);
  value=Wire.read()<<8 | Wire.read();
return value;
}

// Procedure pour inversion Bit Faible/Fort - pour 16 bits
unsigned short read16_LE (byte i2caddr,byte reg)
{
  unsigned short temp;
  temp = read16 (i2caddr,reg);
  return (temp >> 8) | (temp << 8);
}

// Procedure pour Signed int 16 avec inversion Faible et Fort
signed short readS16_LE (byte i2caddr,byte reg)
{
  signed short temp;
  temp = (signed short) read16 (i2caddr,reg);
  return (temp >> 8) | (temp << 8);
}

// Procedure pour Signed int 16
signed short readS16 (byte i2caddr,byte reg)
{
  signed short value;
  value =(signed short) read16 (i2caddr,reg);
  return value;
}

// Procedure pour Signed int 16
unsigned short read24 (byte i2caddr,byte reg)
{
  unsigned short value;
  Wire.beginTransmission((unsigned char) i2caddr);
  Wire.write((unsigned char)reg);
  Wire.endTransmission();
  Wire.requestFrom((unsigned char)i2caddr,(byte)3);
  value=Wire.read();
  value <<=8;
  value |= Wire.read();
  value <<=8;
  value |= Wire.read();
  return value;
}

void write8 (byte i2caddr,byte reg,byte value)
{
  Wire.beginTransmission((unsigned char) i2caddr);
  Wire.write((byte)reg);
  Wire.write((byte)value);
  Wire.endTransmission();
  }

void readCoefficients() {
   bme280_calib.dig_T1 = read16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_T1);
    bme280_calib.dig_T2 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_T2);
    bme280_calib.dig_T3 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_T3);
    
    bme280_calib.dig_P1 = read16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P1);
    bme280_calib.dig_P2 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P2);
    bme280_calib.dig_P3 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P3);
    bme280_calib.dig_P4 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P4);
    bme280_calib.dig_P5 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P5);
    bme280_calib.dig_P6 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P6);
    bme280_calib.dig_P7 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P7);
    bme280_calib.dig_P8 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P8);
    bme280_calib.dig_P9 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_P9);

    bme280_calib.dig_H1 = read8(BME280_ADDRESS,BME280_REGISTER_DIG_H1);
    bme280_calib.dig_H2 = readS16_LE(BME280_ADDRESS,BME280_REGISTER_DIG_H2);
    bme280_calib.dig_H3 = read8(BME280_ADDRESS,BME280_REGISTER_DIG_H3);
    bme280_calib.dig_H4 = (read8(BME280_ADDRESS,BME280_REGISTER_DIG_H4) << 4) | (read8(BME280_ADDRESS,BME280_REGISTER_DIG_H4+1) & 0xF);
    bme280_calib.dig_H5 = (read8(BME280_ADDRESS,BME280_REGISTER_DIG_H5+1) << 4) | (read8(BME280_ADDRESS,BME280_REGISTER_DIG_H5) >> 4);
    bme280_calib.dig_H6 = (char)read8(BME280_ADDRESS,BME280_REGISTER_DIG_H6);
}
float readTemperature(void)
{
  signed int var1, var2;

  signed int adc_T = read24(BME280_ADDRESS,BME280_REGISTER_TEMPDATA);
  adc_T >>= 4;

  var1  = ((((adc_T>>3) - ((signed int)bme280_calib.dig_T1 <<1))) *
     ((signed int)bme280_calib.dig_T2)) >> 11;

  var2  = (((((adc_T>>4) - ((signed int)bme280_calib.dig_T1)) *
       ((adc_T>>4) - ((signed int)bme280_calib.dig_T1))) >> 12) *
     ((signed int)bme280_calib.dig_T3)) >> 14;

  t_fine = var1 + var2;

  float T  = (t_fine * 5 + 128) >> 8;
  return T/100;
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
float readPressure(void) {
  signed long var1, var2, p;

  readTemperature(); // must be done first to get t_fine

  int32_t adc_P = read24(BME280_ADDRESS,BME280_REGISTER_PRESSUREDATA);
  adc_P >>= 4;

  var1 = ((signed long)t_fine) - 128000;
  var2 = var1 * var1 * (signed long)bme280_calib.dig_P6;
  var2 = var2 + ((var1*(signed long)bme280_calib.dig_P5)<<17);
  var2 = var2 + (((signed long)bme280_calib.dig_P4)<<35);
  var1 = ((var1 * var1 * (signed long)bme280_calib.dig_P3)>>8) +
    ((var1 * (signed long)bme280_calib.dig_P2)<<12);
  var1 = (((((signed long)1)<<47)+var1))*((signed long)bme280_calib.dig_P1)>>33;

  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - adc_P;
  p = (((p<<31) - var2)*3125) / var1;
  var1 = (((signed long)bme280_calib.dig_P9) * (p>>13) * (p>>13)) >> 25;
  var2 = (((signed long)bme280_calib.dig_P8) * p) >> 19;

  p = ((p + var1 + var2) >> 8) + (((signed long)bme280_calib.dig_P7)<<4);
  return (float)p/256;
}


/**************************************************************************/
/*!

*/
/**************************************************************************/
float readHumidity(void) {

  readTemperature(); // must be done first to get t_fine

  signed int adc_H = read16(BME280_ADDRESS,BME280_REGISTER_HUMIDDATA);

  signed int v_x1_u32r;

  v_x1_u32r = (t_fine - ((signed int)76800));

  v_x1_u32r = (((((adc_H << 14) - (((signed int)bme280_calib.dig_H4) << 20) -
      (((unsigned short)bme280_calib.dig_H5) * v_x1_u32r)) + ((signed int)16384)) >> 15) *
         (((((((v_x1_u32r * ((signed int)bme280_calib.dig_H6)) >> 10) *
        (((v_x1_u32r * ((unsigned short)bme280_calib.dig_H3)) >> 11) + ((signed int)32768))) >> 10) +
      ((signed int)2097152)) * ((signed int)bme280_calib.dig_H2) + 8192) >> 14));

  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
           ((signed int)bme280_calib.dig_H1)) >> 4));

  v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
  v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
  float h = (v_x1_u32r>>12);
  return  h / 1024.0;
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float readAltitude(float seaLevel)
{
  // Equation taken from BMP180 datasheet (page 16):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

  float atmospheric = readPressure() / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / seaLevel, 0.1903));
}

/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude 
    (in meters), and atmospheric pressure (in hPa).  
    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
*/
/**************************************************************************/
float seaLevelForAltitude(float altitude, float atmospheric)
{
  // Equation taken from BMP180 datasheet (page 17):
  //  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

  // Note that using the equation from wikipedia can give bad results
  // at high altitude.  See this thread for more information:
  //  http://forums.adafruit.com/viewtopic.php?f=22&t=58064
  
  return atmospheric / pow(1.0 - (altitude/44330.0), 5.255);
}
void setup() {
  Serial.begin(9600);
  Serial.println("BME280 test done by D. RAY");
  Wire.begin((int)SDA_PIN,(int)SCL_PIN); // Ouverture de l'I2C */
  if (read8(BME280_ADDRESS,BME280_REGISTER_CHIPID)!= 0x60)
    { Serial.println("BME Non Détecté");
      while (1);
    }
  Serial.println("BME280 detected");    
  write8(BME280_ADDRESS,BME280_REGISTER_CONTROLHUMID,0x05);
  Serial.println("Configuration du BME280 / 16x oversampling pour l'humidité");
  write8(BME280_ADDRESS,BME280_REGISTER_CONTROL,0xB7);
  Serial.println("Configuration du BME280 / Fonctionnement NORMAL / 16x oversampling pour la Température et la pression");
  readCoefficients();
  Serial.println("Lecture des coefficients de correction");
  }



void loop() {
  
 
    Serial.print("Temperature = ");
    Serial.print(readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(readHumidity());
    Serial.println(" %");

    Serial.println();
    delay(2000);
}



