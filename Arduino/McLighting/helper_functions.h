// Prototypes
bool readSegmentStateFS(uint8_t _seg);
// End Prototypes

void getACK(char *buffer) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", buffer );
}

// Call convertColors whenever main_color, back_color or xtra_color changes.
void convertColors() {
  hexcolors_trans[0] = (uint32_t)(main_color.white << 24) | (main_color.red << 16) | (main_color.green << 8) | main_color.blue;
  hexcolors_trans[1] = (uint32_t)(back_color.white << 24) | (back_color.red << 16) | (back_color.green << 8) | back_color.blue;
  hexcolors_trans[2] = (uint32_t)(xtra_color.white << 24) | (xtra_color.red << 16) | (xtra_color.green << 8) | xtra_color.blue;
}

/*uint32_t* convertColors2(uint8_t _w, uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _w2, uint8_t _r2, uint8_t _g2, uint8_t _b2, uint8_t _w3, uint8_t _r3, uint8_t _g3, uint8_t _b3) {
  uint32_t _hexcolors[3] = {};
  _hexcolors[0] = (uint32_t)(_w  << 24) | (_r  << 16) | (_g  << 8) | _b;
  _hexcolors[1] = (uint32_t)(_w2 << 24) | (_r2 << 16) | (_g2 << 8) | _b2;
  _hexcolors[2] = (uint32_t)(_w3 << 24) | (_r3 << 16) | (_g3 << 8) | _b3;
  return _hexcolors;
}*/

uint16_t convertSpeed(uint8_t _mcl_speed) {
  uint16_t _fx_speed = 0;
  if (_mcl_speed < 50) {
    _fx_speed = 65535 - (_mcl_speed * 1000);
  } else if (_mcl_speed < 100) {
    _fx_speed = 16675 - ((_mcl_speed-49) * 250);
  } else if (_mcl_speed < 150) {
    _fx_speed = 4075 - ((_mcl_speed-99) * 50);
  } else if (_mcl_speed < 200) {
    _fx_speed = 1550 - ((_mcl_speed-149) * 25);
  } else {
    _fx_speed = 280 - ((_mcl_speed-199) * 5);
  }
  _fx_speed = constrain(_fx_speed, SPEED_MIN, SPEED_MAX);
  return _fx_speed;
}

/*uint8_t unconvertSpeed(uint16_t _fx_speed) {
  uint16_t _mcl_speed = 0;
  if (_fx_speed <= 280) {
    _mcl_speed = ((280 - _fx_speed)/5) + 199;
  } else if (_fx_speed < 1550) {
    _mcl_speed = ((1550 - _fx_speed)/25) + 149;
  } else if (_fx_speed < 4075) {
    _mcl_speed = ((4075 - _fx_speed)/50) + 99;
  } else if (_mcl_speed < 16500) {
    _mcl_speed = ((16675 - _fx_speed)/250) + 49;
  } else {
    _mcl_speed = ((65535 - _fx_speed)/1000);
  }
  return _mcl_speed;
}*/

bool checkPin(uint8_t pin) {
  #if defined(USE_WS2812FX_DMA)
    #if USE_WS2812FX_DMA == 0
      pin = 3;
    #endif
    #if USE_WS2812FX_DMA == 1
      pin = 1;
    #endif
    #if USE_WS2812FX_DMA == 2
      pin = 2;
    #endif
  #endif
  if (((pin >= 0 && pin <= 5) || (pin >= 12 && pin <= 16)) && (pin != Config.pin)) {
    Config.pin = pin;
    return true;
  }
  return false;
}

