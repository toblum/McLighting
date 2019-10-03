// Prototypes
bool readSegmentStateFS(uint8_t seg);
// End Prototypes

uint16_t convertSpeed(uint8_t mcl_speed) {
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = SPEED_MAX - ws2812_speed;
  ws2812_speed = constrain(ws2812_speed, SPEED_MIN, SPEED_MAX);
  return ws2812_speed;
}

uint8_t unconvertSpeed(uint16_t ws2812_speed) {
  //log((SPEED_MAX - ws2812_speed)/61760) = (0.0002336 * mcl_speed) - (-0.03181 * mcl_speed);
  //log((SPEED_MAX - ws2812_speed)/61760) = (0.0002336 + 0.03181) * mcl_speed;
  uint16_t  mcl_speed = (log((SPEED_MAX - ws2812_speed)/61760))/ (0.0002336 + 0.03181);
  //uint16_t mcl_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  mcl_speed = 255 - mcl_speed;
  mcl_speed = constrain(mcl_speed, 0, 255);
  return mcl_speed;
}

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
  if (((pin >= 0 && pin <= 5) || (pin >= 12 && pin <= 16)) && (pin != WS2812FXStripSettings.pin)) {
    WS2812FXStripSettings.pin = pin;
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
    uint16_t check = checkRGBOrder(WS2812FXStripSettings.RGBOrder);
    if (check != 0) {
      returnOrder = static_cast<neoPixelType>(check);
      strcpy(rgbOrder, WS2812FXStripSettings.RGBOrder);
    } else {
      returnOrder = static_cast<neoPixelType>(checkRGBOrder(RGBORDER));
      strcpy(rgbOrder, RGBORDER);
    }
  }
  DBG_OUTPUT_PORT.println("success!");
  strcpy(WS2812FXStripSettings.RGBOrder, rgbOrder);
  return returnOrder;
}

// function to Initialize the strip
void initStrip(uint16_t stripSize = WS2812FXStripSettings.stripSize, char RGBOrder[5] = WS2812FXStripSettings.RGBOrder, uint8_t pin = WS2812FXStripSettings.pin, uint8_t fxoptions = WS2812FXStripSettings.fxoptions ){
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
    WS2812FXStripSettings.stripSize = stripSize;
    strcpy(WS2812FXStripSettings.RGBOrder, RGBOrder);
    WS2812FXStripSettings.pin = pin;
    WS2812FXStripSettings.fxoptions = fxoptions;
  }
 
  if (ledstates != NULL) {
    delete(ledstates);
  } 
  ledstates = new uint8_t [WS2812FXStripSettings.stripSize];

#if !defined(LED_TYPE_WS2811)
  strip = new WS2812FX(stripSize, pin, checkRGBOrder(RGBOrder) + NEO_KHZ800);
#else
  strip = new WS2812FX(stripSize, pin, checkRGBOrder(RGBOrder) + NEO_KHZ400);
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
    initDMA(stripSize);
    strip->setCustomShow(DMA_Show);
  #endif
  //parameters: index, start, stop, mode, color, speed, options
  for (uint8_t seg=0; seg < num_segments; seg++) {
    if (seg != segment) {  // Read actual segment last
      (readSegmentStateFS(seg)) ? DBG_OUTPUT_PORT.println("Segment state config FS read Success!") : DBG_OUTPUT_PORT.println("Segment state config FS read failure!");
      strip->setSegment(seg,  seg_start,  seg_stop , ws2812fx_mode, hex_colors_trans, convertSpeed(ws2812fx_speed), fxoptions);
    }
  }
  //read actual segment last to set all vars correctly
  (readSegmentStateFS(segment)) ? DBG_OUTPUT_PORT.println("Segment state config FS read Success!") : DBG_OUTPUT_PORT.println("Segment state config FS read failure!");
  strip->setSegment(segment,  seg_start,  seg_stop , ws2812fx_mode, hex_colors_trans, convertSpeed(ws2812fx_speed), fxoptions);
  
  strip->setCustomMode(0, F("Autoplay"), myCustomEffect0);
  strip->setCustomMode(1, F("Custom WS"), myCustomEffect1);
  #if defined(CUSTOM_WS2812FX_ANIMATIONS)
    strip->setCustomMode(2, F("TV"), myCustomEffect2);
    strip->setCustomMode(3, F("E1.31"),  myCustomEffect3);
    strip->setCustomMode(4, F("Fire 2012"), myCustomEffect4);
    strip->setCustomMode(5, F("Gradient"),  myCustomEffect5);
    gReverseDirection = (WS2812FXStripSettings.fxoptions & 128);
    DBG_OUTPUT_PORT.print("Number of Segments: ");
    DBG_OUTPUT_PORT.println(strip->getNumSegments());
  
    if (e131 != NULL) { delete(e131); }
    e131 = new ESPAsyncE131(END_UNIVERSE - START_UNIVERSE + 1);
    float universe_leds = 170.0;  // a universe has only 512 (0..511) channels: 3*170 or 4*128 <= 512
    if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
      //universe_leds = 128.0;
    }
    float float_enduni = stripSize/universe_leds; 
    uint8_t END_UNIVERSE = stripSize/universe_leds;
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


