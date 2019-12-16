// ***************************************************************************
// Request handlers
// ***************************************************************************

// Prototypes
void   handleAutoStart();
char * listStatusJSON();
#if defined(ENABLE_STATE_SAVE)
  bool   writeConfigFS(bool);
  void   tickerSaveConfig();
#endif

#if defined(ENABLE_E131)
void handleE131(){
  if (!e131->isEmpty())
  {
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
    if (NUMLEDS <= multipacketOffset) return;
    uint16_t len = (128 + multipacketOffset > WS2812FXStripSettings.stripSize) ? (WS2812FXStripSettings.stripSize - multipacketOffset) : 128;
  #else*/
    uint16_t multipacketOffset = (universe - START_UNIVERSE) * 170; //if more than 170 LEDs * 3 colors = 510 channels, client will send in next higher universe
    if (WS2812FXStripSettings.stripSize <= multipacketOffset) return;
    uint16_t len = (170 + multipacketOffset > WS2812FXStripSettings.stripSize) ? (WS2812FXStripSettings.stripSize - multipacketOffset) : 170;
/*  #endif */
    for (uint16_t i = 0; i < len; i++){
      uint16_t j = i * 3;
/*  #if defined(RGBW)
      strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], data[j + 3]);
  #else */
      strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], 0);
/*  #endif */
    }
    strip->show();
  }
}
#endif

// Call convertColors whenever main_color, back_color or xtra_color changes.
void convertColors() {
  hex_colors[0] = (uint32_t)(main_color.white << 24) | (main_color.red << 16) | (main_color.green << 8) | main_color.blue;
  hex_colors[1] = (uint32_t)(back_color.white << 24) | (back_color.red << 16) | (back_color.green << 8) | back_color.blue;
  hex_colors[2] = (uint32_t)(xtra_color.white << 24) | (xtra_color.red << 16) | (xtra_color.green << 8) | xtra_color.blue;
}

void getArgs() {
  
  if (mode == SET_ALL || mode == SET_COLOR) {
    if (server.arg("rgb") != "") {
      uint32_t rgb = (uint32_t) strtoul(server.arg("rgb").c_str(), NULL, 16);
      main_color.white = ((rgb >> 24) & 0xFF);
      main_color.red = ((rgb >> 16) & 0xFF);
      main_color.green = ((rgb >> 8) & 0xFF);
      main_color.blue = ((rgb >> 0) & 0xFF);
    } else {
      if ((server.arg("r") != "") && (server.arg("r").toInt() >= 0) && (server.arg("r").toInt() <= 255)) { 
        main_color.red = server.arg("r").toInt();
      }
      if ((server.arg("g") != "") && (server.arg("g").toInt() >= 0) && (server.arg("g").toInt() <= 255)) {
        main_color.green = server.arg("g").toInt();
      }
      if ((server.arg("b") != "") && (server.arg("b").toInt() >= 0) && (server.arg("b").toInt() <= 255)) {
        main_color.blue = server.arg("b").toInt();
      }
      if ((server.arg("w") != "") && (server.arg("w").toInt() >= 0) && (server.arg("w").toInt() <= 255)){
        main_color.white = server.arg("w").toInt();
      }
    }
    if (server.arg("rgb2") != "") {
      uint32_t rgb2 = (uint32_t) strtoul(server.arg("rgb2").c_str(), NULL, 16);
      back_color.white = ((rgb2 >> 24) & 0xFF);
      back_color.red = ((rgb2 >> 16) & 0xFF);
      back_color.green = ((rgb2 >> 8) & 0xFF);
      back_color.blue = ((rgb2 >> 0) & 0xFF);
    } else {
      if ((server.arg("r2") != "") && (server.arg("r2").toInt() >= 0) && (server.arg("r2").toInt() <= 255)) { 
        back_color.red = server.arg("r2").toInt();
      }
      if ((server.arg("g2") != "") && (server.arg("g2").toInt() >= 0) && (server.arg("g2").toInt() <= 255)) {
        back_color.green = server.arg("g2").toInt();
      }
      if ((server.arg("b2") != "") && (server.arg("b2").toInt() >= 0) && (server.arg("b2").toInt() <= 255)) {
        back_color.blue = server.arg("b2").toInt();
      }
      if ((server.arg("w2") != "") && (server.arg("w2").toInt() >= 0) && (server.arg("w2").toInt() <= 255)){
        back_color.white = server.arg("w2").toInt();
      }
    }
    if (server.arg("rgb3") != "") {
      uint32_t rgb3 = (uint32_t) strtoul(server.arg("rgb3").c_str(), NULL, 16);
      xtra_color.white = ((rgb3 >> 24) & 0xFF);
      xtra_color.red = ((rgb3 >> 16) & 0xFF);
      xtra_color.green = ((rgb3 >> 8) & 0xFF);
      xtra_color.blue = ((rgb3 >> 0) & 0xFF);
    } else {
      if ((server.arg("r3") != "") && (server.arg("r3").toInt() >= 0) && (server.arg("r3").toInt() <= 255)) { 
        xtra_color.red = server.arg("r3").toInt();
      }
      if ((server.arg("g3") != "") && (server.arg("g3").toInt() >= 0) && (server.arg("g3").toInt() <= 255)) {
        xtra_color.green = server.arg("g3").toInt();
      }
      if ((server.arg("b3") != "") && (server.arg("b3").toInt() >= 0) && (server.arg("b3").toInt() <= 255)) {
        xtra_color.blue = server.arg("b3").toInt();
      }
      if ((server.arg("w3") != "") && (server.arg("w3").toInt() >= 0) && (server.arg("w3").toInt() <= 255)){
        xtra_color.white = server.arg("w3").toInt();
      }
    }
    main_color.red = constrain(main_color.red, 0, 255);
    main_color.green = constrain(main_color.green, 0, 255);
    main_color.blue = constrain(main_color.blue, 0, 255);
    main_color.white = constrain(main_color.white, 0, 255);
    back_color.red = constrain(back_color.red, 0, 255);
    back_color.green = constrain(back_color.green, 0, 255);
    back_color.blue = constrain(back_color.blue, 0, 255);
    back_color.white = constrain(back_color.white, 0, 255);
    xtra_color.red = constrain(xtra_color.red, 0, 255);
    xtra_color.green = constrain(xtra_color.green, 0, 255);
    xtra_color.blue = constrain(xtra_color.blue, 0, 255);
    xtra_color.white = constrain(xtra_color.white, 0, 255);
    convertColors();
  }
  if (mode == SET_ALL || mode == SET_SPEED || mode == TV) {
    if ((server.arg("s") != "") && (server.arg("s").toInt() >= 0) && (server.arg("s").toInt() <= 255)) {
    ws2812fx_speed = constrain(server.arg("s").toInt(), 0, 255);
    }
  }
  if (mode == SET_ALL || mode == SET_MODE) {
    if ((server.arg("m") != "") && (server.arg("m").toInt() >= 0) && (server.arg("m").toInt() <= strip->getModeCount())) {
      ws2812fx_mode = constrain(server.arg("m").toInt(), 0, strip->getModeCount() - 1);
    }
  }
  if (mode == SET_ALL || mode == SET_BRIGHTNESS || mode == AUTO || mode == TV || mode == E131) {
    if ((server.arg("c") != "") && (server.arg("c").toInt() >= 0) && (server.arg("c").toInt() <= 100)) {
      brightness = constrain((int) server.arg("c").toInt() * 2.55, 0, 255);
    } else if ((server.arg("p") != "") && (server.arg("p").toInt() >= 0) && (server.arg("p").toInt() <= 255)) {
      brightness = constrain(server.arg("p").toInt(), 0, 255);
    }
  }
  
  DBG_OUTPUT_PORT.printf("Get Args: %s\r\n", listStatusJSON());
}

uint16_t convertSpeed(uint8_t mcl_speed) {
  //long ws2812_speed = mcl_speed * 256;
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = SPEED_MAX - ws2812_speed;
  if (ws2812_speed < SPEED_MIN) {
    ws2812_speed = SPEED_MIN;
  }
  if (ws2812_speed > SPEED_MAX) {
    ws2812_speed = SPEED_MAX;
  }
  return ws2812_speed;
}

// ***************************************************************************
// Handler functions for WS and MQTT
// ***************************************************************************
void handleSetMainColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[1], NULL, 16);
  main_color.white = ((rgb >> 24) & 0xFF);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
  mode = SET_COLOR;
}

void handleSetBackColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[2], NULL, 16);
  back_color.white = ((rgb >> 24) & 0xFF);
  back_color.red = ((rgb >> 16) & 0xFF);
  back_color.green = ((rgb >> 8) & 0xFF);
  back_color.blue = ((rgb >> 0) & 0xFF);
  mode = SET_COLOR;
}
void handleSetXtraColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[3], NULL, 16);
  xtra_color.white = ((rgb >> 24) & 0xFF);
  xtra_color.red = ((rgb >> 16) & 0xFF);
  xtra_color.green = ((rgb >> 8) & 0xFF);
  xtra_color.blue = ((rgb >> 0) & 0xFF);
  mode = SET_COLOR;
}

void handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[1], NULL, 16);
  main_color.white = ((rgb >> 24) & 0xFF);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
  DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: R: [%u] G: [%u] B: [%u] W: [%u]\r\n", main_color.red, main_color.green, main_color.blue, main_color.white);
  ws2812fx_mode = FX_MODE_STATIC;
  mode = SET_ALL;
}

void handleSetSingleLED(uint8_t * mypayload, uint8_t firstChar = 0) {
  // decode led index
  char templed[3];
  strncpy (templed, (const char *) &mypayload[firstChar], 2 );
  templed[2] = 0x00;
  uint8_t led = atoi(templed);

  DBG_OUTPUT_PORT.printf("led value: [%i]. Entry threshold: <= [%i] (=> %s)\r\n", led, WS2812FXStripSettings.stripSize, mypayload );
  if (led <= WS2812FXStripSettings.stripSize) {
    char redhex[3];
    char greenhex[3];
    char bluehex[3];
    char whitehex[3];
    strncpy (whitehex, (const char *) &mypayload[2 + firstChar], 2 );
    strncpy (redhex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[6 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[8 + firstChar], 2 );
    whitehex[2] = 0x00;
    redhex[2] = 0x00;
    greenhex[2] = 0x00;
    bluehex[2] = 0x00;
    /*
    ledstates[led].red =   strtol(redhex, NULL, 16);
    ledstates[led].green = strtol(greenhex, NULL, 16);
    ledstates[led].blue =  strtol(bluehex, NULL, 16);
    ledstates[led].white =  strtol(whitehex, NULL, 16);
    DBG_OUTPUT_PORT.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s] rgb.white: [%s]\r\n", redhex, greenhex, bluehex, whitehex);
    DBG_OUTPUT_PORT.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i] rgb.white: [%i]\r\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16), strtol(whitehex, NULL, 16));
    DBG_OUTPUT_PORT.printf("WS: Set single led [%i] to [%i] [%i] [%i] [%i] (%s)!\r\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, ledstates[led].white, mypayload);
    strip->setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, ledstates[led].white);
    */
    LEDState color;
    color.red =   strtol(redhex, NULL, 16);
    color.green = strtol(greenhex, NULL, 16);
    color.blue =  strtol(bluehex, NULL, 16);
    color.white =  strtol(whitehex, NULL, 16);
    //DBG_OUTPUT_PORT.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s] rgb.white: [%s]\r\n", redhex, greenhex, bluehex, whitehex);
    //DBG_OUTPUT_PORT.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i] rgb.white: [%i]\r\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16), strtol(whitehex, NULL, 16));
    //DBG_OUTPUT_PORT.printf("WS: Set single led [%i] to [%i] [%i] [%i] [%i] (%s)!\r\n", led, color.red, color.green, color.blue, color.white, mypayload);
    strip->setPixelColor(led, color.red, color.green, color.blue, color.white);
    strip->show();
  }
  mode = CUSTOM;
}

void handleSetDifferentColors(uint8_t * mypayload) {
  uint8_t* nextCommand = 0;
  nextCommand = (uint8_t*) strtok((char*) mypayload, "+");
  while (nextCommand) {
    handleSetSingleLED(nextCommand, 0);
    nextCommand = (uint8_t*) strtok(NULL, "+");
  }
}

void handleRangeDifferentColors(uint8_t * mypayload) {
  uint8_t* nextCommand = 0;
  nextCommand = (uint8_t*) strtok((char*) mypayload, "R");
  // While there is a range to process R0110<00ff00>

  while (nextCommand) {
    // Loop for each LED.
    char startled[4];
    char endled[4];
    char colorval[9];
    strncpy ( startled, (const char *) &nextCommand[0], 2 );
    startled[3] = 0x00;
    strncpy ( endled, (const char *) &nextCommand[2], 2 );
    endled[3] = 0x00;
    strncpy ( colorval, (const char *) &nextCommand[4], 8 );
    colorval[8] = 0x00;
    uint8_t rangebegin = atoi(startled);
    uint8_t rangeend = atoi(endled);
    DBG_OUTPUT_PORT.printf("Setting RANGE from [%i] to [%i] as color [%s]\r\n", rangebegin, rangeend, colorval);

    while ( rangebegin <= rangeend ) {
      char rangeData[11];
      snprintf(rangeData, sizeof(rangeData), "%02d%s", rangebegin, colorval);
      rangeData[sizeof(rangeData) - 1] = 0x00;
      // Set one LED
      handleSetSingleLED((uint8_t*) rangeData, 0);
      rangebegin++;
    }

    // Next Range at R
    nextCommand = (uint8_t*) strtok(NULL, "R");
  }
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

bool setConfByConfString(String saved_conf_string) {
  if (getValue(saved_conf_string, '|', 0) == "CNF") {
    DBG_OUTPUT_PORT.printf("Parsed conf: %s\r\n", saved_conf_string.c_str());
    getValue(saved_conf_string, '|', 1).toCharArray(HOSTNAME, 64);
  #if defined(ENABLE_MQTT)
    getValue(saved_conf_string, '|', 2).toCharArray(mqtt_host, 64);
    mqtt_port = getValue(saved_conf_string, '|', 3).toInt();
    getValue(saved_conf_string, '|', 4).toCharArray(mqtt_user, 32);
    getValue(saved_conf_string, '|', 5).toCharArray(mqtt_pass, 32);
  #endif
    WS2812FXStripSettings.stripSize = constrain(getValue(saved_conf_string, '|', 6).toInt(), 1, MAXLEDS);
    checkPin(getValue(saved_conf_string, '|', 7).toInt());
    char tmp_rgbOrder[5];
    getValue(saved_conf_string, '|', 8).toCharArray(tmp_rgbOrder, 4);
    checkRGBOrder(tmp_rgbOrder);
    WS2812FXStripSettings.fxoptions = constrain(((getValue(saved_conf_string, '|', 9).toInt()>>1)<<1), 0, 255);
    return true;
  } else {
    DBG_OUTPUT_PORT.println("Saved conf not found!");
    return false;
  }
  return false;
}



bool setModeByStateString(String saved_state_string) {
  if (getValue(saved_state_string, '|', 0) == "STA") {
    DBG_OUTPUT_PORT.printf("Parsed state: %s\r\n", saved_state_string.c_str());
    String str_mode = getValue(saved_state_string, '|', 1);
    mode = static_cast<MODE>(str_mode.toInt());
    String str_ws2812fx_mode = getValue(saved_state_string, '|', 2);
    ws2812fx_mode = str_ws2812fx_mode.toInt();
    String str_ws2812fx_speed = getValue(saved_state_string, '|', 3);
    ws2812fx_speed = str_ws2812fx_speed.toInt();
    String str_brightness = getValue(saved_state_string, '|', 4);
    brightness = str_brightness.toInt();
    String str_red = getValue(saved_state_string, '|', 5);
    main_color.red = str_red.toInt();
    String str_green = getValue(saved_state_string, '|', 6);
    main_color.green = str_green.toInt();
    String str_blue = getValue(saved_state_string, '|', 7);
    main_color.blue = str_blue.toInt();
    String str_white = getValue(saved_state_string, '|', 8);
    main_color.white = str_white.toInt();
    str_red = getValue(saved_state_string, '|', 9);
    back_color.red = str_red.toInt();
    str_green = getValue(saved_state_string, '|', 10);
    back_color.green = str_green.toInt();
    str_blue = getValue(saved_state_string, '|', 11);
    back_color.blue = str_blue.toInt();
    str_white = getValue(saved_state_string, '|', 12);
    back_color.white = str_white.toInt();
    str_red = getValue(saved_state_string, '|', 13);
    xtra_color.red = str_red.toInt();
    str_green = getValue(saved_state_string, '|', 14);
    xtra_color.green = str_green.toInt();
    str_blue = getValue(saved_state_string, '|', 15);
    xtra_color.blue = str_blue.toInt();
    str_white = getValue(saved_state_string, '|', 16);
    xtra_color.white = str_white.toInt();
    convertColors();
    DBG_OUTPUT_PORT.print("Set to state: ");
    DBG_OUTPUT_PORT.println(listStatusJSON());
    return true;
  } else {
    DBG_OUTPUT_PORT.println("Saved conf not found!");
    return false;    
  }
  return false;
}

#if defined(ENABLE_LEGACY_ANIMATIONS)
void handleSetNamedMode(uint8_t * mypayload) {
    if (strcmp((char *) &mypayload[1], "off") == 0) {
      mode = OFF;
    }
    
  #if defined(ENABLE_TV)
    if (strcmp((char *) &mypayload[1], "tv") == 0) {
      mode = TV;
    }
  #endif

  #if defined(ENABLE_E131)
    if (strcmp((char *) &mypayload[1], "e131") == 0) {
      mode = E131;
    }
  #endif
  
    if (strcmp((char *) &mypayload[1], "auto") == 0) {
      mode = AUTO;
    }
    if (strcmp((char *) &mypayload[1], "all") == 0) {
      ws2812fx_mode = FX_MODE_STATIC;
      mode = SET_ALL;
    }
    if (strcmp((char *) &mypayload[1], "wipe") == 0) {
      ws2812fx_mode = FX_MODE_COLOR_WIPE;
      mode = SET_MODE;
    }
    if (strcmp((char *) &mypayload[1], "rainbow") == 0) {
      ws2812fx_mode = FX_MODE_RAINBOW;
      mode = SET_MODE;
    }
    if (strcmp((char *) &mypayload[1], "rainbowCycle") == 0) {
      ws2812fx_mode = FX_MODE_RAINBOW_CYCLE;
      mode = SET_MODE;
    }
    if (strcmp((char *) &mypayload[1], "theaterchase") == 0) {
      ws2812fx_mode = FX_MODE_THEATER_CHASE;
      mode = SET_MODE;
    }
    if (strcmp((char *) &mypayload[1], "twinkleRandom") == 0) {
      ws2812fx_mode = FX_MODE_TWINKLE_RANDOM;
      mode = SET_MODE;
    }
    if (strcmp((char *) &mypayload[1], "theaterchaseRainbow") == 0) {
      ws2812fx_mode = FX_MODE_THEATER_CHASE_RAINBOW;
      mode = SET_MODE;
    }
}
#endif

void handleSetWS2812FXMode(uint8_t * mypayload) {
  if (isDigit(mypayload[1])) {
    ws2812fx_mode = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
    ws2812fx_mode = constrain(ws2812fx_mode, 0, strip->getModeCount() - 1);
    mode = SET_MODE;
  } else  {
    if (strcmp((char *) &mypayload[1], "off") == 0) {
      mode = OFF;
    }
    
    if (strcmp((char *) &mypayload[1], "auto") == 0) {
      mode = AUTO;
    }
    
    #if defined(ENABLE_TV)
      if (strcmp((char *) &mypayload[1], "tv") == 0) {
        mode = TV;
      }
    #endif

    #if defined(ENABLE_E131)
      if (strcmp((char *) &mypayload[1], "e131") == 0) {
        mode = E131;
      }
    #endif
      if (strcmp((char *) &mypayload[1], "custom") == 0) {
        mode = CUSTOM;
      }
  }    
}

char * listStatusJSON() {
  //uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip->getMode(); 
  const size_t bufferSize = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(6) + 500;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["mode"] = (uint8_t) mode;
  root["ws2812fx_mode"] = ws2812fx_mode;
  root["ws2812fx_mode_name"] = strip->getModeName(ws2812fx_mode);
  //root["ws2812fx_mode"] = tmp_mode;
  //root["ws2812fx_mode_name"] = strip->getModeName(tmp_mode);
  root["speed"] = ws2812fx_speed;
  root["brightness"] = brightness;
  JsonArray color = root.createNestedArray("color");
  color.add(main_color.white);
  color.add(main_color.red);
  color.add(main_color.green);
  color.add(main_color.blue);
  color.add(back_color.white);
  color.add(back_color.red);
  color.add(back_color.green);
  color.add(back_color.blue);
  color.add(xtra_color.white);
  color.add(xtra_color.red);
  color.add(xtra_color.green);
  color.add(xtra_color.blue);
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getStatusJSON() {
  char * buffer = listStatusJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

char * listConfigJSON() {
  //uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip->getMode(); 
  #if defined(ENABLE_MQTT)
    const size_t bufferSize = JSON_OBJECT_SIZE(9) + 500;
  #else
    const size_t bufferSize = JSON_OBJECT_SIZE(5) + 150;
  #endif
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["hostname"] = HOSTNAME;
  #if defined(ENABLE_MQTT)
    root["mqtt_host"] = mqtt_host;
    root["mqtt_port"] = mqtt_port;
    root["mqtt_user"] = mqtt_user;
    root["mqtt_pass"] = mqtt_pass;
  #endif
  root["ws_cnt"]    = WS2812FXStripSettings.stripSize;
  root["ws_rgbo"]   = WS2812FXStripSettings.RGBOrder;
  root["ws_pin"]    = WS2812FXStripSettings.pin;
  root["ws_fxopt"]  = WS2812FXStripSettings.fxoptions;
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getConfigJSON() {
  char * buffer = listConfigJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

char * listModesJSON() {
  const size_t bufferSize = JSON_ARRAY_SIZE(strip->getModeCount() + 3) + (strip->getModeCount() + 3)*JSON_OBJECT_SIZE(2) + 2000;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonArray root = jsonBuffer.to<JsonArray>();
  JsonObject objectoff = root.createNestedObject();
  objectoff["mode"] = "off";
  objectoff["name"] = "OFF";
  #if defined(ENABLE_TV)
  JsonObject objecttv = root.createNestedObject();
  objecttv["mode"] = "tv";
  objecttv["name"] = "TV";
  #endif
  #if defined(ENABLE_E131)
  JsonObject objecte131 = root.createNestedObject();
  objecte131["mode"] = "e131";
  objecte131["name"] = "E131";
  #endif
  JsonObject objectcustom = root.createNestedObject();
  objectcustom["mode"] = "custom";
  objectcustom["name"] = "CUSTOM WS";
  for (uint8_t i = 0; i < strip->getModeCount(); i++) {
    JsonObject object = root.createNestedObject();
    object["mode"] = i;
    object["name"] = strip->getModeName(i);
  }
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getModesJSON() {
  char * buffer = listModesJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

// ***************************************************************************
// HTTP request handlers
// ***************************************************************************
void handleMinimalUpload() {
  char message[] = "<!DOCTYPE html>\
    <html>\
      <head>\
        <title>ESP8266 Upload</title>\
        <meta charset=\"utf-8\">\
        <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\
        <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
      </head>\
      <body>\
        <form action=\"/edit\" method=\"post\" enctype=\"multipart/form-data\">\
          <input type=\"file\" name=\"data\">\
          <input type=\"text\" name=\"path\" value=\"/\">\
          <button>Upload</button>\
         </form>\
      </body>\
    </html>";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "text/html", message );
}

void handleNotFound() {
  String message = "File Not Found\r\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}

// ***************************************************************************
// Functions and variables for automatic cycling
// ***************************************************************************
Ticker autoTicker;
uint8_t autoCount = 0;

void autoTick() {
  uint32_t setcolors[] = {autoParams[autoCount][0],autoParams[autoCount][1],autoParams[autoCount][2]};
  strip->setColors(0, setcolors);
  strip->setSpeed(convertSpeed((uint8_t)autoParams[autoCount][3]));
  strip->setMode((uint8_t)autoParams[autoCount][4]);
  autoTicker.once_ms((uint32_t)autoParams[autoCount][5], autoTick);
  DBG_OUTPUT_PORT.print("autoTick ");
    DBG_OUTPUT_PORT.printf("autoTick[%d]: {0x%06x, %d, %d, %d}\r\n", autoCount, autoParams[autoCount][0], (uint8_t)autoParams[autoCount][1], (uint8_t)autoParams[autoCount][2], (uint32_t)autoParams[autoCount][3], (uint32_t)autoParams[autoCount][4], (uint32_t)autoParams[autoCount][5]);

  autoCount++;
  if (autoCount >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount = 0;
}

void handleAutoStart() {
  DBG_OUTPUT_PORT.println("Starting AUTO mode."); 
  autoCount = 0;
  autoTick();
}

void handleAutoStop() {
    DBG_OUTPUT_PORT.println("Stopping AUTO mode."); 
    autoTicker.detach();
}

// ***************************************************************************
// Functions and variables 
// ***************************************************************************
void Dbg_Prefix(bool mqtt, uint8_t num) {
  if (mqtt == true)  {
    DBG_OUTPUT_PORT.print("MQTT: "); 
  } else {
    DBG_OUTPUT_PORT.print("WS: ");
    webSocket.sendTXT(num, "OK");
  }
}

void checkpayload(uint8_t * payload, bool mqtt = false, uint8_t num = 0) {
  // # ==> Set main color - ## ==> Set 2nd color - ### ==> Set 3rd color
  if (payload[0] == '#') {
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", payload);
    #endif
    if (payload[2] == '#') {
      handleSetXtraColor(payload);
      DBG_OUTPUT_PORT.printf("Set 3rd color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n",  xtra_color.red, xtra_color.green, xtra_color.blue, xtra_color.white);
    } else if (payload[1] == '#') {
      handleSetBackColor(payload);
      DBG_OUTPUT_PORT.printf("Set 2nd color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n",  back_color.red, back_color.green, back_color.blue, back_color.white);
    } else {
      handleSetMainColor(payload);
      DBG_OUTPUT_PORT.printf("Set main color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n", main_color.red, main_color.green, main_color.blue, main_color.white);
    }
  }

  // ? ==> Set speed
  if (payload[0] == '?') {
    uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    ws2812fx_speed = constrain(d, 0, 255);
    mode = SET_SPEED;
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set speed to: [%u]\r\n", ws2812fx_speed);
  }

  // % ==> Set brightness
  if (payload[0] == '%') {
    uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    brightness = constrain(b, 0, 255);
    mode = SET_BRIGHTNESS;
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set brightness to: [%u]\r\n", brightness);
  }

  // * ==> Set main color and light all LEDs (Shortcut)
  if (payload[0] == '*') {
    handleSetAllMode(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set main color and light all LEDs [%s]\r\n", payload);
  }

  // ! ==> Set single LED in given color
  if (payload[0] == '!') {
    handleSetSingleLED(payload, 1);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set single LED in given color [%s]\r\n", payload);
  }

  // + ==> Set multiple LED in the given colors
  if (payload[0] == '+') {
    handleSetDifferentColors(payload);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set multiple LEDs in given color [%s]\r\n", payload);
  }

  // + ==> Set range of LEDs in the given color
  if (payload[0] == 'R') {
    handleRangeDifferentColors(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set range of LEDs in given color [%s]\r\n", payload);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", payload);
    #endif
  }

  #if defined(ENABLE_LEGACY_ANIMATIONS)
    // = ==> Activate named mode
    if (payload[0] == '=') {
      // we get mode data
      handleSetNamedMode(payload);
      Dbg_Prefix(mqtt, num);
      DBG_OUTPUT_PORT.printf("Activated mode [%u]!\r\n", mode);
    }
  #endif

  // $ ==> Get status Info.
  if (payload[0] == '$') {
    char * buffer = listStatusJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: ");
      #if defined(ENABLE_MQTT)
        #if ENABLE_MQTT == 0
          mqtt_client->publish(mqtt_outtopic, buffer);
        #endif
        #if ENABLE_MQTT == 1
          mqtt_client->publish(mqtt_outtopic, qospub, false, buffer);
        #endif
      #endif
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
      webSocket.sendTXT(num, buffer);
    }
    DBG_OUTPUT_PORT.printf("Get status info: %s\r\n", buffer);
    free (buffer);
  }


    // $ ==> Get config Info.
  if (payload[0] == 'C') {
    bool updateStrip = false;
    bool updateConf  = false;
    if (payload[1] == 's') {
      if (payload[2] == 'c') {
        char tmp_count[6];
        snprintf(tmp_count, sizeof(tmp_count), "%s", &payload[3]);
        tmp_count[5] = 0x00;
        WS2812FXStripSettings.stripSize = constrain(atoi(tmp_count), 1, MAXLEDS);
        updateStrip = true;
      }
      if (payload[2] == 'r') {     
        char tmp_rgbOrder[5];
        snprintf(tmp_rgbOrder, sizeof(tmp_rgbOrder), "%s", &payload[3]);
        tmp_rgbOrder[4] = 0x00;
        checkRGBOrder(tmp_rgbOrder);
        updateStrip=true;    
      }
    #if !defined(USE_WS2812FX_DMA)
      if (payload[2] == 'p') {
        char tmp_pin[3];
        snprintf(tmp_pin, sizeof(tmp_pin), "%s", &payload[3]);
        tmp_pin[2] = 0x00;
        checkPin(atoi(tmp_pin));
        updateStrip = true;
      }
    #endif
      if (payload[2] == 'o') {
         char tmp_fxoptions[4];
         snprintf(tmp_fxoptions, sizeof(tmp_fxoptions), "%s", &payload[3]);
         tmp_fxoptions[3] = 0x00;
         WS2812FXStripSettings.fxoptions = ((constrain(atoi(tmp_fxoptions), 0, 255)>>1)<<1);
         updateStrip = true;
      }        
    }
    if (updateStrip){
      mode = INIT_STRIP;   
    }
    if (payload[1] == 'h') {
      snprintf(HOSTNAME, sizeof(HOSTNAME), "%s", &payload[2]);
      HOSTNAME[sizeof(HOSTNAME) - 1] = 0x00;
      updateConf = true;
    }
  #if defined(ENABLE_MQTT)
    if (payload[1] == 'm') {
      if (payload[2] == 'h') {
        snprintf(mqtt_host, sizeof(mqtt_host), "%s", &payload[3]);
        mqtt_host[sizeof(mqtt_host) - 1] = 0x00;
        updateConf = true;
      }
      if (payload[2] == 'p') {
        char tmp_port[6];
        snprintf(tmp_port, sizeof(tmp_port), "%s", &payload[3]);
        tmp_port[sizeof(tmp_port) - 1] = 0x00;
        mqtt_port = constrain(atoi(tmp_port), 0, 65535);
        updateConf = true;
      }
      if (payload[2] == 'u') {
        snprintf(mqtt_user, sizeof(mqtt_user), "%s", &payload[3]);
        mqtt_user[sizeof(mqtt_user) - 1] = 0x00;
        updateConf = true;
      }
      if (payload[2] == 'w') {
        snprintf(mqtt_pass, sizeof(mqtt_pass), "%s", &payload[3]);
        mqtt_pass[sizeof(mqtt_pass) - 1] = 0x00;
        updateConf = true;
      }    
    }
    if (updateConf) {
      initMqtt();
    }    
  #endif   

    char * buffer = listConfigJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: ");
      #if defined(ENABLE_MQTT)
        #if ENABLE_MQTT == 0
          mqtt_client->publish(mqtt_outtopic, buffer);
        #endif
        #if ENABLE_MQTT == 1
          mqtt_client->publish(mqtt_outtopic, qospub, false, buffer);
        #endif
      #endif
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
      webSocket.sendTXT(num, buffer);
    } 
#if defined(ENABLE_STATE_SAVE)
    if (updateStrip || updateConf) {
      DBG_OUTPUT_PORT.println("Saving config.json!");
      if(!settings_save_conf.active()) settings_save_conf.once(3, tickerSaveConfig);
    }
#endif
    updateStrip = false;
    updateConf  = false;
    DBG_OUTPUT_PORT.printf("Get status info: %s\r\n", buffer);
    free (buffer);
  }

  // ~ ==> Get WS2812 modes.
  if (payload[0] == '~') {
    char * buffer  = listModesJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
      #if defined(ENABLE_MQTT)
        #if ENABLE_MQTT == 0
          uint16_t msg_len = strlen(buffer) + 1;
          mqtt_client->beginPublish(mqtt_outtopic, msg_len, true);
          mqtt_client->write((const uint8_t*)buffer, msg_len);
          mqtt_client->endPublish();       
        #endif
        #if ENABLE_MQTT == 1
          mqtt_client->publish(mqtt_outtopic, qospub, false, buffer);
        #endif  
      #endif
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
      webSocket.sendTXT(num, buffer);
    }
    DBG_OUTPUT_PORT.println("Get WS2812 modes.");
    DBG_OUTPUT_PORT.println(buffer);
    free (buffer);
  }

  // / ==> Set WS2812 mode.
  if (payload[0] == '/') {
    handleSetWS2812FXMode(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set WS2812 mode: [%s]\r\n", payload);
  }
}

// ***************************************************************************
// WS request handlers
// ***************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      DBG_OUTPUT_PORT.printf("WS: [%u] Disconnected!\r\n", num);
      break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_OUTPUT_PORT.printf("WS: [%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      DBG_OUTPUT_PORT.printf("WS: [%u] get Text: %s\r\n", num, payload);

      checkpayload(payload, false, num);

      // start auto cycling
      if (strcmp((char *)payload, "start") == 0 ) {
        mode = AUTO;
        webSocket.sendTXT(num, "OK");
      }

      // stop auto cycling
      if (strcmp((char *)payload, "stop") == 0 ) {
        mode = SET_ALL;
        webSocket.sendTXT(num, "OK");
      }
      break;
  }
}

// ***************************************************************************
// MQTT callback / connection handler
// ***************************************************************************
#if defined(ENABLE_MQTT)

  #if defined(ENABLE_HOMEASSISTANT)
     void tickerSendState(){
       new_ha_mqtt_msg = true;
     }

     LEDState temp2rgb(uint16_t kelvin) {
       uint16_t tmp_internal = kelvin / 100.0;
       LEDState tmp_color;

       // red
       if (tmp_internal <= 66) {
         tmp_color.red = 255;
       } else {
         float tmp_red = 329.698727446 * pow(tmp_internal - 60, -0.1332047592);
         if (tmp_red < 0) {
           tmp_color.red = 0;
         } else if (tmp_red > 255) {
           tmp_color.red = 255;
         } else {
           tmp_color.red = tmp_red;
         }
       }

       // green
       if (tmp_internal <= 66) {
         float tmp_green = 99.4708025861 * log(tmp_internal) - 161.1195681661;
         if (tmp_green < 0) {
           tmp_color.green = 0;
         } else if (tmp_green > 255) {
           tmp_color.green = 255;
         } else {
           tmp_color.green = tmp_green;
         }
       } else {
         float tmp_green = 288.1221695283 * pow(tmp_internal - 60, -0.0755148492);
         if (tmp_green < 0) {
           tmp_color.green = 0;
         } else if (tmp_green > 255) {
           tmp_color.green = 255;
         } else {
           tmp_color.green = tmp_green;
         }
       }

       // blue
       if (tmp_internal >= 66) {
         tmp_color.blue = 255;
       } else if (tmp_internal <= 19) {
         tmp_color.blue = 0;
       } else {
         float tmp_blue = 138.5177312231 * log(tmp_internal - 10) - 305.0447927307;
         if (tmp_blue < 0) {
           tmp_color.blue = 0;
         } else if (tmp_blue > 255) {
           tmp_color.blue = 255;
         } else {
           tmp_color.blue = tmp_blue;
         }
       }
       return tmp_color;
     }

     void sendState() {
       const size_t bufferSize = JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 1000;
       DynamicJsonDocument jsonBuffer(bufferSize);
       JsonObject root = jsonBuffer.to<JsonObject>();
       root["state"] = (mode != OFF) ? on_cmd : off_cmd;
       JsonObject color = root.createNestedObject("color");
       color["r"] = main_color.red;
       color["g"] = main_color.green;
       color["b"] = main_color.blue;
       color["w"] = main_color.white;
       color["r2"] = back_color.red;
       color["g2"] = back_color.green;
       color["b2"] = back_color.blue;
       color["w2"] = back_color.white;
       color["r3"] = xtra_color.red;
       color["g3"] = xtra_color.green;
       color["b3"] = xtra_color.blue;
       color["w3"] = xtra_color.white;
       if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
         root["white_value"]= main_color.white;
       }
       root["brightness"] = brightness;
       root["color_temp"] = color_temp;
       root["speed"] = ws2812fx_speed;
       //char modeName[30];
       //strncpy_P(modeName, (PGM_P)strip->getModeName(strip->getMode()), sizeof(modeName)); // copy from progmem
       #if defined(ENABLE_HOMEASSISTANT)
       if (mode == OFF){
         root["effect"] = "OFF";
       } else {
         if (mode == AUTO){
           root["effect"] = "AUTO";
         } else {
           if (mode == TV){
             root["effect"] = "TV";
           } else {
             if (mode == E131){
               root["effect"] = "E131";
             } else {
               if (mode == CUSTOM){
                 root["effect"] = "CUSTOM WS"; 
               } else {
                 root["effect"] = strip->getModeName(strip->getMode());
               }
             }
           }
         }
       }
       #endif
      char buffer[measureJson(root) + 1];
      serializeJson(root, buffer, sizeof(buffer));
      jsonBuffer.clear();
      #if ENABLE_MQTT == 0
      mqtt_client->publish(mqtt_ha_state_out, buffer, true);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\r\n", mqtt_ha_state_out, buffer);
      #endif
      #if ENABLE_MQTT == 1
      mqtt_client->publish(mqtt_ha_state_out, 1, true, buffer);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\r\n", mqtt_ha_state_out, buffer);
      #endif
      new_ha_mqtt_msg = false;
      ha_send_data.detach();
      DBG_OUTPUT_PORT.printf("Heap size: %u\r\n", ESP.getFreeHeap());
    }

    bool processJson(char* message) {
      const size_t bufferSize = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(12) + 500;
      DynamicJsonDocument jsonBuffer(bufferSize);
      DeserializationError error = deserializeJson(jsonBuffer, message);
      if (error) {
        DBG_OUTPUT_PORT.print("parseObject() failed: ");
        DBG_OUTPUT_PORT.println(error.c_str());
        jsonBuffer.clear();
        return false;
      }
      //DBG_OUTPUT_PORT.println("JSON ParseObject() done!");
      JsonObject root = jsonBuffer.as<JsonObject>();
      
      if (root.containsKey("state")) {
        const char* state_in = root["state"];
        if (strcmp(state_in, on_cmd) == 0) {
          mode = SET_ALL;
        }
        else if (strcmp(state_in, off_cmd) == 0) {
          mode = OFF;
          jsonBuffer.clear();
          return true;
        }
      }

      if (root.containsKey("color")) {
        JsonObject color = root["color"];
        main_color.red = (uint8_t) color["r"];
        main_color.green = (uint8_t) color["g"];
        main_color.blue = (uint8_t) color["b"];
        main_color.white = (uint8_t) color["w"];
        back_color.red = (uint8_t) color["r2"];
        back_color.green = (uint8_t) color["g2"];
        back_color.blue = (uint8_t) color["b2"];
        back_color.white = (uint8_t) color["w2"];
        xtra_color.red = (uint8_t) color["r3"];
        xtra_color.green = (uint8_t) color["g3"];
        xtra_color.blue = (uint8_t) color["b3"];
        xtra_color.white = (uint8_t) color["w3"];
        mode = SET_COLOR;
      }
      
      if (root.containsKey("white_value")) {
        uint8_t json_white_value = constrain((uint8_t) root["white_value"], 0, 255);
        if (json_white_value != main_color.white) {
          main_color.white = json_white_value;
          mode = SET_COLOR;
        }
      }
      
      if (root.containsKey("speed")) {
        uint8_t json_speed = constrain((uint8_t) root["speed"], 0, 255);
        if (json_speed != ws2812fx_speed) {
          ws2812fx_speed = json_speed;
          mode = SET_SPEED;
        }
      }

      if (root.containsKey("color_temp")) {
        //temp comes in as mireds, need to convert to kelvin then to RGB
        color_temp = (uint16_t) root["color_temp"];
        uint16_t kelvin  = 1000000 / color_temp;
        main_color = temp2rgb(kelvin);
        mode = SET_COLOR;
      }

      if (root.containsKey("brightness")) {
        uint8_t json_brightness = constrain((uint8_t) root["brightness"], 0, 255); //fix #224
        if (json_brightness != brightness) {
          brightness = json_brightness;
          mode = SET_BRIGHTNESS;
        }
      }

      if (root.containsKey("effect")) {
        String effectString = root["effect"].as<String>();
        #if defined(ENABLE_HOMEASSISTANT)
          if(effectString == "OFF"){
            mode = OFF;
          }
          if(effectString == "AUTO"){
            mode = AUTO;
          }
        #endif
        #if defined(ENABLE_TV) and defined(ENABLE_HOMEASSISTANT)
          if(effectString == "TV"){
            mode = TV;
          }
        #endif
        #if defined(ENABLE_E131) and defined(ENABLE_HOMEASSISTANT)
          if(effectString == "E131"){
            mode = E131;
          }
         #endif
          if(effectString == "CUSTOM WS"){
            mode = CUSTOM;
          }
        for (uint8_t i = 0; i < strip->getModeCount(); i++) {
          if(String(strip->getModeName(i)) == effectString) {
            mode = SET_MODE;
            ws2812fx_mode = i;
            break;
          }
        }
      }
      jsonBuffer.clear();
      return true;
    }
  #endif

  #if ENABLE_MQTT == 0
  void onMqttMessage(char* topic, byte* payload_in, uint16_t length) {
  #endif
  
  #if ENABLE_MQTT == 1
  void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
  #endif
    uint8_t * payload = (uint8_t *) malloc(length + 1);
    memcpy(payload, payload_in, length);
    payload[length] = 0;
    DBG_OUTPUT_PORT.printf("MQTT: Recieved [%s]: %s\r\n", topic, payload);
 
    #if defined(ENABLE_HOMEASSISTANT)
      if (strcmp(topic, mqtt_ha_state_in) == 0) {
        if (!processJson((char*)payload)) {
          return;
        }
        if(!ha_send_data.active()) ha_send_data.once(5, tickerSendState);
        } else if (strcmp(topic, mqtt_intopic) == 0) {
    #endif

    checkpayload(payload, true);

    #if defined(ENABLE_HOMEASSISTANT)
    }
    #endif
    free(payload);
  }

  #if ENABLE_MQTT == 0
  void mqtt_reconnect() {
    // Loop until we're reconnected
    while (!mqtt_client->connected() && mqtt_reconnect_retries < MQTT_MAX_RECONNECT_TRIES) {
      mqtt_reconnect_retries++;
      DBG_OUTPUT_PORT.printf("Attempting MQTT connection %d / %d ...\r\n", mqtt_reconnect_retries, MQTT_MAX_RECONNECT_TRIES);
      // Attempt to connect
      if (mqtt_client->connect(mqtt_clientid, mqtt_user, mqtt_pass, mqtt_will_topic, 2, true, mqtt_will_payload, true)) {
        DBG_OUTPUT_PORT.println("MQTT connected!");
        // Once connected, publish an announcement...
        char message[18 + strlen(HOSTNAME) + 1];
        strcpy(message, "McLighting ready: ");
        strcat(message, HOSTNAME);
        mqtt_client->publish(mqtt_outtopic, message);
        // ... and resubscribe
        mqtt_client->subscribe(mqtt_intopic, qossub);
        if(mqtt_lwt_boot_flag) {
          mqtt_client->publish(mqtt_will_topic, "ONLINE");
          //mqtt_lwt_boot_flag = false;
        }
        #if defined(ENABLE_HOMEASSISTANT)
          ha_send_data.detach();
          mqtt_client->subscribe(mqtt_ha_state_in, qossub);
          ha_send_data.once(5, tickerSendState);
          #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
            const size_t bufferSize = JSON_ARRAY_SIZE(strip->getModeCount()+ 4) + JSON_OBJECT_SIZE(11) + 1500;
            DynamicJsonDocument jsonBuffer(bufferSize);
            JsonObject root = jsonBuffer.to<JsonObject>();
            root["name"] = mqtt_clientid;
            #if defined(MQTT_HOME_ASSISTANT_0_87_SUPPORT)
            root["schema"] = "json";
            #else
            root["platform"] = "mqtt_json";
            #endif
            root["state_topic"] = mqtt_ha_state_out;
            root["command_topic"] = mqtt_ha_state_in;
            #if !defined(MQTT_HOME_ASSISTANT_0_87_SUPPORT)
            root["on_command_type"] = "first";
            #endif
            root["brightness"] = "true";
            root["rgb"] = "true";
            if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
              root["white_value"]= "true";
            }
            root["optimistic"] = "false";
            root["color_temp"] = "true";
            root["effect"] = "true";
            JsonArray effect_list = root.createNestedArray("effect_list");
            effect_list.add("OFF");
            effect_list.add("AUTO");
            #if defined(ENABLE_TV)
              effect_list.add("TV");
            #endif
            #if defined(ENABLE_E131)
               effect_list.add("E131");
            #endif
            effect_list.add("CUSTOM WS");
            for (uint8_t i = 0; i < strip->getModeCount(); i++) {
              effect_list.add(strip->getModeName(i));
            }
            // Following will never work for PubSubClient as message size > 1.6kB
            // char buffer[measureJson(json) + 1];
            // serializeJson(root, buffer, sizeof(buffer));
            // mqtt_client->publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), buffer, true);

            // Alternate way to publish large messages using PubSubClient
            uint16_t msg_len = measureJson(root) + 1;
            char buffer[msg_len];
            serializeJson(root, buffer, sizeof(buffer));
            DBG_OUTPUT_PORT.println(buffer);
            mqtt_client->beginPublish(mqtt_ha_config, msg_len-1, true);
            mqtt_client->write((const uint8_t*)buffer, msg_len-1);
            mqtt_client->endPublish();
          #endif
        #endif

        DBG_OUTPUT_PORT.printf("MQTT topic in: %s\r\n", mqtt_intopic);
        DBG_OUTPUT_PORT.printf("MQTT topic out: %s\r\n", mqtt_outtopic);
      } else {
        DBG_OUTPUT_PORT.print("failed, rc=");
        DBG_OUTPUT_PORT.print(mqtt_client->state());
        DBG_OUTPUT_PORT.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if (mqtt_reconnect_retries >= MQTT_MAX_RECONNECT_TRIES) {
      DBG_OUTPUT_PORT.printf("MQTT connection failed, giving up after %d tries ...\r\n", mqtt_reconnect_retries);
    }
  }
  #endif
  #if ENABLE_MQTT == 1
    void connectToWifi() {
      DBG_OUTPUT_PORT.println("Re-connecting to Wi-Fi...");
      WiFi.setSleepMode(WIFI_NONE_SLEEP);
      WiFi.mode(WIFI_STA);
      WiFi.hostname(HOSTNAME); 
      WiFi.begin();
    }

    void connectToMqtt() {
      DBG_OUTPUT_PORT.println("Connecting to MQTT...");
      mqtt_client->connect();
    }

    void onWifiConnect(const WiFiEventStationModeGotIP& event) {
      DBG_OUTPUT_PORT.println("Connected to Wi-Fi.");
      connectToMqtt();
    }

    void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
      DBG_OUTPUT_PORT.println("Disconnected from Wi-Fi.");
      #if defined(ENABLE_HOMEASSISTANT)
         ha_send_data.detach();
      #endif
      mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
      wifiReconnectTimer.once(2, connectToWifi);
    }

    void onMqttConnect(bool sessionPresent) {
      DBG_OUTPUT_PORT.println("Connected to MQTT.");
      DBG_OUTPUT_PORT.print("Session present: ");
      DBG_OUTPUT_PORT.println(sessionPresent);
      char message[18 + strlen(HOSTNAME) + 1];
      strcpy(message, "McLighting ready: ");
      strcat(message, HOSTNAME);
      mqtt_client->publish(mqtt_outtopic, qospub, false, message);
      //Subscribe
      uint16_t packetIdSub1 = mqtt_client->subscribe(mqtt_intopic, qossub);
      DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub1);
      if(mqtt_lwt_boot_flag) {
        mqtt_client->publish(mqtt_will_topic, qospub, false, "ONLINE");
        mqtt_lwt_boot_flag = false;
      }
      #if defined(ENABLE_HOMEASSISTANT)
        ha_send_data.detach();
        uint16_t packetIdSub2 = mqtt_client->subscribe((char *)mqtt_ha_state_in, qossub);
        DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub2);
        #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
          const size_t bufferSize = JSON_ARRAY_SIZE(strip->getModeCount()+ 4) + JSON_OBJECT_SIZE(11) + 1500;
          DynamicJsonDocument jsonBuffer(bufferSize);
          JsonObject root = jsonBuffer.to<JsonObject>();
          root["name"] = mqtt_clientid;
          #if defined(MQTT_HOME_ASSISTANT_0_87_SUPPORT)
          root["schema"] = "json";
          #else
          root["platform"] = "mqtt_json";
          #endif
          root["state_topic"] = mqtt_ha_state_out;
          root["command_topic"] = mqtt_ha_state_in;
          #if !defined(MQTT_HOME_ASSISTANT_0_87_SUPPORT)
          root["on_command_type"] = "first";
          #endif
          root["brightness"] = "true";
          root["rgb"] = "true";
          if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
            root["white_value"]= "true";
          }
          root["optimistic"] = "false";
          root["color_temp"] = "true";
          root["effect"] = "true";
          JsonArray effect_list = root.createNestedArray("effect_list");
          effect_list.add("OFF");
          effect_list.add("AUTO");
          #if defined(ENABLE_TV)
            effect_list.add("TV");
          #endif
          #if defined(ENABLE_E131)
             effect_list.add("E131");
          #endif
          effect_list.add("CUSTOM WS");
          for (uint8_t i = 0; i < strip->getModeCount(); i++) {
            effect_list.add(strip->getModeName(i));
          }
          char buffer[measureJson(root) + 1];
          serializeJson(root, buffer, sizeof(buffer));
          jsonBuffer.clear();
          mqtt_client->publish(mqtt_ha_config, qospub, true, buffer);
        #endif
      #endif
    }

    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
      DBG_OUTPUT_PORT.print("Disconnected from MQTT, reason: ");
      if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
        DBG_OUTPUT_PORT.println("Bad server fingerprint.");
      } else if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
        DBG_OUTPUT_PORT.println("TCP Disconnected.");
      } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
        DBG_OUTPUT_PORT.println("Bad server fingerprint.");
      } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
        DBG_OUTPUT_PORT.println("MQTT Identifier rejected.");
      } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
        DBG_OUTPUT_PORT.println("MQTT server unavailable.");
      } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
        DBG_OUTPUT_PORT.println("MQTT malformed credentials.");
      } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
        DBG_OUTPUT_PORT.println("MQTT not authorized.");
      } else if (reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE) {
        DBG_OUTPUT_PORT.println("Not enough space on esp8266.");
      }
      if (WiFi.isConnected()) {
        mqttReconnectTimer.once(5, connectToMqtt);
      }
    }
  #endif
