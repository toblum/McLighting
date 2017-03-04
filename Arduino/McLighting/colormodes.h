// ***************************************************************************
// Color modes
// ***************************************************************************

int dipInterval = 10;
int darkTime = 250;
unsigned long currentDipTime;
unsigned long dipStartTime;
unsigned long currentMillis;
int ledState = LOW;
long previousMillis = 0;
int led = 5;
int interval = 2000;
int twitch = 50;
int dipCount = 0;
int analogLevel = 100;
boolean timeToDip = false;
int ledStates[NUMLEDS];

void hsb2rgbAN1(uint16_t index, uint8_t sat, uint8_t bright, uint8_t myled) {
  // Source: https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
  uint8_t temp[5], n = (index >> 8) % 3;
  temp[0] = temp[3] = (uint8_t)((                                        (sat ^ 255)  * bright) / 255);
  temp[1] = temp[4] = (uint8_t)((((( (index & 255)        * sat) / 255) + (sat ^ 255)) * bright) / 255);
  temp[2] =          (uint8_t)(((((((index & 255) ^ 255) * sat) / 255) + (sat ^ 255)) * bright) / 255);

  strip.setPixelColor(myled, temp[n + 2], temp[n + 1], temp[n]);
}


void updateLed (int led, int brightness) {
  ledStates[led] = brightness;

  for (int i = 0; i < NUMLEDS; i++)
  {
    uint16_t index = (i % 3 == 0) ? 400 : random(0, 767);
    hsb2rgbAN1(index, 200, ledStates[i], i);
  }
  strip.show();
}


// See: http://forum.mysensors.org/topic/85/phoneytv-for-vera-is-here/13
void tv() {
  checkForRequests();
  if (exit_func) {
    exit_func = false;
    return;
  }

  if (timeToDip == false)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis > interval)
    {
      previousMillis = currentMillis;
      interval = random(750, 4001); //Adjusts the interval for more/less frequent random light changes
      twitch = random(40, 100); // Twitch provides motion effect but can be a bit much if too high
      dipCount = dipCount++;
    }
    if (currentMillis - previousMillis < twitch)
    {
      led = random(0, (strip.numPixels() - 1));
      analogLevel = random(50, 255); // set the range of the 3 pwm leds
      ledState = ledState == LOW ? HIGH : LOW; // if the LED is off turn it on and vice-versa:

      updateLed(led, (ledState) ? 255 : 0);

      if (dipCount > dipInterval)
      {
        DBG_OUTPUT_PORT.println("dip");
        timeToDip = true;
        dipCount = 0;
        dipStartTime = millis();
        darkTime = random(50, 150);
        dipInterval = random(5, 250); // cycles of flicker
      }
      //strip.show();
    }
  }
  else
  {
    DBG_OUTPUT_PORT.println("Dip Time");
    currentDipTime = millis();
    if (currentDipTime - dipStartTime < darkTime)
    {
      for (int i = 3; i < strip.numPixels(); i++)
      {
        updateLed(i, 0);
      }
    }
    else
    {
      timeToDip = false;
    }
    strip.show();
  }
}



//*************************************
// Artnet source : http://forum.arduino.cc/index.php?action=profile;u=130243
// http://forum.arduino.cc/index.php?topic=434498.0
//************************************
#ifdef ENABLE_ARTNET
  void ICACHE_FLASH_ATTR sendWS() {
    uint32_t writeBits;
    uint8_t  bitMask, time;
    os_intr_lock();
    
    for (uint16_t t = 0; t < uniSize; t++) { // outer loop counting bytes
      bitMask = 0x80;
      while (bitMask) {
        // time=0ns : start by setting bit on
        time = 4;
        while (time--) {
          WRITE_PERI_REG( 0x60000304, WSbit );  // do ON bits // T=0
        }
        if ( uniData[t] & bitMask ) {
          writeBits = 0;  // if this is a '1' keep the on time on for longer, so dont write an off bit
        }
        else {
          writeBits = WSbit;  // else it must be a zero, so write the off bit !
        }
        time = 4;
        while (time--) {
          WRITE_PERI_REG( 0x60000308, writeBits );  // do OFF bits // T='0' time 350ns
        }
        time = 6;
        while (time--) {
          WRITE_PERI_REG( 0x60000308, WSbit );  // switch all bits off  T='1' time 700ns
        }
        // end of bite write time=1250ns
        bitMask >>= 1;
      }
    }
    os_intr_unlock();
  }
  
  void artnet() {
    // checkForRequests();
    if (exit_func) {
      exit_func = false;
      return;
    }
    
    if (udp.parsePacket()) {
      udp.read(hData, ARTNET_HEADER + 1);
      if ( hData[0] == 'A' && hData[1] == 'r' && hData[2] == 't' && hData[3] == '-' && hData[4] == 'N' && hData[5] == 'e' && hData[6] == 't') {
        if ( hData[8] == 0x00 && hData[9] == ARTNET_DATA && hData[15] == net ) {
          if ( hData[14] == (subnet << 4) + universe ) { // UNIVERSE
            uniSize = (hData[16] << 8) + (hData[17]);
            udp.read(uniData, uniSize);
            //Serial.print("ArtNet packet RX Uni 0 - size:");
            //Serial.println(uniSize);
            sendWS();
          }
        }
      }
    }
  }
#endif


