// ***************************************************************************
// Request handlers
// ***************************************************************************
// ***************************************************************************
// Handler functions for WS and MQTT
// ***************************************************************************
bool handleSetMainColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[1], NULL, 16);
  if (rgb != segState.colors[State.segment][0]) {
    main_color.white = ((rgb >> 24) & 0xFF);
    main_color.red = ((rgb >> 16) & 0xFF);
    main_color.green = ((rgb >> 8) & 0xFF);
    main_color.blue = ((rgb >> 0) & 0xFF);
    return true;
  }
  return false;
}

bool handleSetBackColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[2], NULL, 16);
  if (rgb != segState.colors[State.segment][1]) {
    back_color.white = ((rgb >> 24) & 0xFF);
    back_color.red = ((rgb >> 16) & 0xFF);
    back_color.green = ((rgb >> 8) & 0xFF);
    back_color.blue = ((rgb >> 0) & 0xFF);
    return true;
  }
  return false;
}
bool handleSetXtraColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[3], NULL, 16);
  if (rgb != segState.colors[State.segment][2]) {
    xtra_color.white = ((rgb >> 24) & 0xFF);
    xtra_color.red = ((rgb >> 16) & 0xFF);
    xtra_color.green = ((rgb >> 8) & 0xFF);
    xtra_color.blue = ((rgb >> 0) & 0xFF);
    return true;
  }
  return false;
}

bool handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtoul((const char *) &mypayload[1], NULL, 16);
  if ((State.mode = OFF) || (segState.mode[State.segment] != strip->getMode(State.segment)) || (rgb != segState.colors[State.segment][0])) {
    main_color.white = ((rgb >> 24) & 0xFF);
    main_color.red = ((rgb >> 16) & 0xFF);
    main_color.green = ((rgb >> 8) & 0xFF);
    main_color.blue = ((rgb >> 0) & 0xFF);
    DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: R: [%u] G: [%u] B: [%u] W: [%u]\r\n", main_color.red, main_color.green, main_color.blue, main_color.white);
    fx_mode = FX_MODE_STATIC;
    State.mode = SET;
    return true;
  }
  return false;
}