#endif

// ***************************************************************************
// Button management
// ***************************************************************************
#if defined(ENABLE_BUTTON)
  void shortKeyPress() {
    DBG_OUTPUT_PORT.printf("Short button press\r\n");
    if (mode == OFF) {
      setModeByStateString(BTN_MODE_SHORT);
      prevmode = mode;
      mode = SET_ALL;
    } else {
      mode = OFF;
    }
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    DBG_OUTPUT_PORT.printf("Medium button press\r\n");
    setModeByStateString(BTN_MODE_MEDIUM);
    prevmode = mode;
    mode = SET_ALL;
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DBG_OUTPUT_PORT.printf("Long button press\r\n");
    setModeByStateString(BTN_MODE_LONG);
    prevmode = mode;
    mode = SET_ALL;
  }

  void button() {
    if (millis() - keyPrevMillis >= keySampleIntervalMs) {
      keyPrevMillis = millis();

      byte currKeyState = digitalRead(ENABLE_BUTTON);

      if ((prevKeyState == HIGH) && (currKeyState == LOW)) {
        // key goes from not pressed to pressed
        KeyPressCount = 0;
      }
      else if ((prevKeyState == LOW) && (currKeyState == HIGH)) {
        if (KeyPressCount < longKeyPressCountMax && KeyPressCount >= mediumKeyPressCountMin) {
          mediumKeyPress();
        }
        else {
          if (KeyPressCount < mediumKeyPressCountMin) {
            shortKeyPress();
          }
        }
      }
      else if (currKeyState == LOW) {
        KeyPressCount++;
        if (KeyPressCount >= longKeyPressCountMax) {
          longKeyPress();
        }
      }
      prevKeyState = currKeyState;
    }
  }