neoPixelType checkRGBOrder(char rgbOrder[5]) {
  for( uint8_t i=0 ; i < sizeof(rgbOrder) ; ++i ) rgbOrder[i] = toupper(rgbOrder[i]) ;
  DBG_OUTPUT_PORT.printf("Checking RGB Order: %s ...", rgbOrder);
  neoPixelType returnOrder = 0;
  if (strcmp(rgbOrder, "GRB") == 0)  {
    returnOrder = NEO_GRB;
  } else if (strcmp(rgbOrder, "GBR") == 0) {
    returnOrder = NEO_GBR;
  } else if (strcmp(rgbOrder, "RGB") == 0) {
    returnOrder = NEO_RGB;
  } else if (strcmp(rgbOrder, "RBG") == 0) {
    returnOrder = NEO_RBG;
  } else if (strcmp(rgbOrder, "BRG") == 0) {
    returnOrder = NEO_BRG;
  } else if (strcmp(rgbOrder, "BGR") == 0) {
    returnOrder = NEO_BGR;
  } else if (strcmp(rgbOrder, "WGRB") == 0) {
    returnOrder = NEO_WGRB;
  } else if (strcmp(rgbOrder, "WGBR") == 0) {
    returnOrder = NEO_WGBR;
  } else if (strcmp(rgbOrder, "WRGB") == 0) {
    returnOrder = NEO_WRGB;
  } else if (strcmp(rgbOrder, "WRBG") == 0) {
    returnOrder = NEO_WRBG;
  } else if (strcmp(rgbOrder, "WBRG") == 0) {
    returnOrder = NEO_WBRG;
  } else if (strcmp(rgbOrder, "WBGR") == 0) {
    returnOrder = NEO_WBGR;
  } else if (strcmp(rgbOrder, "GWRB") == 0) {
    returnOrder = NEO_GWRB;
  } else if (strcmp(rgbOrder, "GWBR") == 0) {
    returnOrder = NEO_GWBR;
  } else if (strcmp(rgbOrder, "RWGB") == 0) {
    returnOrder = NEO_RWGB;
  } else if (strcmp(rgbOrder, "RWBG") == 0) {
    returnOrder = NEO_RWBG;
  } else if (strcmp(rgbOrder, "BWRG") == 0) {
    returnOrder = NEO_BWRG;
  } else if (strcmp(rgbOrder, "BWGR") == 0) {
    returnOrder = NEO_BWGR;
  } else if (strcmp(rgbOrder, "GRWB") == 0) {
    returnOrder = NEO_GRWB;
  } else if (strcmp(rgbOrder, "GBWR") == 0) {
    returnOrder = NEO_GBWR;
  } else if (strcmp(rgbOrder, "RGWB") == 0) {
    returnOrder = NEO_RGWB;
  } else if (strcmp(rgbOrder, "RBWG") == 0) {
    returnOrder = NEO_RBWG;
  } else if (strcmp(rgbOrder, "BRWG") == 0){
    returnOrder = NEO_BRWG;
  } else if (strcmp(rgbOrder, "BGWR") == 0) {
    returnOrder = NEO_GRBW;
  } else if (strcmp(rgbOrder, "GRBW") == 0) {
    returnOrder = NEO_GRBW;
  } else if (strcmp(rgbOrder, "GBWR") == 0) {
    returnOrder = NEO_GBRW;
  } else if (strcmp(rgbOrder, "RGBW") == 0) {
    returnOrder = NEO_RGBW;
  } else if (strcmp(rgbOrder, "RBGW") == 0) {
    returnOrder = NEO_RBGW;
  } else if (strcmp(rgbOrder, "BRGW") == 0) {
    returnOrder = NEO_BRGW;
  } else if (strcmp(rgbOrder, "BGRW") == 0) {
    returnOrder = NEO_BGRW;
  } else {
    DBG_OUTPUT_PORT.print("invalid input!");
    uint16_t check = checkRGBOrder(Config.RGBOrder);
    if (check != 0) {
      returnOrder = static_cast<neoPixelType>(check);
      strcpy(rgbOrder, Config.RGBOrder);
    } else {
      returnOrder = static_cast<neoPixelType>(checkRGBOrder(RGBORDER));
      strcpy(rgbOrder, RGBORDER);
    }
  }
  DBG_OUTPUT_PORT.println("success!");
  strcpy(Config.RGBOrder, rgbOrder);
  return returnOrder;
}