void handleSetSingleLED(uint8_t * mypayload, uint8_t firstChar = 0) {
  // decode led index
  char templed[5];
  strncpy (templed, (const char *) &mypayload[firstChar], 4 );
  templed[4] = 0x00;
  uint8_t led = atoi(templed);

  DBG_OUTPUT_PORT.printf("led value: [%i]. Entry threshold: <= [%i] (=> %s)\r\n", led, Config.stripSize, mypayload );
  if (led <= Config.stripSize) {
    char redhex[3];
    char greenhex[3];
    char bluehex[3];
    char whitehex[3];
    strncpy (whitehex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (redhex, (const char *) &mypayload[6 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[8 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[10 + firstChar], 2 );
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
  State.mode = HOLD;
  fx_mode= FX_MODE_CUSTOM_1;
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
  // While there is a range to process R00010010<0000ff00>

  while (nextCommand) {
    // Loop for each LED.
    char startled[5];
    char endled[5];
    char colorval[9];
    strncpy ( startled, (const char *) &nextCommand[0], 4 );
    startled[4] = 0x00;
    strncpy ( endled, (const char *) &nextCommand[4], 4 );
    endled[4] = 0x00;
    strncpy ( colorval, (const char *) &nextCommand[8], 8 );
    colorval[8] = 0x00;
    uint8_t rangebegin = atoi(startled);
    uint8_t rangeend = atoi(endled);
    DBG_OUTPUT_PORT.printf("Setting RANGE from [%i] to [%i] as color [%s]\r\n", rangebegin, rangeend, colorval);

    while ( rangebegin <= rangeend ) {
      char rangeData[18];
      snprintf(rangeData, sizeof(rangeData), "%04d%s", rangebegin, colorval);
      rangeData[sizeof(rangeData) - 1] = 0x00;
      // Set one LED
      handleSetSingleLED((uint8_t*) rangeData, 0);
      rangebegin++;
    }

    // Next Range at R
    nextCommand = (uint8_t*) strtok(NULL, "R");
  }
}

bool setModeByStateString(String saved_state_string) {
  if (getValue(saved_state_string, '|', 0) == "STA") {
    DBG_OUTPUT_PORT.printf("Parsed state: %s\r\n", saved_state_string.c_str());
    String str_mode = getValue(saved_state_string, '|', 1);
    State.mode = static_cast<MODE>(str_mode.toInt());
    String str_fx_mode = getValue(saved_state_string, '|', 2);
    fx_mode = str_fx_mode.toInt();
    String str_fx_speed = getValue(saved_state_string, '|', 3);
    segState.speed[State.segment] = str_fx_speed.toInt();
    String str_brightness = getValue(saved_state_string, '|', 4);
    State.brightness = str_brightness.toInt();
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
    DBG_OUTPUT_PORT.print("Set to state: ");
    DBG_OUTPUT_PORT.println(listStateJSON());
    //prevmode=mode;
    //State.mode = SET;
    return true;
  } else {
    DBG_OUTPUT_PORT.println("Saved state not found!");
    return false;
  }
  return false;
}

void handleSetWS2812FXMode(uint8_t * mypayload) {
  if (isDigit(mypayload[1])) {
    fx_mode = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
    fx_mode = constrain(fx_mode, 0, strip->getModeCount() - 1);
    State.mode = SET;
  } else  {
    if (strcmp((char *) &mypayload[1], "off") == 0) {
      if (State.mode == OFF) { State.mode = SET; } else { State.mode = OFF; };
    }
    if (strcmp((char *) &mypayload[1], "on") == 0) {
      State.mode = SET;
    }
  }
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
// Functions and variables
// ***************************************************************************
void Dbg_Prefix(bool _mqtt, uint8_t _num) {
  if (_mqtt == true)  {
    DBG_OUTPUT_PORT.print("MQTT: ");
  } else {
    DBG_OUTPUT_PORT.print("WS: ");
    webSocket.sendTXT(_num, "OK");
  }
}

void checkpayload(uint8_t * _payload, bool mqtt = false, uint8_t num = 0) {
  // Select segment
  boolean _updateState = false;
  boolean _updateSegState = false;
  // / ==> Set active segment
  if (_payload[0] == 'S') {
    if (_payload[1] == 's') {
      uint8_t _seg = (uint8_t) strtol((const char *) &_payload[2], NULL, 10);
      _seg = constrain(_seg, 0, Config.segments - 1);
      if (prevsegment != _seg) {
        prevsegment = State.segment;
        State.segment = _seg;
        getSegmentParams(State.segment);
        //memcpy(hexcolors_trans, segState.colors[State.segment], sizeof(hexcolors_trans));
        _updateState = true;
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set segment to: [%u]\r\n", State.segment);
      }
    }
    // / ==> Set segment first LED
    if (_payload[1] == '[') {
      uint16_t _seg_start = (uint16_t) strtol((const char *) &_payload[2], NULL, 10);
      _seg_start = constrain(_seg_start, 0, Config.stripSize - 1);
      if (_seg_start != segState.start) {
        segState.start = _seg_start;
        _updateSegState = true;
        setSegmentSize();
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set segment start to: [%u]\r\n", _seg_start);
      }
    }
    // / ==> Set segment last LED
    if (_payload[1] == ']') {
      uint16_t _seg_stop = (uint16_t) strtol((const char *) &_payload[2], NULL, 10);
      _seg_stop = constrain(_seg_stop, segState.start, Config.stripSize - 1);
      if (_seg_stop != segState.stop) {
        segState.stop = _seg_stop;
        _updateSegState = true;
        setSegmentSize();
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set segment stop to: [%u]\r\n", _seg_stop);
      }
    }
    if (_payload[1] == 'o') {
      char _fx_options[4];
      snprintf(_fx_options, sizeof(_fx_options), "%s", &_payload[2]);
      _fx_options[3] = 0x00;
      if (((constrain(atoi(_fx_options), 0, 255)>>1)<<1) != segState.options) {
        segState.options= ((constrain(atoi(_fx_options), 0, 255)>>1)<<1);
        _updateSegState = true;
        strip->setOptions(State.segment, segState.options);
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set segment options to: [%u]\r\n", segState.options);
      }
    }
    char * buffer = listSegmentStateJSON(State.segment);
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
  }
  // / ==> Set WS2812 mode.
  if (_payload[0] == '/') {
    handleSetWS2812FXMode(_payload);
    if (fx_mode !=  strip->getMode(State.segment)) {
      _updateSegState = true;
      Dbg_Prefix(mqtt, num);
      DBG_OUTPUT_PORT.printf("Set WS2812 mode: [%s]\r\n", _payload);
    }
  }
  // # ==> Set main color - ## ==> Set 2nd color - ### ==> Set 3rd color
  if (_payload[0] == '#') {
    if (_payload[2] == '#') {
      _updateSegState = handleSetXtraColor(_payload);
      if (_updateSegState) {
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set 3rd color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n",  xtra_color.red, xtra_color.green, xtra_color.blue, xtra_color.white);
      }
    } else if (_payload[1] == '#') {
      _updateSegState = handleSetBackColor(_payload);
      if (_updateSegState) {
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set 2nd color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n",  back_color.red, back_color.green, back_color.blue, back_color.white);
      }
    } else {
      _updateSegState = handleSetMainColor(_payload);
      if (_updateSegState) {
        Dbg_Prefix(mqtt, num);
        DBG_OUTPUT_PORT.printf("Set main color to: R: [%u] G: [%u] B: [%u] W: [%u]\r\n", main_color.red, main_color.green, main_color.blue, main_color.white);
      }
    }
    if (_updateSegState) {
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", _payload);
      #endif
      State.mode = SET;
    }
  }

  // ? ==> Set speed
  if (_payload[0] == '?') {
    uint16_t _fx_speed = (uint16_t) strtol((const char *) &_payload[1], NULL, 10);
    segState.speed[State.segment] = constrain(_fx_speed, SPEED_MIN, SPEED_MAX );
    State.mode = SET;
    _updateSegState = true;
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set speed to: [%u]\r\n", segState.speed[State.segment]);
  }

  // % ==> Set brightness
  if (_payload[0] == '%') {
    uint8_t b = (uint8_t) strtol((const char *) &_payload[1], NULL, 10);
    State.brightness = constrain(b, 0, 255);
    if (strip->getBrightness() != State.brightness) {
      State.mode = SET;
      Dbg_Prefix(mqtt, num);
      DBG_OUTPUT_PORT.printf("Set brightness to: [%u]\r\n", State.brightness);
      _updateState = true;
    }
  }

  // * ==> Set main color and light all LEDs (Shortcut)
  if (_payload[0] == '*') {
    _updateSegState = handleSetAllMode(_payload);
    if (_updateSegState) {
      Dbg_Prefix(mqtt, num);
      DBG_OUTPUT_PORT.printf("Set main color and light all LEDs [%s]\r\n", _payload);
    }
  }

  // ! ==> Set single LED in given color
  if (_payload[0] == '!') {
    handleSetSingleLED(_payload, 1);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", _payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set single LED in given color [%s]\r\n", _payload);
  }

  // + ==> Set multiple LED in the given colors
  if (_payload[0] == '+') {
    handleSetDifferentColors(_payload);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", _payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set multiple LEDs in given color [%s]\r\n", _payload);
  }

  // + ==> Set range of LEDs in the given color
  if (_payload[0] == 'R') {
    handleRangeDifferentColors(_payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set range of LEDs in given color [%s]\r\n", _payload);
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %s", _payload);
    #endif
  }
#if defined(ENABLE_STATE_SAVE)
  if (_updateState) {
    if(save_state.active()) save_state.detach();
    save_state.once(3, tickerSaveState);
  }
  if (_updateSegState) {
    State.mode = SET;
    if(save_seg_state.active()) save_seg_state.detach();
    save_seg_state.once(3, tickerSaveSegmentState);

  }
  _updateState = false;
  _updateSegState = false;
#endif

  // $ ==> Get config Info.
  if (_payload[0] == 'C') {
    bool _updateStrip = false;
    bool _updateConfig  = false;
    bool _updateState = false;
    if (_payload[1] == 's') {
      if (_payload[2] == 's') {
        char _num_segments[3];
        snprintf(_num_segments, sizeof(_num_segments), "%s", &_payload[3]);
        _num_segments[2] = 0x00;
        Config.segments = constrain(atoi(_num_segments), 1, MAX_NUM_SEGMENTS - 1);
        if (State.segment >=  Config.segments) {
          State.segment = Config.segments - 1;
          _updateState = true;
        }
        _updateStrip = true;
      }
      if (_payload[2] == 'c') {
        char tmp_count[6];
        snprintf(tmp_count, sizeof(tmp_count), "%s", &_payload[3]);
        tmp_count[5] = 0x00;
        Config.stripSize = constrain(atoi(tmp_count), 1, MAXLEDS);
        _updateStrip = true;
      }
      if (_payload[2] == 'r') {
        char _rgbOrder[5];
        snprintf(_rgbOrder, sizeof(_rgbOrder), "%s", &_payload[3]);
        _rgbOrder[4] = 0x00;
        checkRGBOrder(_rgbOrder);
        _updateStrip=true;
      }
    #if !defined(USE_WS2812FX_DMA)
      if (_payload[2] == 'p') {
        char tmp_pin[3];
        snprintf(tmp_pin, sizeof(tmp_pin), "%s", &_payload[3]);
        tmp_pin[2] = 0x00;
        checkPin(atoi(tmp_pin));
        _updateStrip = true;
      }
    #endif
    }
    if (_updateStrip){
      initStrip();
    }
    if (_payload[1] == 'h') {
      snprintf(HOSTNAME, sizeof(HOSTNAME), "%s", &_payload[2]);
      HOSTNAME[sizeof(HOSTNAME) - 1] = 0x00;
      _updateConfig = true;
    }
  #if defined(ENABLE_MQTT)
    if (_payload[1] == 'm') {
      if (_payload[2] == 'h') {
        snprintf(mqtt_host, sizeof(mqtt_host), "%s", &_payload[3]);
        mqtt_host[sizeof(mqtt_host) - 1] = 0x00;
        _updateConfig = true;
      }
      if (_payload[2] == 'p') {
        char tmp_port[6];
        snprintf(tmp_port, sizeof(tmp_port), "%s", &_payload[3]);
        tmp_port[sizeof(tmp_port) - 1] = 0x00;
        mqtt_port = constrain(atoi(tmp_port), 0, 65535);
        _updateConfig = true;
      }
      if (_payload[2] == 'u') {
        snprintf(mqtt_user, sizeof(mqtt_user), "%s", &_payload[3]);
        mqtt_user[sizeof(mqtt_user) - 1] = 0x00;
        _updateConfig = true;
      }
      if (_payload[2] == 'w') {
        snprintf(mqtt_pass, sizeof(mqtt_pass), "%s", &_payload[3]);
        mqtt_pass[sizeof(mqtt_pass) - 1] = 0x00;
        _updateConfig = true;
      }
    }
    if (_updateConfig) {
      initMqtt();
    }
  #endif
    if (_payload[1] == 'e') {
      char _transEffect[2];
      snprintf(_transEffect, sizeof(_transEffect), "%s", &_payload[2]);
      _transEffect[sizeof(_transEffect) - 1] = 0x00;
      Config.transEffect = atoi(_transEffect);
      _updateConfig = true;
    }

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
    if (_updateStrip || _updateConfig) {
      if(save_conf.active()) save_conf.detach();
      save_conf.once(3, tickerSaveConfig);
    }
    if (_updateState) {
      if(save_state.active()) save_state.detach();
      save_state.once(3, tickerSaveState);
    }
#endif
    _updateStrip = false;
    _updateConfig  = false;
    _updateState = false;
    DBG_OUTPUT_PORT.printf("Get status info: %s\r\n", buffer);
    free (buffer);
  }

  // $ ==> Get status Info.
  if (_payload[0] == '$') {
    char * buffer = listStateJSONfull();
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

  // ~ ==> Get WS2812 modes.
  if (_payload[0] == '~') {
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
       root["state"] = (State.mode != OFF) ? on_cmd : off_cmd;
       #if defined(ENABLE_MQTT_INCLUDE_IP)
         root["ip"] = WiFi.localIP().toString();
       #endif
       root["segment"] = State.segment;
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
       if (strstr(Config.RGBOrder, "W") != NULL) {
         root["white_value"]= main_color.white;
       }
       root["brightness"] = State.brightness;
       root["color_temp"] = color_temp;
       root["speed"] = segState.speed[State.segment];
       //char modeName[30];
       //strncpy_P(modeName, (PGM_P)strip->getModeName(strip->getMode()), sizeof(modeName)); // copy from progmem
       #if defined(ENABLE_HOMEASSISTANT)
       if (State.mode == OFF){
         root["effect"] = "OFF";
       } else {
         root["effect"] = strip->getModeName(strip->getMode());
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
      bool _updateState    = false;
      bool _updateSegState = false;
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
          State.mode = SET;
          _updateState = true;
        }
        else if (strcmp(state_in, off_cmd) == 0) {
          State.mode = OFF;
          _updateState = true;
          jsonBuffer.clear();
          return true;
        }
      }
      if (root.containsKey("segment")) {
        uint8_t json_segment = constrain((uint8_t) root["segment"], 0, Config.segments - 1);
        if (prevsegment != json_segment) {
          prevsegment = State.segment;
          State.segment = json_segment;
          getSegmentParams(State.segment);
          State.mode = SET;
          _updateState = true;
        }
      }
      if (root.containsKey("color")) {
        JsonObject color = root["color"];
        if (color.containsKey("r")) { main_color.red = (uint8_t) color["r"]; }
        if (color.containsKey("g")) { main_color.green = (uint8_t) color["g"]; }
        if (color.containsKey("b")) { main_color.blue = (uint8_t) color["b"]; }
        if (color.containsKey("w")) { main_color.white = (uint8_t) color["w"]; }
        if (color.containsKey("r2")) { back_color.red = (uint8_t) color["r2"]; }
        if (color.containsKey("g2")) { back_color.green = (uint8_t) color["g2"]; }
        if (color.containsKey("b2")) { back_color.blue = (uint8_t) color["b2"]; }
        if (color.containsKey("w2")) { back_color.white = (uint8_t) color["w2"]; }
        if (color.containsKey("r3")) { xtra_color.red = (uint8_t) color["r3"]; }
        if (color.containsKey("g3")) { xtra_color.green = (uint8_t) color["g3"]; }
        if (color.containsKey("b3")) { xtra_color.blue = (uint8_t) color["b3"]; }
        if (color.containsKey("w3")) { xtra_color.white = (uint8_t) color["w3"]; }
        _updateSegState = true;
      }

      if (root.containsKey("white_value")) {
        uint8_t json_white_value = constrain((uint8_t) root["white_value"], 0, 255);
        if (json_white_value != main_color.white) {
          main_color.white = json_white_value;
          _updateSegState = true;
        }
      }

      if (root.containsKey("speed")) {
        uint8_t _fx_speed = constrain((uint8_t) root["speed"], 0, 255);
        if (_fx_speed != segState.speed[State.segment]) {
          segState.speed[State.segment] = _fx_speed;
          _updateSegState = true;
        }
      }

      if (root.containsKey("color_temp")) {
        //temp comes in as mireds, need to convert to kelvin then to RGB
        color_temp = (uint16_t) root["color_temp"];
        uint16_t kelvin  = 1000000 / color_temp;
        main_color = temp2rgb(kelvin);
        _updateSegState = true;
      }

      if (root.containsKey("brightness")) {
        uint8_t json_brightness = constrain((uint8_t) root["brightness"], 0, 255); //fix #224
        if (json_brightness != State.brightness) {
          State.brightness = json_brightness;
          State.mode = SET;
          _updateState = true;
        }
      }

      if (root.containsKey("effect")) {
        String effectString = root["effect"].as<String>();
        #if defined(ENABLE_HOMEASSISTANT)
          if(effectString == "OFF"){
            State.mode = OFF;
            _updateState = true;
          }
        #endif
        for (uint8_t i = 0; i < strip->getModeCount(); i++) {
          if(String(strip->getModeName(i)) == effectString) {
            State.mode = SET;
            fx_mode = i;
            _updateState = true;
            _updateSegState = true;
            break;
          }
        }
      }
      #if defined(ENABLE_STATE_SAVE)
        if (_updateState) {
          if(save_state.active()) save_state.detach();
          save_state.once(3, tickerSaveState);
        }
        if (_updateSegState) {
          State.mode = SET;
          if(save_seg_state.active()) save_seg_state.detach();
          save_seg_state.once(3, tickerSaveSegmentState);
        }
      #endif
        _updateState = false;
        _updateSegState = false;
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
        if(ha_send_data.active()) ha_send_data.detach();
        ha_send_data.once(5, tickerSendState);
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
            if (strstr(Config.RGBOrder, "W") != NULL) {
              root["white_value"]= "true";
            }
            root["optimistic"] = "false";
            root["color_temp"] = "true";
            root["effect"] = "true";
            JsonArray effect_list = root.createNestedArray("effect_list");
            effect_list.add("OFF");
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
          if (strstr(Config.RGBOrder, "W") != NULL) {
            root["white_value"]= "true";
          }
          root["optimistic"] = "false";
          root["color_temp"] = "true";
          root["effect"] = "true";
          JsonArray effect_list = root.createNestedArray("effect_list");
          effect_list.add("OFF");
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
    if (State.mode == OFF) {
      setModeByStateString(BTN_MODE_SHORT);
      prevmode = State.mode;
      State.mode = SET;
    } else {
      State.mode = OFF;
    }
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    DBG_OUTPUT_PORT.printf("Medium button press\r\n");
    setModeByStateString(BTN_MODE_MEDIUM);
    prevmode = State.mode;
    State.mode = SET;
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DBG_OUTPUT_PORT.printf("Long button press\r\n");
    setModeByStateString(BTN_MODE_LONG);
    prevmode = State.mode;
    State.mode = SET;
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
    State.mode = SET;
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
    if (millis() - keyPrevMillis_gy33 >= keySampleIntervalMs) {
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

#if defined(ENABLE_REMOTE)
// ***************************************************************************
// Request handler for IR remote support
// ***************************************************************************
void handleRemote() {
    uint8_t chng = 1;
    bool _updateState = false;
    bool _updateSegState = false;
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
        if (State.mode == OFF) {
          State.mode = SET;
          _updateState = true;
        } else {
          State.mode = OFF;
          _updateState = true;;
        }
      }
      if (State.mode == HOLD) {
        if (results.value == rmt_commands[BRIGHTNESS_UP]) { //Brightness Up
          last_remote_cmd = results.value;
          if (State.brightness + chng <= 255) {
            State.brightness = State.brightness + chng;
            brightness_trans = State.brightness;
            _updateState = true;
          }
        }
        if (results.value == rmt_commands[BRIGHTNESS_DOWN]) { //Brightness down
          last_remote_cmd = results.value;
          if (State.brightness - chng >= 0) {
            State.brightness = State.brightness - chng;
            brightness_trans = State.brightness;
            _updateState = true;
          }
        }
        if ((segState.mode[State.segment] < FX_MODE_CUSTOM_0) || (segState.mode[State.segment] > FX_MODE_CUSTOM_1)) {
          if (results.value == rmt_commands[SPEED_UP]) { //Speed Up
            last_remote_cmd = results.value;
            if (segState.speed[State.segment] + chng <= 65535) {
              segState.speed[State.segment] = segState.speed[State.segment] + chng;
              _updateSegState = true;
            }
          }
          if (results.value == rmt_commands[SPEED_DOWN]) { //Speed down
            last_remote_cmd = results.value;
            if (segState.speed[State.segment] - chng >= 0) {
              segState.speed[State.segment] = segState.speed[State.segment] - chng;
              _updateSegState = true;
            }
          }
        }
        if ((segState.mode[State.segment] < FX_MODE_CUSTOM_0) || (segState.mode[State.segment] > FX_MODE_CUSTOM_4)) {
          if (results.value == rmt_commands[RED_UP]) { //Red Up
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.red + chng <= 255) {
                main_color.red = main_color.red + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.red + chng <= 255) {
                back_color.red = back_color.red + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.red + chng <= 255) {
                xtra_color.red = xtra_color.red + chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[RED_DOWN]) { //Red down
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.red - chng >= 0) {
                main_color.red = main_color.red - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.red - chng >= 0) {
                back_color.red = back_color.red - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.red - chng >= 0) {
                xtra_color.red = xtra_color.red - chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[GREEN_UP]) { //Green Up
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.green + chng <= 255) {
                main_color.green = main_color.green + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.green + chng <= 255) {
                back_color.green = back_color.green + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.green + chng <= 255) {
                xtra_color.green = xtra_color.green + chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[GREEN_DOWN]) { //Green down
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.green - chng >= 0) {
                main_color.green = main_color.green - chng;;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.green - chng >= 0) {
                back_color.green = back_color.green - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.green - chng >= 0) {
                xtra_color.green = xtra_color.green - chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[BLUE_UP]) { //Blue Up
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.blue + chng <= 255) {
                main_color.blue = main_color.blue + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.blue + chng <= 255) {
                back_color.blue = back_color.blue + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.blue + chng <= 255) {
                xtra_color.blue = xtra_color.blue + chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[BLUE_DOWN]) { //Blue down
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.blue - chng >= 0) {
                main_color.blue = main_color.blue - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.blue - chng >= 0) {
                back_color.blue = back_color.blue - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.blue - chng >= 0) {
                xtra_color.blue = xtra_color.blue - chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[WHITE_UP]) { //White Up
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.white + chng <= 255) {
                main_color.white = main_color.white + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.white + chng <= 255) {
                back_color.white = back_color.white + chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.white + chng <= 255) {
                xtra_color.white = xtra_color.white + chng;
                _updateSegState = true;
              }
            }
          }
          if (results.value == rmt_commands[WHITE_DOWN]) { //White down
            last_remote_cmd = results.value;
            if (selected_color == 1) {
              if (main_color.white - chng >= 0) {
                main_color.white = main_color.white - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 2) {
              if (back_color.white - chng >= 0) {
                back_color.white = back_color.white - chng;
                _updateSegState = true;
              }
            }
            if (selected_color == 3) {
              if (xtra_color.white - chng >= 0) {
                xtra_color.white = xtra_color.white - chng;
                _updateSegState = true;
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
        }
      } // end of if HOLD
      if (results.value == rmt_commands[MODE_UP]) { //Mode Up
        last_remote_cmd = results.value;
        if ((segState.mode[State.segment] < strip->getModeCount()-1) && (State.mode == HOLD)) {
          fx_mode = segState.mode[State.segment] + 1;
        }
        _updateSegState = true;
      }
      if (results.value == rmt_commands[MODE_DOWN]) { //Mode down
        last_remote_cmd = results.value;
        if ((segState.mode[State.segment] > 0) && (State.mode == HOLD)) {
          fx_mode = segState.mode[State.segment] - 1;
        }
        _updateSegState = true;
      }
      if (results.value == rmt_commands[AUTOMODE]) { // Toggle Automode
        last_remote_cmd = 0;
        fx_mode = FX_MODE_CUSTOM_0;
        _updateSegState = true;
      }
    #if defined(CUSTOM_WS2812FX_ANIMATIONS)
      if (results.value == rmt_commands[CUST_1]) { // Select TV Mode
        last_remote_cmd = 0;
        fx_mode = FX_MODE_CUSTOM_2;
        _updateSegState = true;
      }
    #endif
      if (results.value == rmt_commands[CUST_2]) { // Select Custom Mode 2
        last_remote_cmd = 0;
        fx_mode = FX_MODE_RAINBOW_CYCLE;
        _updateSegState = true;
      }
      if (results.value == rmt_commands[CUST_3]) { // Select Custom Mode 3
        last_remote_cmd = 0;
        fx_mode = FX_MODE_FIRE_FLICKER;
        _updateSegState = true;
      }
      if (results.value == rmt_commands[SEG_UP]) { // Select segment up
        last_remote_cmd = 0;
        if ((State.segment < Config.segments - 1) && (State.mode == HOLD)) {
          prevsegment = State.segment;
          State.segment = State.segment + 1;
          getSegmentParams(State.segment);
        }
        _updateSegState = true;
      }
      if (results.value == rmt_commands[SEG_DOWN]) { // Select segment down
        last_remote_cmd = 0;
        if ((State.segment > 0) && (State.mode == HOLD)) {
          prevsegment = State.segment;
          State.segment = State.segment - 1;
          getSegmentParams(State.segment);
        }
        _updateSegState = true;
      }
      irrecv.resume();  // Receive the next value
    }
    #if defined(ENABLE_STATE_SAVE)
      if (_updateState) {
        if(save_state.active()) save_state.detach();
        save_state.once(3, tickerSaveState);
      }
      if (_updateSegState) {
        State.mode = SET;
        if(save_seg_state.active()) save_seg_state.detach();
        save_seg_state.once(3, tickerSaveSegmentState);
      }
    #endif
      _updateState = false;
      _updateSegState = false;
  }
#endif