#endif
  
#if defined(ENABLE_BUTTON_GY33)
  void shortKeyPress_gy33() {
    DBG_OUTPUT_PORT.printf("Short GY-33 button press\r\n");
    uint16_t red, green, blue, cl, ct, lux;
    tcs.getRawData(&red, &green, &blue, &cl, &lux, &ct);
    DBG_OUTPUT_PORT.printf("Raw Colors: R: [%d] G: [%d] B: [%d] Clear: [%d] Lux: [%d] Colortemp: [%d]\r\n", (int)red, (int)green, (int)blue, (int)cl, (int)lux, (int)ct);
    uint8_t r, g, b, col, conf;
    tcs.getData(&r, &g, &b, &col, &conf);
    DBG_OUTPUT_PORT.printf("Colors: R: [%d] G: [%d] B: [%d] Color: [%d] Conf: [%d]\r\n", (int)r, (int)g, (int)b, (int)col, (int)conf);
    main_color.red = (pow((r/255.0), GAMMA)*255); main_color.green = (pow((g/255.0), GAMMA)*255); main_color.blue = (pow((b/255.0), GAMMA)*255);main_color.white = 0; 
    ws2812fx_mode = FX_MODE_STATIC;
    prevmode = HOLD;
    mode = SET_ALL;
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress_gy33() {   
      tcs.setConfig(MCU_LED_03, MCU_WHITE_OFF);
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress_gy33() {
      tcs.setConfig(MCU_LED_OFF, MCU_WHITE_OFF);  
  }

  void button_gy33() {
    if (millis() - keyPrevMillis_gy33 >= keySampleIntervalMs_gy33) {
      keyPrevMillis_gy33 = millis();

      byte currKeyState_gy33 = digitalRead(ENABLE_BUTTON_GY33);

      if ((prevKeyState_gy33 == HIGH) && (currKeyState_gy33 == LOW)) {
        // key goes from not pressed to pressed
        KeyPressCount_gy33 = 0;
      }
      else if ((prevKeyState_gy33 == LOW) && (currKeyState_gy33 == HIGH)) {
        if (KeyPressCount_gy33 < longKeyPressCountMax_gy33 && KeyPressCount_gy33 >= mediumKeyPressCountMin_gy33) {
          mediumKeyPress_gy33();
        }
        else {
          if (KeyPressCount_gy33 < mediumKeyPressCountMin_gy33) {
            shortKeyPress_gy33();
          }
        }
      }
      else if (currKeyState_gy33 == LOW) {
        KeyPressCount_gy33++;
        if (KeyPressCount_gy33 >= longKeyPressCountMax_gy33) {
          longKeyPress_gy33();
        }
      }
      prevKeyState_gy33 = currKeyState_gy33;
    }
  }
#endif

#if defined(ENABLE_STATE_SAVE)
  void tickerSaveState(){
    updateState = true;
  }

  void tickerSaveConfig(){
    updateConfig = true;
  }
  
  #if ENABLE_STATE_SAVE == 0 
    // ***************************************************************************
    // EEPROM helper
    // ***************************************************************************
    String readEEPROM(uint16_t offset, uint16_t len) {
      String res = "";
      for (uint16_t i = 0; i < len; ++i)
      {
        res += char(EEPROM.read(i + offset));
        //DBG_OUTPUT_PORT.println(char(EEPROM.read(i + offset)));
      }
      DBG_OUTPUT_PORT.printf("readEEPROM(): %s\r\n", res.c_str());
      return res;
    }
    
    void writeEEPROM(uint16_t offset, uint16_t len, String value) {
      DBG_OUTPUT_PORT.printf("writeEEPROM(): %s\r\n", value.c_str());
      for (uint16_t i = 0; i < len; ++i)
      {
        if (i < value.length()) {
          EEPROM.write(i + offset, value[i]);
        } else {
          EEPROM.write(i + offset, 0);
        }
      }
    } 
  #endif
  #if ENABLE_STATE_SAVE == 1  
    // Write configuration to FS JSON
    bool writeConfigFS(bool saveConfig){
      if (saveConfig) {
        //FS save
        DBG_OUTPUT_PORT.println("Saving config: ");    
        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile) {
          DBG_OUTPUT_PORT.println("Failed!");
          settings_save_conf.detach();
          updateConfig = false;
          return false;
        }
        DBG_OUTPUT_PORT.println(listConfigJSON());
        configFile.print(listConfigJSON());
        configFile.close();
        settings_save_conf.detach();
        updateConfig = false;
        return true;
        //end save
      } else {
        DBG_OUTPUT_PORT.println("SaveConfig is false!");
        return false;
      }
    }
      
    // Read search_str to FS
    bool readConfigFS() {
      //read configuration from FS JSON
      if (SPIFFS.exists("/config.json")) {
        //file exists, reading and loading
        DBG_OUTPUT_PORT.print("Reading config file... ");
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
          DBG_OUTPUT_PORT.println("Opened!");
          size_t size = configFile.size();
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          configFile.close();
       #if defined(ENABLE_MQTT)
          const size_t bufferSize = JSON_OBJECT_SIZE(5) + 500;
       #else
          const size_t bufferSize = JSON_OBJECT_SIZE(1) + 150;
       #endif
          DynamicJsonDocument jsonBuffer(bufferSize);
          DeserializationError error = deserializeJson(jsonBuffer, buf.get());
          DBG_OUTPUT_PORT.print("Config: ");
          if (!error) {
            DBG_OUTPUT_PORT.println("Parsed!");
            JsonObject root = jsonBuffer.as<JsonObject>();
            serializeJson(root, DBG_OUTPUT_PORT);
            DBG_OUTPUT_PORT.println("");
            strcpy(HOSTNAME, root["hostname"]);
          #if defined(ENABLE_MQTT)
            strcpy(mqtt_host, root["mqtt_host"]);
            mqtt_port = root["mqtt_port"].as<uint16_t>();
            strcpy(mqtt_user, root["mqtt_user"]);
            strcpy(mqtt_pass, root["mqtt_pass"]);
          #endif
            WS2812FXStripSettings.stripSize = constrain(root["ws_cnt"].as<uint16_t>(), 1, MAXLEDS);
            char tmp_rgbOrder[5];
            strcpy(tmp_rgbOrder, root["ws_rgbo"]);
            checkRGBOrder(tmp_rgbOrder);
            uint8_t temp_pin;
            checkPin((uint8_t) root["ws_pin"]);
            WS2812FXStripSettings.fxoptions = constrain(root["ws_fxopt"].as<uint8_t>(), 0, 255) & 0xFE;
            jsonBuffer.clear();
            return true;
          } else {
            DBG_OUTPUT_PORT.print("Failed to load json config: ");
            DBG_OUTPUT_PORT.println(error.c_str());
            jsonBuffer.clear();
          }
        } else {
          DBG_OUTPUT_PORT.println("Failed to open /config.json");
        }
      } else {
        DBG_OUTPUT_PORT.println("Coudnt find config.json");
        writeConfigFS(true);
      }
      //end read
      return false;
    }
  
    bool writeStateFS(bool saveConfig){
      if (saveConfig) {
        //save the strip state to FS JSON
        DBG_OUTPUT_PORT.print("Saving state: ");
        //SPIFFS.remove("/stripstate.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
        File configFile = SPIFFS.open("/stripstate.json", "w");
        if (!configFile) {
          DBG_OUTPUT_PORT.println("Failed!");
          settings_save_state.detach();
          updateState = false;
          return false;
        }
        DBG_OUTPUT_PORT.println(listStatusJSON());
        configFile.print(listStatusJSON());
        configFile.close();
        settings_save_state.detach();
        updateState = false;
        return true;
        //end save
      } else {
        DBG_OUTPUT_PORT.println("SaveStateConfig is false!");
        return false;
      }
    }
    
    bool readStateFS() {
      //read strip state from FS JSON
      //if (resetsettings) { SPIFFS.begin(); SPIFFS.remove("/config.json"); SPIFFS.format(); delay(1000);}
      if (SPIFFS.exists("/stripstate.json")) {
        //file exists, reading and loading
        DBG_OUTPUT_PORT.print("Reading state file... ");
        File configFile = SPIFFS.open("/stripstate.json", "r");
        if (configFile) {
          DBG_OUTPUT_PORT.println("Opened!");
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          configFile.close();
          const size_t bufferSize = JSON_OBJECT_SIZE(5) + JSON_ARRAY_SIZE(12) + 500;
          DynamicJsonDocument jsonBuffer(bufferSize);
          DeserializationError error = deserializeJson(jsonBuffer, buf.get());
          DBG_OUTPUT_PORT.print("Config: ");
          if (!error) {
            DBG_OUTPUT_PORT.print("Parsed");
            JsonObject root = jsonBuffer.as<JsonObject>();
            serializeJson(root, DBG_OUTPUT_PORT);
            DBG_OUTPUT_PORT.println("");
            mode = static_cast<MODE>(root["mode"].as<uint8_t>());
            ws2812fx_mode = root["ws2812fx_mode"].as<uint8_t>();
            ws2812fx_speed = root["speed"].as<uint8_t>();
            brightness =  root["brightness"];
            main_color.white = root["color"][0].as<uint8_t>();
            main_color.red =  root["color"][1].as<uint8_t>();
            main_color.green = root["color"][2].as<uint8_t>();
            main_color.blue =  root["color"][3].as<uint8_t>();
            back_color.white = root["color"][4].as<uint8_t>();
            back_color.red =  root["color"][5].as<uint8_t>();
            back_color.green =  root["color"][6].as<uint8_t>();
            back_color.blue =  root["color"][7].as<uint8_t>();
            xtra_color.white = root["color"][8].as<uint8_t>();
            xtra_color.red = root["color"][9].as<uint8_t>();
            xtra_color.green =  root["color"][10].as<uint8_t>();
            xtra_color.blue = root["color"][11].as<uint8_t>();
            convertColors();
            jsonBuffer.clear();
            return true;
          } else {
            DBG_OUTPUT_PORT.print("Failed to load json config: ");
            DBG_OUTPUT_PORT.println(error.c_str());
            jsonBuffer.clear();
          }
        } else {
          DBG_OUTPUT_PORT.println("Failed to open \"/stripstate.json\"");
        }
      } else {
        DBG_OUTPUT_PORT.println("Couldn't find \"/stripstate.json\"");
        writeStateFS(true);
      }
      //end read
      return false;
    }
  #endif