// Call convertColors whenever main_color, back_color or xtra_color changes.
void convertColors() {
  hex_colors_trans[0] = (uint32_t)(main_color.white << 24) | (main_color.red << 16) | (main_color.green << 8) | main_color.blue;
  hex_colors_trans[1] = (uint32_t)(back_color.white << 24) | (back_color.red << 16) | (back_color.green << 8) | back_color.blue;
  hex_colors_trans[2] = (uint32_t)(xtra_color.white << 24) | (xtra_color.red << 16) | (xtra_color.green << 8) | xtra_color.blue;
}

uint32_t* convertColors2(uint8_t w, uint8_t r, uint8_t g, uint8_t b, uint8_t w2, uint8_t r2, uint8_t g2, uint8_t b2, uint8_t w3, uint8_t r3, uint8_t g3, uint8_t b3) {
  uint32_t hexcolors[3] = {};
  hexcolors[0] = (uint32_t)(w  << 24) | (r  << 16) | (g  << 8) | b;
  hexcolors[1] = (uint32_t)(w2 << 24) | (r2 << 16) | (g2 << 8) | b2;
  hexcolors[2] = (uint32_t)(w3 << 24) | (r3 << 16) | (g3 << 8) | b3;
  return hexcolors;
}

void getSegmentParams(uint8_t seg) {
  seg_start        = strip->getSegment(seg)->start;;
  seg_stop         = strip->getSegment(seg)->stop;;
  ws2812fx_mode    = strip->getMode(seg);
  main_color.white = ((strip->getColors(seg)[0] >> 24) & 0xFF);
  main_color.red   = ((strip->getColors(seg)[0] >> 16) & 0xFF);
  main_color.green = ((strip->getColors(seg)[0] >>  8) & 0xFF);
  main_color.blue  = ((strip->getColors(seg)[0])  & 0xFF);
  back_color.white = ((strip->getColors(seg)[1] >> 24) & 0xFF);
  back_color.red   = ((strip->getColors(seg)[1] >> 16) & 0xFF);
  back_color.green = ((strip->getColors(seg)[1] >>  8) & 0xFF);
  back_color.blue  = ((strip->getColors(seg)[1]) & 0xFF);
  xtra_color.white = ((strip->getColors(seg)[2] >> 24) & 0xFF);
  xtra_color.red   = ((strip->getColors(seg)[2] >> 16) & 0xFF);
  xtra_color.green = ((strip->getColors(seg)[2] >>  8) & 0xFF);
  xtra_color.blue  = ((strip->getColors(seg)[2] >>  0) & 0xFF);
}

void calculateColorTransitionSteps() {
  //compare all colors and calculate steps
  trans_cnt_max = 0;
  int     calculate_max[4] = {};
  for (uint8_t i=0; i<3; i++){
    for (uint8_t j=0; j<4; j++) {
      calculate_max[j] = ((hex_colors[i] >> ((3-j)*8)) & 0xFF) - ((hex_colors_trans[i] >> ((3-j)*8)) & 0xFF);
      calculate_max[j] = abs(calculate_max[j]);
      trans_cnt_max = max(trans_cnt_max, calculate_max[j]);
    }
  }
}

void convertColorsFade() {
  if (transEffect) {
      if (trans_cnt > 1) {
        memcpy(hex_colors, strip->getColors(segment), sizeof(hex_colors));
        DBG_OUTPUT_PORT.println("Color transistion aborted. Restarting...!");
        trans_cnt = 1;
      }
    calculateColorTransitionSteps();
  }
}

uint32_t scale_wrgb(uint32_t wrgb, uint8_t level) {
    uint8_t w = ((wrgb >> 24) & 0xFF) * level / 255;
    uint8_t r = ((wrgb >> 16) & 0xFF) * level / 255;
    uint8_t g = ((wrgb >>  8) & 0xFF) * level / 255;
    uint8_t b = ((wrgb) & 0xFF)       * level / 255;
    return (w << 24) | (r << 16) | (g << 8) | b;
}

uint32_t trans(uint32_t newcolor, uint32_t oldcolor, uint8_t level) {
  level = (level * (255/trans_cnt_max));
  newcolor = scale_wrgb(newcolor, level);
  oldcolor = scale_wrgb(oldcolor, 255-level);
  return newcolor + oldcolor;
}