// function to Initialize the strip
void initStrip(uint16_t _stripSize = Config.stripSize, uint8_t _num_segments = Config.segments, char _RGBOrder[5] = Config.RGBOrder, uint8_t _pin = Config.pin){
  DBG_OUTPUT_PORT.println("Initializing strip!");
/*#if defined(USE_WS2812FX_DMA)
  if (dma != NULL) {
     delete(dma);
  }
#endif*/
  if (strip != NULL) {
    strip->strip_off();
    delay(10);
    if(strip->isRunning()) strip->stop();
    strip->resetSegments();
    strip->resetSegmentRuntimes();
    delete(strip);
    Config.stripSize = _stripSize;
    strcpy(Config.RGBOrder, _RGBOrder);
    Config.pin = _pin;
  }

  if (ledstates != NULL) {
    delete(ledstates);
  }
  ledstates = new uint8_t [_stripSize];

#if !defined(LED_TYPE_WS2811)
  strip = new WS2812FX(_stripSize, _pin, checkRGBOrder(_RGBOrder) + NEO_KHZ800);
#else
  strip = new WS2812FX(_stripSize, _pin, checkRGBOrder(_RGBOrder) + NEO_KHZ400);
#endif
  // Parameter 1 = number of pixels in strip
  // Parameter 2 = Arduino pin number (most are valid)
  // Parameter 3 = pixel type flags, add together as needed:
  //   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
  //   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
  //   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
  //   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

  // IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
  // pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
  // and minimize distance between Arduino and first pixel.  Avoid connecting
  // on a live circuit...if you must, connect GND first.
  strip->init();
  #if defined(USE_WS2812FX_DMA)
    initDMA(_stripSize);
    strip->setCustomShow(DMA_Show);
  #endif
  //parameters: index, start, stop, mode, color, speed, options
  for (uint8_t _seg=0; _seg < Config.segments; _seg++) {
    if (_seg != State.segment) {  // Read actual segment last
      (readSegmentStateFS(_seg)) ? DBG_OUTPUT_PORT.println("Segment state config FS read Success!") : DBG_OUTPUT_PORT.println("Segment state config FS read failure!");
      memcpy(segState.colors[_seg], hexcolors_trans, sizeof(hexcolors_trans));
      strip->setSegment(_seg,  segState.start,  segState.stop, segState.mode[_seg], segState.colors[_seg], convertSpeed(segState.speed[_seg]), segState.options);
    }
  }
  //read actual segment last to set all vars correctly
  (readSegmentStateFS(State.segment)) ? DBG_OUTPUT_PORT.println("Segment state config FS read Success!") : DBG_OUTPUT_PORT.println("Segment state config FS read failure!");
  memcpy(segState.colors[State.segment], hexcolors_trans, sizeof(hexcolors_trans));
  strip->setSegment(State.segment,  segState.start,  segState.stop , segState.mode[State.segment], hexcolors_trans, convertSpeed(segState.speed[State.segment]), segState.options);
  fx_speed = segState.speed[State.segment];
  fx_mode = segState.mode[State.segment];
  brightness_trans = State.brightness;
  prevsegment = State.segment;
  strip->setCustomMode(0, F("Autoplay"), handleAuto);
  strip->setCustomMode(1, F("Custom WS"), handleCustomWS);
  strip->setCustomMode(9, F("Segment OFF"), handleSegmentOFF);
  #if defined(CUSTOM_WS2812FX_ANIMATIONS)
    strip->setCustomMode(2, F("TV"), handleTV);
    strip->setCustomMode(3, F("E1.31"),  handleE131);
    strip->setCustomMode(4, F("Fire 2012"), handleFire2012);
    strip->setCustomMode(5, F("Gradient"),  handleGradient);
    DBG_OUTPUT_PORT.print("Number of Segments: ");
    DBG_OUTPUT_PORT.println(strip->getNumSegments());

    if (e131 != NULL) { delete(e131); }
    e131 = new ESPAsyncE131(END_UNIVERSE - START_UNIVERSE + 1);
    float universe_leds = 170.0;  // a universe has only 512 (0..511) channels: 3*170 or 4*128 <= 512
    if (strstr(Config.RGBOrder, "W") != NULL) {
      //universe_leds = 128.0;
    }
    float float_enduni = _stripSize/universe_leds;
    uint8_t END_UNIVERSE = _stripSize/universe_leds;
    if (float_enduni > END_UNIVERSE) {
      END_UNIVERSE = END_UNIVERSE +1;
    }

    // if (e131.begin(E131_UNICAST))                              // Listen via Unicast
    if (e131->begin(E131_MULTICAST, START_UNIVERSE, END_UNIVERSE)) {// Listen via Multicast
        DBG_OUTPUT_PORT.println(F("Listening for data..."));
    } else {
        DBG_OUTPUT_PORT.println(F("*** e131.begin failed ***"));
    }
  #endif
}