#endif

#if defined(ENABLE_REMOTE)
// ***************************************************************************
// Request handler for IR remote support
// ***************************************************************************
void handleRemote() {
    uint8_t chng = 1;
    if (irrecv.decode(&results)) {
      DBG_OUTPUT_PORT.print("IR Code: 0x");
      DBG_OUTPUT_PORT.print(uint64ToString(results.value, HEX));
      DBG_OUTPUT_PORT.println("");
      if (results.value == rmt_commands[REPEATCMD]) { //Repeat
        results.value = last_remote_cmd;
        chng = 5;
      }
      if (results.value == rmt_commands[ON_OFF]) {   // ON/OFF TOGGLE
        last_remote_cmd = 0;
        if (mode == OFF) {
          mode = SET_ALL;
        } else {
          mode = OFF;
        }
      }
      if ((mode != AUTO) && (mode != OFF)) {
        if (results.value == rmt_commands[BRIGHTNESS_UP]) { //Brightness Up
          last_remote_cmd = results.value;
          if (brightness + chng <= 255) {
            brightness = brightness + chng;
            mode = SET_BRIGHTNESS;
          }
        }
        if (results.value == rmt_commands[BRIGHTNESS_DOWN]) { //Brightness down
          last_remote_cmd = results.value;
          if (brightness - chng >= 0) {
            brightness = brightness - chng;
            mode = SET_BRIGHTNESS;
          }
        }
      }
      if ((mode !=AUTO) && (mode != E131) && (mode != OFF)) {
        if (results.value == rmt_commands[SPEED_UP]) { //Speed Up
          last_remote_cmd = results.value;
          if (ws2812fx_speed + chng <= 255) {
            ws2812fx_speed = ws2812fx_speed + chng;
            mode = SET_SPEED;
          }
        }
        if (results.value == rmt_commands[SPEED_DOWN]) { //Speed down
          last_remote_cmd = results.value;
          if (ws2812fx_speed - chng >= 0) {
            ws2812fx_speed = ws2812fx_speed - chng;
            mode = SET_SPEED;
          }
        }
      }
      if (mode == HOLD) {
        if (results.value == rmt_commands[RED_UP]) { //Red Up
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.red + chng <= 255) {
              main_color.red = main_color.red + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 2) {
            if (back_color.red + chng <= 255) {
              back_color.red = back_color.red + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.red + chng <= 255) {
              xtra_color.red = xtra_color.red + chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[RED_DOWN]) { //Red down
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.red - chng >= 0) {
              main_color.red = main_color.red - chng;
              mode = SET_COLOR; 
            }
          }
          if (selected_color == 2) {
            if (back_color.red - chng >= 0) {
              back_color.red = back_color.red - chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.red - chng >= 0) {
              xtra_color.red = xtra_color.red - chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[GREEN_UP]) { //Green Up
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.green + chng <= 255) {
              main_color.green = main_color.green + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 2) {
            if (back_color.green + chng <= 255) {
              back_color.green = back_color.green + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.green + chng <= 255) {
              xtra_color.green = xtra_color.green + chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[GREEN_DOWN]) { //green down
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.green - chng >= 0) {
              main_color.green = main_color.green - chng;;
              mode = SET_COLOR; 
            }
          }
          if (selected_color == 2) {
            if (back_color.green - chng >= 0) {
              back_color.green = back_color.green - chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.green - chng >= 0) {
              xtra_color.green = xtra_color.green - chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[BLUE_UP]) { //Blue Up
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.blue + chng <= 255) {
              main_color.blue = main_color.blue + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 2) {
            if (back_color.blue + chng <= 255) {
              back_color.blue = back_color.blue + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.blue + chng <= 255) {
              xtra_color.blue = xtra_color.blue + chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[BLUE_DOWN]) { //BLUE down
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.blue - chng >= 0) {
              main_color.blue = main_color.blue - chng;
              mode = SET_COLOR; 
            }
          }
          if (selected_color == 2) {
            if (back_color.blue - chng >= 0) {
              back_color.blue = back_color.blue - chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.blue - chng >= 0) {
              xtra_color.blue = xtra_color.blue - chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[WHITE_UP]) { //White Up
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.white + chng <= 255) {
              main_color.white = main_color.white + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 2) {
            if (back_color.white + chng <= 255) {
              back_color.white = back_color.white + chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.white + chng <= 255) {
              xtra_color.white = xtra_color.white + chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[WHITE_DOWN]) { //White down
          last_remote_cmd = results.value;
          if (selected_color == 1) {
            if (main_color.white - chng >= 0) {
              main_color.white = main_color.white - chng;
              mode = SET_COLOR; 
            }
          }
          if (selected_color == 2) {
            if (back_color.white - chng >= 0) {
              back_color.white = back_color.white - chng;
              mode = SET_COLOR;
            }
          }
          if (selected_color == 3) {
            if (xtra_color.white - chng >= 0) {
              xtra_color.white = xtra_color.white - chng;
              mode = SET_COLOR;
            }
          }
        }
        if (results.value == rmt_commands[COL_M]) { // Select Main Color
          last_remote_cmd = 0;
          selected_color = 1;
        } 
        if (results.value == rmt_commands[COL_B]) { // Select Back Color
          last_remote_cmd = 0;
          selected_color = 2;
        } 
        if (results.value == rmt_commands[COL_X]) { // Select Extra Color
          last_remote_cmd = 0;
          selected_color = 3;
        }
      } // end of if HOLD
      if (results.value == rmt_commands[MODE_UP]) { //Mode Up
        last_remote_cmd = results.value;
        if ((ws2812fx_mode < strip->getModeCount()-1) && (mode == HOLD)) {
          ws2812fx_mode = ws2812fx_mode + 1;
        }
        mode = SET_MODE;
      }
      if (results.value == rmt_commands[MODE_DOWN]) { //Mode down
        last_remote_cmd = results.value;
        if ((ws2812fx_mode > 0) && (mode == HOLD)) {
          ws2812fx_mode = ws2812fx_mode - 1;
        }
        mode = SET_MODE;
      }
      if (results.value == rmt_commands[AUTOMODE]) { // Toggle Automode
        last_remote_cmd = 0;
        if (mode != AUTO) {
          mode = AUTO;
        } else {
          mode = SET_ALL;
        }
      }
    #if defined(ENABLE_TV)
      if (results.value == rmt_commands[CUST_1]) { // Select TV Mode
        last_remote_cmd = 0;
        if (mode == TV) {
          mode = SET_ALL;
        } else {
          mode = TV;
        }  
      }
    #endif 
      if (results.value == rmt_commands[CUST_2]) { // Select Custom Mode 2
        last_remote_cmd = 0;
        ws2812fx_mode = 12;
        mode = SET_MODE;
      } 
      if (results.value == rmt_commands[CUST_3]) { // Select Custom Mode 3
        last_remote_cmd = 0;
        ws2812fx_mode = 48;
        mode = SET_MODE;
      } 
      if (results.value == rmt_commands[CUST_4]) { // Select Custom Mode 4
        last_remote_cmd = 0;
        ws2812fx_mode = 21;
        mode = SET_MODE; 
      }
      if (results.value == rmt_commands[CUST_5]) { // Select Custom Mode 5
        last_remote_cmd = 0;
        ws2812fx_mode = 46;
        mode = SET_MODE;
      } 
      irrecv.resume();  // Receive the next value
    }
  }
#endif
