#include <brzo_i2c.h>   //https://github.com/pasko-zh/brzo_i2c

/**************************************************************************/
/*! 
    @file     GY33_MCU.h
    @author   BPoHVoodoo (FabLab Luenen)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, 
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/
#ifndef _MCU_H_
#define _MCU_H_

#include <brzo_i2c.h>        // https://github.com/pasko-zh/brzo_i2c

#define MCU_ADDRESS          (0x5A)

#define SCL_SPEED            100
#define SCL_STRETCH_TIMEOUT  50000

#define MCU_LED_OFF          (0xA0)
#define MCU_LED_01           (0x90)
#define MCU_LED_02           (0x80)
#define MCU_LED_03           (0x70)
#define MCU_LED_04           (0x60)
#define MCU_LED_05           (0x50)
#define MCU_LED_06           (0x40)
#define MCU_LED_07           (0x30)
#define MCU_LED_08           (0x20)
#define MCU_LED_09           (0x10)
#define MCU_LED_10           (0x00)
#define MCU_WHITE_OFF        (0x00)  /* No Whitebalance  */
#define MCU_WHITE_ON         (0x01)  /* Whitebalance */

#define MCU_RDATAH           (0x00)    /* Raw Red channel data */
#define MCU_RDATAL           (0x01)
#define MCU_GDATAH           (0x02)    /* Raw Green channel data */
#define MCU_GDATAL           (0x03)
#define MCU_BDATAH           (0x04)    /* Raw Blue channel data */
#define MCU_BDATAL           (0x05)
#define MCU_CDATAH           (0x06)    /* Clear channel data */
#define MCU_CDATAL           (0x07)
#define MCU_LDATAH           (0x08)    /* Lux channel data */
#define MCU_LDATAL           (0x09)
#define MCU_CTDATAH          (0x0A)    /* Colortemperature channel data */
#define MCU_CTDATAL          (0x0B)
#define MCU_RDATA            (0x0C)    /* Red channel data */
#define MCU_GDATA            (0x0D)    /* Green channel data */
#define MCU_BDATA            (0x0E)    /* Blue channel data */
#define MCU_COLDATA          (0x0F)    /* Blue channel data */
#define MCU_CONFIG           (0x10)    /* Config channel data */

class GY33_MCU {
 public:
  GY33_MCU();
 
  boolean  begin(void);
  void     getRawData(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c, uint16_t *lux, uint16_t *ct);
  void     getData(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *c, uint8_t *conf);
  uint16_t calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b);
  uint16_t calculateLux(uint16_t r, uint16_t g, uint16_t b);
  uint8_t  write8 (uint8_t reg, uint8_t val);
  uint8_t  read8 (uint8_t reg);
  uint16_t read16 (uint8_t reg);
  void     setConfig(uint8_t h, uint8_t l);
  uint8_t  getConfig(void);
 private:
  boolean _MCUInitialised;
  
  void     disable(void);
};

#endif