void getSegmentParams(uint8_t _seg) {
  segState.start   = strip->getSegment(_seg)->start;;
  segState.stop    = strip->getSegment(_seg)->stop;;
  //segState.mode[_seg]     = strip->getMode(_seg);
  //segState.speed[_seg]    = unconvertSpeed(strip->getSpeed(_seg));
  //fx_mode          = segState.mode[_seg];
  //fx_speed         = segState.speed[_seg];
  main_color.white = ((segState.colors[_seg][0] >> 24) & 0xFF);
  main_color.red   = ((segState.colors[_seg][0] >> 16) & 0xFF);
  main_color.green = ((segState.colors[_seg][0] >>  8) & 0xFF);
  main_color.blue  = ((segState.colors[_seg][0])  & 0xFF);
  back_color.white = ((segState.colors[_seg][1] >> 24) & 0xFF);
  back_color.red   = ((segState.colors[_seg][1] >> 16) & 0xFF);
  back_color.green = ((segState.colors[_seg][1] >>  8) & 0xFF);
  back_color.blue  = ((segState.colors[_seg][1]) & 0xFF);
  xtra_color.white = ((segState.colors[_seg][2] >> 24) & 0xFF);
  xtra_color.red   = ((segState.colors[_seg][2] >> 16) & 0xFF);
  xtra_color.green = ((segState.colors[_seg][2] >>  8) & 0xFF);
  xtra_color.blue  = ((segState.colors[_seg][2] >>  0) & 0xFF);
  segState.options = strip->getOptions(_seg);
}

void setSegmentSize() {
  strip->strip_off();
  delay(10);
  if(strip->isRunning()) strip->stop();
  strip->resetSegmentRuntimes();
  strip->setSegment(State.segment,  segState.start,  segState.stop , segState.mode[State.segment], hexcolors_trans, convertSpeed(segState.speed[State.segment]), segState.options);
}

uint8_t calculateColorTransitionSteps(uint8_t _seg) {
  //compare all colors and calculate steps
  int     _trans_cnt_max = 0;
  int     _calculate_max[4] = {};
  for (uint8_t i=0; i<3; i++){
    for (uint8_t j=0; j<4; j++) {
      _calculate_max[j] = ((strip->getColors(_seg)[i] >> ((3-j)*8)) & 0xFF) - ((hexcolors_trans[i] >> ((3-j)*8)) & 0xFF);
      _calculate_max[j] = abs(_calculate_max[j]);
      _trans_cnt_max = max(_trans_cnt_max, _calculate_max[j]);
    }
  }
  return _trans_cnt_max;
}

uint8_t convertColorsFade(uint8_t _seg) {
  if (Config.transEffect) {
      if (trans_cnt > 1) {
        //memcpy(segState.colors[_seg], strip->getColors(_seg), sizeof(segState.colors[_seg]));
        DBG_OUTPUT_PORT.println("Color transistion aborted. Restarting...!");
        trans_cnt = 1;
      }
    return calculateColorTransitionSteps(_seg);
  } else {
    return 0;
  }
}

uint32_t scale_wrgb(uint32_t wrgb, uint8_t level) {
    uint8_t w = (uint16_t)(((wrgb >> 24) & 0xFF) * level / 255);
    uint8_t r = (uint16_t)(((wrgb >> 16) & 0xFF) * level / 255);
    uint8_t g = (uint16_t)(((wrgb >>  8) & 0xFF) * level / 255);
    uint8_t b = (uint16_t)(((wrgb) & 0xFF)       * level / 255);
    return (w << 24) | (r << 16) | (g << 8) | b;
}

uint32_t trans(uint32_t _newcolor, uint32_t _oldcolor, uint8_t _level, uint8_t _steps) {
  if (_steps == 0) { _steps = 1; };
  _level = (uint16_t)(_level * 255 / _steps);
  _newcolor = scale_wrgb(_newcolor, _level);
  _oldcolor = scale_wrgb(_oldcolor, 255-_level);
  return _newcolor + _oldcolor;
}
