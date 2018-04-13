/**************************************************************************/
/*!
    @file     GY33_MCU.h
    @author   BPoHVoodoo (FabLab Luenen)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, 
    All rights reserved.
    
    Driver for the GY-33 MCU digital color sensors.

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/

#include <stdlib.h>
#include <math.h>

#include "GY33_MCU.h"

/*========================================================================*/
/*                          PRIVATE FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    @brief  Implements missing powf function
*/
/**************************************************************************/
float powf(const float x, const float y)
{
  return (float)(pow((double)x, (double)y));
}

/**************************************************************************/
/*!
    @brief  Writes a register and an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t GY33_MCU::write8 (uint8_t reg, uint8_t val)
{
  uint8_t buf[2];
  brzo_i2c_start_transaction(MCU_ADDRESS, SCL_SPEED);
  buf[0]=reg;
  buf[1]=val;
  brzo_i2c_write(buf, 2, false);
  return brzo_i2c_end_transaction();
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value over I2C
*/
/**************************************************************************/
uint8_t GY33_MCU::read8(uint8_t reg)
{
  uint8_t buf[2];
  brzo_i2c_start_transaction(MCU_ADDRESS, SCL_SPEED);
  buf[0]=reg;
  brzo_i2c_write(buf,1, true);
  brzo_i2c_read(buf, 1, false);
  brzo_i2c_end_transaction();
  return buf[0];
}

/**************************************************************************/
/*!
    @brief  Reads a 16 bit values over I2C
*/
/**************************************************************************/
uint16_t GY33_MCU::read16(uint8_t reg)
{
  uint16_t x; uint16_t t;
  uint8_t buf[2];
  brzo_i2c_start_transaction(MCU_ADDRESS, SCL_SPEED);
  buf[0]=reg;
  brzo_i2c_write(buf, 1, true);
  brzo_i2c_read(buf, 2, false);
  brzo_i2c_end_transaction();  
  x = buf[0];
  t = buf[1];
  x <<= 8;
  x |= t;
  return buf[0] << 8 | buf[1];
}


/*========================================================================*/
/*                            CONSTRUCTORS                                */
/*========================================================================*/

/**************************************************************************/
/*!
    Constructor
*/
/**************************************************************************/
GY33_MCU::GY33_MCU() 
{
  _MCUInitialised = false;
}

/*========================================================================*/
/*                           PUBLIC FUNCTIONS                             */
/*========================================================================*/

/**************************************************************************/
/*!
    Initializes I2C and configures the sensor (call this function before
    doing anything else)
*/
/**************************************************************************/
boolean GY33_MCU::begin(void) 
{
  brzo_i2c_setup(SDA, SCL, SCL_STRETCH_TIMEOUT);
  
  /* Make sure we're actually connected */
  uint8_t x = read8(MCU_CONFIG);
  Serial.println(x, HEX);
  if (x != 0x10) 
  {
    return false;
  }
  _MCUInitialised = true;

  return true;
}


/**************************************************************************/
/*!
    @brief  Reads the raw red, green, blue and clear channel values
*/
/**************************************************************************/
void GY33_MCU::getRawData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c, uint16_t *lux, uint16_t *ct)
{
  if (!_MCUInitialised) begin();

  *r   = read16(MCU_RDATAH);
  *g   = read16(MCU_GDATAH);
  *b   = read16(MCU_BDATAH);
  *c   = read16(MCU_CDATAH);
  *lux = read16(MCU_LDATAH);
  *ct  = read16(MCU_CTDATAH);
}


void GY33_MCU::getData (uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *c, uint8_t *conf)
{
  if (!_MCUInitialised) begin();

  *r = read8(MCU_RDATA);
  *g = read8(MCU_GDATA);
  *b = read8(MCU_BDATA);
  *c = read8(MCU_COLDATA);
  *conf = read8(MCU_CONFIG);
}

/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to color temperature in degrees
            Kelvin
*/
/**************************************************************************/
uint16_t GY33_MCU::calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
{
  float X, Y, Z;      /* RGB to XYZ correlation      */
  float xc, yc;       /* Chromaticity co-ordinates   */
  float n;            /* McCamy's formula            */
  float cct;

  /* 1. Map RGB values to their XYZ counterparts.    */
  /* Based on 6500K fluorescent, 3000K fluorescent   */
  /* and 60W incandescent values for a wide range.   */
  /* Note: Y = Illuminance or lux                    */
  X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
  Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
  Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

  /* 2. Calculate the chromaticity co-ordinates      */
  xc = (X) / (X + Y + Z);
  yc = (Y) / (X + Y + Z);

  /* 3. Use McCamy's formula to determine the CCT    */
  n = (xc - 0.3320F) / (0.1858F - yc);

  /* Calculate the final CCT */
  cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

  /* Return the results in degrees Kelvin */
  return (uint16_t)cct;
}

/**************************************************************************/
/*!
    @brief  Converts the raw R/G/B values to lux
*/
/**************************************************************************/
uint16_t GY33_MCU::calculateLux(uint16_t r, uint16_t g, uint16_t b)
{
  float illuminance;

  /* This only uses RGB ... how can we integrate clear or calculate lux */
  /* based exclusively on clear since this might be more reliable?      */
  illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

  return (uint16_t)illuminance;
}

void GY33_MCU::setConfig(uint8_t high, uint8_t low) {
   Serial.println("GY-33: ");
   Serial.println(high | low, HEX);
   write8(MCU_CONFIG, high | low);
}
uint8_t GY33_MCU::getConfig(void)
{
  if (!_MCUInitialised) begin();

  return read8(MCU_CONFIG);
}
