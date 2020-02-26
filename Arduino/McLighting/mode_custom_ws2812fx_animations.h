// Prototypes
uint16_t convertSpeed(uint8_t _mcl_speed);
uint32_t trans(uint32_t _newcolor, uint32_t _oldcolor, uint8_t _level, uint8_t _steps);
// End Prototypes
/*
Example of adding the example: https://github.com/kitesurfer1404/WS2812FX/blob/master/examples/ws2812fx_custom_FastLED/ws2812fx_custom_FastLED.ino
as a custom effect

More info on how to create custom aniamtions for WS2812FX: https://github.com/kitesurfer1404/WS2812FX/blob/master/extras/WS2812FX%20Users%20Guide.md#custom-effects
*/

uint16_t handleSegmentOFF(void) {
  WS2812FX::Segment* _seg = strip->getSegment();
  return _seg->speed/(_seg->stop - _seg->start);
}

// ***************************************************************************
// Function for automatic cycling
// ***************************************************************************
void handleAutoPlay(uint8_t _seg) {
  //WS2812FX::Segment* _seg = strip->getSegment();
  if (autoDelay[_seg] <= millis()) {
  uint32_t _hex_colors[3] = {};
    //if (_seg ==
      _hex_colors[0] = autoParams[autoCount[_seg]][0];
      _hex_colors[1] = autoParams[autoCount[_seg]][1];
      _hex_colors[2] = autoParams[autoCount[_seg]][2];
    //}
    strip->setColors(_seg, _hex_colors);
    strip->setSpeed(_seg, convertSpeed((uint16_t)autoParams[autoCount[_seg]][3]));
    strip->setMode(_seg, (uint8_t)autoParams[autoCount[_seg]][4]);
    //strip->trigger();
    autoDelay[_seg] = millis() + (unsigned long)autoParams[autoCount[_seg]][5];
    DBG_OUTPUT_PORT.printf("autoTick[%d][%d]: {0x%08x, 0x%08x, 0x%08x, %d, %d, %d}\r\n", _seg, autoCount[_seg], autoParams[autoCount[_seg]][0], autoParams[autoCount[_seg]][1], autoParams[autoCount[_seg]][2], autoParams[autoCount[_seg]][3], autoParams[autoCount[_seg]][4], autoParams[autoCount[_seg]][5]);
    autoCount[_seg]++;
    if (autoCount[_seg] >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount[_seg] = 0;
  }
}

uint16_t handleAuto(void) {
  WS2812FX::Segment* _seg = strip->getSegment();
  return _seg->speed/(_seg->stop - _seg->start);
}

uint16_t handleCustomWS(void) {
  WS2812FX::Segment* _seg = strip->getSegment();
  return _seg->speed/(_seg->stop - _seg->start);
}

#if defined(CUSTOM_WS2812FX_ANIMATIONS)
  // ***************************************************************************
  // TV mode to be reviewed
  // ***************************************************************************
  uint16_t darkTime[10] = {250,250,250,250,250,250,250,250,250,250};
  uint8_t  dipInterval[10] = {10,10,10,10,10,10,10,10,10,10};
  unsigned long dipStartTime[10] = {};
  uint8_t  ledState[10] = {LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW};
  unsigned long previousMillis[10]= {0,0,0,0,0,0,0,0,0,0};
  uint16_t interv[10] = {2000,2000,2000,2000,2000,2000,2000,2000,2000,2000};
  uint8_t  twitch[10]= {50,50,50,50,50,50,50,50,50,50};
  uint8_t  dipCount[10] = {0,0,0,0,0,0,0,0,0,0};
  bool     timeToDip[10] = {false,false,false,false,false,false,false,false,false,false};


  void hsb2rgbAN1(uint16_t index, uint8_t sat, uint8_t bright, uint16_t led) {
    // Source: https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
    uint8_t temp[5], n = (index >> 8) % 3;
    temp[0] = temp[3] = (uint8_t)((                                         (sat ^ 255)  * bright) / 255);
    temp[1] = temp[4] = (uint8_t)((((( (index & 255)        * sat) / 255) + (sat ^ 255)) * bright) / 255);
    temp[2] =           (uint8_t)(((((((index & 255) ^ 255) * sat) / 255) + (sat ^ 255)) * bright) / 255);
    strip->setPixelColor(led, temp[n + 2], temp[n + 1], temp[n], 0);
  }


  uint16_t handleTV(void) {
    WS2812FX::Segment* _seg = strip->getSegment();
    uint8_t _seg_num = strip->getSegmentIndex();
    if (timeToDip[_seg_num] == false) {
      if((millis() - previousMillis[_seg_num]) > interv[_seg_num]) {
        previousMillis[_seg_num] = millis();
        //interv = random(750,4001);//Adjusts the interval for more/less frequent random light changes
        interv[_seg_num] = random(800-(512 - (_seg->speed/64)),6001-(2731 - (_seg->speed/24)));
        twitch[_seg_num] = random(40,100);// Twitch provides motion effect but can be a bit much if too high
        dipCount[_seg_num]++;
      }
      if((millis() - previousMillis[_seg_num]) < twitch[_seg_num]) {
        uint16_t led=random(_seg->start, _seg->stop);
        ledState[_seg_num] = ledState[_seg_num] == LOW ? HIGH : LOW; // if the LED is off turn it on and vice-versa:
        ledstates[led] = ((ledState[_seg_num]) ? 255 : 0);
        for (uint16_t j=_seg->start; j<=_seg->stop; j++) {
          uint16_t index = (j%3 == 0) ? 400 : random(0,767);
          hsb2rgbAN1(index, 200, ledstates[j], j);
        }
        if (dipCount[_seg_num] > dipInterval[_seg_num]) {
          timeToDip[_seg_num] = true;
          dipCount[_seg_num] = 0;
          dipStartTime[_seg_num] = millis();
          darkTime[_seg_num] = random(50,150);
          dipInterval[_seg_num] = random(5,250);// cycles of flicker
        }
      }
    } else {
      if (millis() - dipStartTime[_seg_num] < darkTime[_seg_num]) {
        for (uint16_t i = _seg->start; i <= _seg->stop; i++) {
          ledstates[i] = 0;
          for (uint16_t j=_seg->start; j<=_seg->stop; j++) {
            uint16_t index = (j%3 == 0) ? 400 : random(0,767);
            hsb2rgbAN1(index, 200, ledstates[j], j);
          }
        }
      } else {
        timeToDip[_seg_num] = false;
      }
    }
    return _seg->speed/(_seg->stop - _seg->start);
  }
  // ***************************************************************************
  // E1.31 mode
  // ***************************************************************************
  uint16_t handleE131(void) {
  WS2812FX::Segment* _seg = strip->getSegment();
  return _seg->speed/(_seg->stop - _seg->start);
  }
  
  void handleE131Play(void) {
    if (!e131->isEmpty()) {
      e131_packet_t packet;
      e131->pull(&packet); // Pull packet from ring buffer
      
      uint16_t universe = htons(packet.universe);
      uint8_t *data = packet.property_values + 1;

      if (universe < START_UNIVERSE || universe > END_UNIVERSE) return; //async will take care about filling the buffer

      // Serial.printf("Universe %u / %u Channels | Packet#: %u / Errors: %u / CH1: %u\n",
      //               htons(packet.universe),                 // The Universe for this packet
      //               htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
      //               e131.stats.num_packets,                 // Packet counter
      //               e131.stats.packet_errors,               // Packet error counter
      //               packet.property_values[1]);             // Dimmer data for Channel 1
  /*  #if defined(RGBW)
      uint16_t multipacketOffset = (universe - START_UNIVERSE) * 128; //if more than 128 LEDs * 4 colors = 512 channels, client will send in next higher universe
      if (Config.stripSize <= multipacketOffset) return _seg->speed/(_seg->stop - _seg->start);
      uint16_t len = (128 + multipacketOffset > Config.stripSize) ? (Config.stripSize - multipacketOffset) : 128;
    #else*/
      uint16_t multipacketOffset = (universe - START_UNIVERSE) * 170; //if more than 170 LEDs * 3 colors = 510 channels, client will send in next higher universe
      if (Config.stripSize <= multipacketOffset) return;
      uint16_t len = (170 + multipacketOffset > Config.stripSize) ? (Config.stripSize - multipacketOffset) : 170;
  /*  #endif */
      for (uint8_t k = 0; k < Config.segments; k++) {
        if (segState.mode[k] == FX_MODE_CUSTOM_3) {
          for (uint16_t i = 0; i < len; i++){
            if ((i >= strip->getSegment(k)->start) && (i <= strip->getSegment(k)->stop)) {
              uint16_t j = i * 3;
        /*  #if defined(RGBW)
              strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], data[j + 3]);
          #else */
              strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], 0);
       /*  #endif */
            }
          }
        }
      }
    }
  }

  /*
   * paste in the Fire2012 code with a small edit at the end which uses the
   * setPixelColor() function to copy the color data to the ws2812fx instance.
  */
  #include <FastLED.h>  //https://github.com/FastLED/FastLED
  // Fire2012 by Mark Kriegsman, July 2012
  // as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
  ////
  // This basic one-dimensional 'fire' simulation works roughly as follows:
  // There's a underlying array of 'heat' cells, that model the temperature
  // at each point along the line.  Every cycle through the simulation,
  // four steps are performed:
  //  1) All cells cool down a little bit, losing heat to the air
  //  2) The heat from each cell drifts 'up' and diffuses a little
  //  3) Sometimes randomly new 'sparks' of heat are added at the bottom
  //  4) The heat from each cell is rendered as a color into the leds array
  //     The heat-to-color mapping uses a black-body radiation approximation.
  //
  // Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
  //
  // This simulation scales it self a bit depending on NUM_LEDS; it should look
  // "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
  //
  // I recommend running this simulation at anywhere from 30-100 frames per second,
  // meaning an interframe delay of about 10-35 milliseconds.
  //
  // Looks best on a high-density LED setup (60+ pixels/meter).
  //
  //
  // There are two main parameters you can play with to control the look and
  // feel of your fire: COOLING (used in step 1 above), and SPARKING (used
  // in step 3 above).
  //
  // COOLING: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
  // Default 50, suggested range 20-100
  #define COOLING  70

  // SPARKING: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
  // Default 120, suggested range 50-200.
  #define SPARKING 120

  uint16_t handleFire2012(void) {
  // Array of temperature readings at each simulation cell
    WS2812FX::Segment* _seg = strip->getSegment();

    // Step 1.  Cool down every cell a little
    for( uint16_t i = _seg->start; i <= _seg->stop; i++) {
      ledstates[i] = qsub8(ledstates[i],  random8(0, ((COOLING * 10) / (_seg->stop - _seg->start)+1) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( uint16_t k= _seg->stop; k >= (_seg->start + 2); k--) {
      ledstates[k] = (ledstates[k - 1] + ledstates[k - 2] + ledstates[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      uint8_t y = random8(7) + _seg->start;
      ledstates[y] = qadd8(ledstates[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( uint16_t j = _seg->start; j <= _seg->stop; j++) {
      CRGB color = HeatColor(ledstates[j]);
      uint16_t pixel;
      if ((_seg->options & 128) > 0) {
        pixel = _seg->stop + (_seg->start - j);
      } else {
        pixel = j;
      }
      strip->setPixelColor(pixel, color.red, color.green, color.blue, 0);
    }
    return _seg->speed/(_seg->stop - _seg->start);
  }

  uint16_t handleGradient(void) {
    WS2812FX::Segment* _seg = strip->getSegment();
    for(uint16_t j = 0; j <= (_seg->stop - _seg->start); j++) {
      uint16_t pixel;
      if ((_seg->options & 128) > 0) {
        pixel = _seg->stop - j;
      } else {
        pixel = _seg->start + j;
      }
      uint32_t color = trans(_seg->colors[1], _seg->colors[0], j, (_seg->stop - _seg->start));
      strip->setPixelColor(pixel, ((color >> 16) & 0xFF), ((color >> 8) & 0xFF), ((color >> 0) & 0xFF), ((color >> 24) & 0xFF));
    }
    return _seg->speed/(_seg->stop - _seg->start);
  }
#endif
