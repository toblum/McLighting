// ***************************************************************************
// Request handlers
// ***************************************************************************

// Prototypes
void   handleAutoStart(void);
String listStatusJSON(void);

#if defined(ENABLE_E131)
void handleE131(){
  if (!e131.isEmpty())
  {
    e131_packet_t packet;
    e131.pull(&packet); // Pull packet from ring buffer

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
    uint16_t len = (128 + multipacketOffset > NUMLEDS) ? (NUMLEDS - multipacketOffset) : 128;
  #else*/
    uint16_t multipacketOffset = (universe - START_UNIVERSE) * 170; //if more than 170 LEDs * 3 colors = 510 channels, client will send in next higher universe
    if (NUMLEDS <= multipacketOffset) return;
    uint16_t len = (170 + multipacketOffset > NUMLEDS) ? (NUMLEDS - multipacketOffset) : 170;
/*  #endif */
    for (uint16_t i = 0; i < len; i++){
      uint16_t j = i * 3;
/*  #if defined(RGBW)
      strip.setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], data[j + 3]);
  #else */
      strip.setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], 0);
/*  #endif */
    }
    strip.show();
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
  if ((server.arg("s") != "") && (server.arg("s").toInt() >= 0) && (server.arg("s").toInt() <= 255)) {
  ws2812fx_speed = constrain(server.arg("s").toInt(), 0, 255);
  }

  if ((server.arg("m") != "") && (server.arg("m").toInt() >= 0) && (server.arg("m").toInt() <= strip.getModeCount())) {
    ws2812fx_mode = constrain(server.arg("m").toInt(), 0, strip.getModeCount() - 1);
  }

  if ((server.arg("c") != "") && (server.arg("c").toInt() >= 0) && (server.arg("c").toInt() <= 100)) {
    brightness = constrain((int) server.arg("c").toInt() * 2.55, 0, 255);
  } else if ((server.arg("p") != "") && (server.arg("p").toInt() >= 0) && (server.arg("p").toInt() <= 255)) {
    brightness = constrain(server.arg("p").toInt(), 0, 255);
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
  DBG_OUTPUT_PORT.print("Get Args: ");
  DBG_OUTPUT_PORT.println(listStatusJSON());
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
  DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: R: [%u] G: [%u] B: [%u] W: [%u]\n", main_color.red, main_color.green, main_color.blue, main_color.white);
  ws2812fx_mode = FX_MODE_STATIC;
  mode = SET_ALL;
}

void handleSetSingleLED(uint8_t * mypayload, uint8_t firstChar = 0) {
  // decode led index
  char templed[3];
  strncpy (templed, (const char *) &mypayload[firstChar], 2 );
  uint8_t led = atoi(templed);

  DBG_OUTPUT_PORT.printf("led value: [%i]. Entry threshold: <= [%i] (=> %s)\n", led, strip.numPixels(), mypayload );
  if (led <= strip.numPixels()) {
    char redhex[3];
    char greenhex[3];
    char bluehex[3];
    char whitehex[3];
    strncpy (redhex, (const char *) &mypayload[2 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[6 + firstChar], 2 );
    strncpy (whitehex, (const char *) &mypayload[8 + firstChar], 2 );
    ledstates[led].red =   strtol(redhex, NULL, 16);
    ledstates[led].green = strtol(greenhex, NULL, 16);
    ledstates[led].blue =  strtol(bluehex, NULL, 16);
    ledstates[led].white =  strtol(whitehex, NULL, 16);
    DBG_OUTPUT_PORT.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s] rgb.white: [%s]\n", redhex, greenhex, bluehex, whitehex);
    DBG_OUTPUT_PORT.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i] rgb.white: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16), strtol(whitehex, NULL, 16));
    DBG_OUTPUT_PORT.printf("WS: Set single led [%i] to [%i] [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, ledstates[led].white, mypayload);
    strip.setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, ledstates[led].white);
    strip.show();
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
    char startled[3] = { 0, 0, 0 };
    char endled[3] = { 0, 0, 0 };
    char colorval[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    strncpy ( startled, (const char *) &nextCommand[0], 2 );
    strncpy ( endled, (const char *) &nextCommand[2], 2 );
    strncpy ( colorval, (const char *) &nextCommand[4], 8 );
    int rangebegin = atoi(startled);
    int rangeend = atoi(endled);
    DBG_OUTPUT_PORT.printf("Setting RANGE from [%i] to [%i] as color [%s] \n", rangebegin, rangeend, colorval);

    while ( rangebegin <= rangeend ) {
      char rangeData[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      if ( rangebegin < 10 ) {
        // Create the valid 'nextCommand' structure
        sprintf(rangeData, "0%d%s", rangebegin, colorval);
      }
      if ( rangebegin >= 10 ) {
        // Create the valid 'nextCommand' structure
        sprintf(rangeData, "%d%s", rangebegin, colorval);
      }
      // Set one LED
      handleSetSingleLED((uint8_t*) rangeData, 0);
      rangebegin++;
    }

    // Next Range at R
    nextCommand = (uint8_t*) strtok(NULL, "R");
  }
}

void setModeByStateString(String saved_state_string) {
  String str_mode = getValue(saved_state_string, '|', 1);
  mode = static_cast<MODE>(str_mode.toInt());
  DBG_OUTPUT_PORT.print("mode: ");
  DBG_OUTPUT_PORT.println(mode);
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
#endif
}

void handleSetWS2812FXMode(uint8_t * mypayload) {
  if (isDigit(mypayload[1])) {
    ws2812fx_mode = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
    ws2812fx_mode = constrain(ws2812fx_mode, 0, strip.getModeCount() - 1);
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
  }    
}

String listStatusJSON(void) {
  //uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip.getMode(); 
  const size_t bufferSize = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(6) + 150;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["mode"] = (uint8_t) mode;
  root["ws2812fx_mode"] = ws2812fx_mode;
  root["ws2812fx_mode_name"] = strip.getModeName(ws2812fx_mode);
  //root["ws2812fx_mode"] = tmp_mode;
  //root["ws2812fx_mode_name"] = strip.getModeName(tmp_mode);
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
  String json;
  serializeJson(root, json);
  jsonBuffer.clear();
  return json;
}

void getStatusJSON() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", listStatusJSON() );
}

String listModesJSON(void) {
  const size_t bufferSize = JSON_ARRAY_SIZE(strip.getModeCount() + 3) + (strip.getModeCount() + 3)*JSON_OBJECT_SIZE(2) + 2000;
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
  for (uint8_t i = 0; i < strip.getModeCount(); i++) {
    JsonObject object = root.createNestedObject();
    object["mode"] = i;
    object["name"] = strip.getModeName(i);
  }
  String json;
  serializeJson(root, json);
  jsonBuffer.clear();
  return json;
}

void getModesJSON() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", listModesJSON() );
}

// ***************************************************************************
// HTTP request handlers
// ***************************************************************************
void handleMinimalUpload() {
  char temp[1500];

  snprintf ( temp, 1500,
   "<!DOCTYPE html>\
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
    </html>"
  );
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
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
int autoCount = 0;

void autoTick() {
  uint32_t setcolors[] = {autoParams[autoCount][0],autoParams[autoCount][1],autoParams[autoCount][2]};
  strip.setColors(0, setcolors);
  strip.setSpeed(convertSpeed((uint8_t)autoParams[autoCount][3]));
  strip.setMode((uint8_t)autoParams[autoCount][4]);
  autoTicker.once((float)autoParams[autoCount][5], autoTick);
  DBG_OUTPUT_PORT.print("autoTick ");
  DBG_OUTPUT_PORT.println(autoCount);

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
      sprintf(mqtt_buf, "OK %s", payload);
    #endif
    if (payload[2] == '#') {
      handleSetXtraColor(payload);
      DBG_OUTPUT_PORT.printf("Set 3rd color to: R: [%u] G: [%u] B: [%u] W: [%u]\n",  xtra_color.red, xtra_color.green, xtra_color.blue, xtra_color.white);
    } else if (payload[1] == '#') {
      handleSetBackColor(payload);
      DBG_OUTPUT_PORT.printf("Set 2nd color to: R: [%u] G: [%u] B: [%u] W: [%u]\n",  back_color.red, back_color.green, back_color.blue, back_color.white);
    } else {
      handleSetMainColor(payload);
      DBG_OUTPUT_PORT.printf("Set main color to: R: [%u] G: [%u] B: [%u] W: [%u]\n", main_color.red, main_color.green, main_color.blue, main_color.white);
    }
  }

  // ? ==> Set speed
  if (payload[0] == '?') {
    uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    ws2812fx_speed = constrain(d, 0, 255);
    mode = SET_SPEED;
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set speed to: [%u]\n", ws2812fx_speed);
  }

  // % ==> Set brightness
  if (payload[0] == '%') {
    uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    brightness = constrain(b, 0, 255);
    mode = SET_BRIGHTNESS;
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set brightness to: [%u]\n", brightness);
  }

  // * ==> Set main color and light all LEDs (Shortcut)
  if (payload[0] == '*') {
    handleSetAllMode(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set main color and light all LEDs [%s]\n", payload);
  }

  // ! ==> Set single LED in given color
  if (payload[0] == '!') {
    handleSetSingleLED(payload, 1);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      sprintf(mqtt_buf, "OK %s", payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set single LED in given color [%s]\n", payload);
  }

  // + ==> Set multiple LED in the given colors
  if (payload[0] == '+') {
    handleSetDifferentColors(payload);
    Dbg_Prefix(mqtt, num);
    #if defined(ENABLE_MQTT)
      sprintf(mqtt_buf, "OK %s", payload);
    #endif
    DBG_OUTPUT_PORT.printf("Set multiple LEDs in given color [%s]\n", payload);
  }

  // + ==> Set range of LEDs in the given color
  if (payload[0] == 'R') {
    handleRangeDifferentColors(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set range of LEDs in given color [%s]\n", payload);
    #if defined(ENABLE_MQTT)
      sprintf(mqtt_buf, "OK %s", payload);
    #endif
  }

  #if defined(ENABLE_LEGACY_ANIMATIONS)
    // = ==> Activate named mode
    if (payload[0] == '=') {
      // we get mode data
      handleSetNamedMode(payload);
      Dbg_Prefix(mqtt, num);
      DBG_OUTPUT_PORT.printf("Activated mode [%u]!\n", mode);
    }
  #endif

  // $ ==> Get status Info.
  if (payload[0] == '$') {
    String json = listStatusJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: ");
      #if defined(ENABLE_MQTT)
        #if ENABLE_MQTT == 0
          mqtt_client.publish(mqtt_outtopic, json.c_str());
        #endif
        #if ENABLE_MQTT == 1
          amqttClient.publish(mqtt_outtopic, qospub, false, json.c_str());
        #endif
      #endif
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
      webSocket.sendTXT(num, json);
    }
    DBG_OUTPUT_PORT.println("Get status info: " + json);
  }

  // ~ ==> Get WS2812 modes.
  if (payload[0] == '~') {
    String json = listModesJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
      #if defined(ENABLE_MQTT)
        #if ENABLE_MQTT == 0
          // TODO: Fix this, doesn't return anything. Too long?
          // Hint: https://github.com/knolleary/pubsubclient/issues/110
          DBG_OUTPUT_PORT.printf("Error: Not implemented. Message too large for pubsubclient.");
          mqtt_client.publish(mqtt_outtopic, "ERROR: Not implemented. Message too large for pubsubclient.");
          //String json_modes = listModesJSON();
          //DBG_OUTPUT_PORT.printf(json_modes.c_str());
      
          //int res = mqtt_client.publish(mqtt_outtopic, json_modes.c_str(), json_modes.length());
          //DBG_OUTPUT_PORT.printf("Result: %d / %d", res, json_modes.length());
        #endif
        #if ENABLE_MQTT == 1
          amqttClient.publish(mqtt_outtopic, qospub, false, json.c_str());
        #endif  
      #endif
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
      webSocket.sendTXT(num, json);
    }
    DBG_OUTPUT_PORT.println("Get WS2812 modes.");
    DBG_OUTPUT_PORT.println(json);
  }

  // / ==> Set WS2812 mode.
  if (payload[0] == '/') {
    handleSetWS2812FXMode(payload);
    Dbg_Prefix(mqtt, num);
    DBG_OUTPUT_PORT.printf("Set WS2812 mode: [%s]\n", payload);
  }
}

// ***************************************************************************
// WS request handlers
// ***************************************************************************
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      DBG_OUTPUT_PORT.printf("WS: [%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        DBG_OUTPUT_PORT.printf("WS: [%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;

    case WStype_TEXT:
      DBG_OUTPUT_PORT.printf("WS: [%u] get Text: %s\n", num, payload);

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

     LEDState temp2rgb(unsigned int kelvin) {
       int tmp_internal = kelvin / 100.0;
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
       root["brightness"] = brightness;
       root["color_temp"] = color_temp;
       root["speed"] = ws2812fx_speed;
       //char modeName[30];
       //strncpy_P(modeName, (PGM_P)strip.getModeName(strip.getMode()), sizeof(modeName)); // copy from progmem
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
               root["effect"] = strip.getModeName(strip.getMode());
             }
           }
         }
       }
       #endif
      char buffer[measureJson(root) + 1];
      serializeJson(root, buffer, sizeof(buffer));
      jsonBuffer.clear();
      #if ENABLE_MQTT == 0
      mqtt_client.publish(mqtt_ha_state_out, buffer, true);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out, buffer);
      #endif
      #if ENABLE_MQTT == 1
      amqttClient.publish(mqtt_ha_state_out, 1, true, buffer);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out, buffer);
      #endif
      new_ha_mqtt_msg = false;
      ha_send_data.detach();
      DBG_OUTPUT_PORT.printf("Heap size: %u\n", ESP.getFreeHeap());
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
        unsigned int kelvin  = 1000000 / color_temp;
        main_color = temp2rgb(kelvin);
        mode = SET_COLOR;
      }

      if (root.containsKey("brightness")) {
        uint8_t json_brightness = constrain((uint8_t) root["brightness"], 0, 255); //fix #224
        if (json_brightness != brightness) {
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
        for (uint8_t i = 0; i < strip.getModeCount(); i++) {
          if(String(strip.getModeName(i)) == effectString) {
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

  #if ENABLE_MQTT == 1
    void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
    DBG_OUTPUT_PORT.print("MQTT: Recieved ["); DBG_OUTPUT_PORT.print(topic);
//    DBG_OUTPUT_PORT.print("]: "); DBG_OUTPUT_PORT.println(payload_in);
    uint8_t * payload = (uint8_t *) malloc(length + 1);
    memcpy(payload, payload_in, length);
    payload[length] = 0;
    DBG_OUTPUT_PORT.printf("]: %s\n", payload);
  #endif

  #if ENABLE_MQTT == 0
  void mqtt_callback(char* topic, byte* payload_in, unsigned int length) {
    uint8_t * payload = (uint8_t *)malloc(length + 1);
    memcpy(payload, payload_in, length);
    payload[length] = 0;
    DBG_OUTPUT_PORT.printf("MQTT: Message arrived [%s]\n", payload);
  #endif
  
    #if defined(ENABLE_HOMEASSISTANT)
      if (strcmp(topic, mqtt_ha_state_in) == 0) {
        if (!processJson((char*)payload)) {
          return;
        }
        if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
      #if ENABLE_MQTT == 0
        } else if (strcmp(topic, (char *)mqtt_intopic) == 0) {
      #endif
      #if ENABLE_MQTT == 1
        } else if (strcmp(topic, mqtt_intopic) == 0) {
      #endif 
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
    while (!mqtt_client.connected() && mqtt_reconnect_retries < MQTT_MAX_RECONNECT_TRIES) {
      mqtt_reconnect_retries++;
      DBG_OUTPUT_PORT.printf("Attempting MQTT connection %d / %d ...\n", mqtt_reconnect_retries, MQTT_MAX_RECONNECT_TRIES);
      // Attempt to connect
      if (mqtt_client.connect(mqtt_clientid, mqtt_user, mqtt_pass)) {
        DBG_OUTPUT_PORT.println("MQTT connected!");
        // Once connected, publish an announcement...
        char * message = new char[18 + strlen(HOSTNAME) + 1];
        strcpy(message, "McLighting ready: ");
        strcat(message, HOSTNAME);
        mqtt_client.publish(mqtt_outtopic, message);
        // ... and resubscribe
        mqtt_client.subscribe(mqtt_intopic, qossub);
        #if defined(ENABLE_HOMEASSISTANT)
          ha_send_data.detach();
          mqtt_client.subscribe(mqtt_ha_state_in, qossub);
          #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
            const size_t bufferSize = JSON_ARRAY_SIZE(strip.getModeCount()+ 4) + JSON_OBJECT_SIZE(11) + 1500;
            DynamicJsonDocument jsonBuffer(bufferSize);
            JsonObject json = jsonBuffer.to<JsonObject>();
            json["name"] = HOSTNAME;
            #if defined(MQTT_HOME_ASSISTANT_0_84_SUPPORT)
            json["schema"] = "json";
            #else
            json["platform"] = "mqtt_json";
            #endif
            json["state_topic"] = mqtt_ha_state_out;
            json["command_topic"] = mqtt_ha_state_in;
            json["on_command_type"] = "first";
            json["brightness"] = "true";
            json["rgb"] = "true";
            json["optimistic"] = "false";
            json["color_temp"] = "true";
            json["effect"] = "true";
            JsonArray effect_list = json.createNestedArray("effect_list");
            effect_list.add("OFF");
            effect_list.add("AUTO");
            #if defined(ENABLE_TV)
              effect_list.add("TV");
            #endif
            #if defined(ENABLE_E131)
               effect_list.add("E131");
            #endif
            for (uint8_t i = 0; i < strip.getModeCount(); i++) {
              effect_list.add(strip.getModeName(i));
            }
            char buffer[measureJson(json) + 1];
            serializeJson(json, buffer, sizeof(buffer));
            jsonBuffer.clear();
            mqtt_client.publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), buffer, true);
          #endif
        #endif

        DBG_OUTPUT_PORT.printf("MQTT topic in: %s\n", mqtt_intopic);
        DBG_OUTPUT_PORT.printf("MQTT topic out: %s\n", mqtt_outtopic);
      } else {
        DBG_OUTPUT_PORT.print("failed, rc=");
        DBG_OUTPUT_PORT.print(mqtt_client.state());
        DBG_OUTPUT_PORT.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if (mqtt_reconnect_retries >= MQTT_MAX_RECONNECT_TRIES) {
      DBG_OUTPUT_PORT.printf("MQTT connection failed, giving up after %d tries ...\n", mqtt_reconnect_retries);
    }
  }
  #endif
  #if ENABLE_MQTT == 1
    void connectToWifi() {
      DBG_OUTPUT_PORT.println("Re-connecting to Wi-Fi...");
      WiFi.setSleepMode(WIFI_NONE_SLEEP);
      WiFi.mode(WIFI_STA);
      WiFi.begin();
    }

    void connectToMqtt() {
      DBG_OUTPUT_PORT.println("Connecting to MQTT...");
      amqttClient.connect();
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
      char * message = new char[18 + strlen(HOSTNAME) + 1];
      strcpy(message, "McLighting ready: ");
      strcat(message, HOSTNAME);
      amqttClient.publish(mqtt_outtopic, qospub, false, message);
      //Subscribe
      uint16_t packetIdSub1 = amqttClient.subscribe((char *)mqtt_intopic, qossub);
      DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub1);
      #if defined(ENABLE_HOMEASSISTANT)
        ha_send_data.detach();
        uint16_t packetIdSub2 = amqttClient.subscribe((char *)mqtt_ha_state_in, qossub);
        DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub2);
        #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
          const size_t bufferSize = JSON_ARRAY_SIZE(strip.getModeCount()+ 4) + JSON_OBJECT_SIZE(11) + 1500;
          DynamicJsonDocument jsonBuffer(bufferSize);
          JsonObject json = jsonBuffer.to<JsonObject>();
          json["name"] = HOSTNAME;
          #if defined(MQTT_HOME_ASSISTANT_0_84_SUPPORT)
          json["schema"] = "json";
          #else
          json["platform"] = "mqtt_json";
          #endif
          json["state_topic"] = mqtt_ha_state_out;
          json["command_topic"] = mqtt_ha_state_in;
          json["on_command_type"] = "first";
          json["brightness"] = "true";
          json["rgb"] = "true";
          json["optimistic"] = "false";
          json["color_temp"] = "true";
          json["effect"] = "true";
          JsonArray effect_list = json.createNestedArray("effect_list");
          effect_list.add("OFF");
          effect_list.add("AUTO");
          #if defined(ENABLE_TV)
            effect_list.add("TV");
          #endif
          #if defined(ENABLE_E131)
             effect_list.add("E131");
          #endif
          for (uint8_t i = 0; i < strip.getModeCount(); i++) {
            effect_list.add(strip.getModeName(i));
          }
          char buffer[measureJson(json) + 1];
          serializeJson(json, buffer, sizeof(buffer));
          jsonBuffer.clear();
          amqttClient.publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), qospub, true, buffer);
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
    DBG_OUTPUT_PORT.printf("Short button press\n");
    if (mode == OFF) {
      setModeByStateString(BTN_MODE_SHORT);
      mode = SET_ALL;
    } else {
      mode = OFF;
    }
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    DBG_OUTPUT_PORT.printf("Medium button press\n");
    setModeByStateString(BTN_MODE_MEDIUM);
    mode = SET_ALL;
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DBG_OUTPUT_PORT.printf("Long button press\n");
    setModeByStateString(BTN_MODE_LONG);
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
    DBG_OUTPUT_PORT.printf("Short GY-33 button press\n");
//    tcs.setConfig(MCU_LED_04, MCU_WHITE_OFF);
//    delay(500);
    uint16_t red, green, blue, cl, ct, lux;
    tcs.getRawData(&red, &green, &blue, &cl, &lux, &ct);
    DBG_OUTPUT_PORT.printf("Raw Colors: R: [%d] G: [%d] B: [%d] Clear: [%d] Lux: [%d] Colortemp: [%d]\n", (int)red, (int)green, (int)blue, (int)cl, (int)lux, (int)ct);
    uint8_t r, g, b, col, conf;
    tcs.getData(&r, &g, &b, &col, &conf);
    DBG_OUTPUT_PORT.printf("Colors: R: [%d] G: [%d] B: [%d] Color: [%d] Conf: [%d]\n", (int)r, (int)g, (int)b, (int)col, (int)conf);
    main_color.red = (pow((r/255.0), 2.5)*255); main_color.green = (pow((g/255.0), 2.5)*255); main_color.blue = (pow((b/255.0), 2.5)*255);main_color.white = 0; 
    ws2812fx_mode = 0;
    mode = SET_ALL;
//    tcs.setConfig(MCU_LED_OFF, MCU_WHITE_OFF);
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress_gy33() {   
      tcs.setConfig(MCU_LED_06, MCU_WHITE_OFF);
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
  #if ENABLE_STATE_SAVE == 1
    bool updateFS = false;
    
    #if defined(ENABLE_MQTT)
      // Write configuration to FS JSON
      bool writeConfigFS(bool saveConfig){
        if (saveConfig) {
          //FS save
          updateFS = true;
          DBG_OUTPUT_PORT.print("Saving config: ");
          const size_t bufferSize = JSON_OBJECT_SIZE(4) + 150;
          DynamicJsonDocument jsonBuffer(bufferSize);
          JsonObject json = jsonBuffer.to<JsonObject>();
          json["mqtt_host"] = mqtt_host;
          json["mqtt_port"] = mqtt_port;
          json["mqtt_user"] = mqtt_user;
          json["mqtt_pass"] = mqtt_pass;
        
          //SPIFFS.remove("/config.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
          File configFile = SPIFFS.open("/config.json", "w");
          if (!configFile) DBG_OUTPUT_PORT.println("failed to open config file for writing");
      
          serializeJson(json, DBG_OUTPUT_PORT);
          serializeJson(json, configFile);
          jsonBuffer.clear();
          configFile.close();
          updateFS = false;
          return true;
          //end save
        } else {
          DBG_OUTPUT_PORT.println("SaveConfig is False!");
          return false;
        }
      }
      
      // Read search_str to FS
      bool readConfigFS() {
        //read configuration from FS JSON
        updateFS = true;
        if (SPIFFS.exists("/config.json")) {
          //file exists, reading and loading
          DBG_OUTPUT_PORT.print("Reading config file... ");
          File configFile = SPIFFS.open("/config.json", "r");
          if (configFile) {
            DBG_OUTPUT_PORT.println("Opened!");
            size_t size = configFile.size();
            std::unique_ptr<char[]> buf(new char[size]);
            configFile.readBytes(buf.get(), size);
            const size_t bufferSize = JSON_OBJECT_SIZE(4) + 150;
            DynamicJsonDocument jsonBuffer(bufferSize);
            DeserializationError error = deserializeJson(jsonBuffer, buf.get());
            DBG_OUTPUT_PORT.print("Config: ");
            if (!error) {
              DBG_OUTPUT_PORT.println(" Parsed!");
              JsonObject json = jsonBuffer.as<JsonObject>();
              serializeJson(json, DBG_OUTPUT_PORT);
              strcpy(mqtt_host, json["mqtt_host"]);
              strcpy(mqtt_port, json["mqtt_port"]);
              strcpy(mqtt_user, json["mqtt_user"]);
              strcpy(mqtt_pass, json["mqtt_pass"]);
              updateFS = false;
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
        }
        //end read
        updateFS = false;
        return false;
      }
    #endif
  
    bool writeStateFS(){
      updateFS = true;
      //save the strip state to FS JSON
      DBG_OUTPUT_PORT.print("Saving cfg: ");
      //SPIFFS.remove("/stripstate.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
      File configFile = SPIFFS.open("/stripstate.json", "w");
      if (!configFile) {
        DBG_OUTPUT_PORT.println("Failed!");
        updateFS = false;
        settings_save_state.detach();
        updateState = false;
        return false;
      }
      DBG_OUTPUT_PORT.println(listStatusJSON());
      configFile.print(listStatusJSON());
      configFile.close();
      updateFS = false;
      settings_save_state.detach();
      updateState = false;
      return true;
      //end save
    }
    
    bool readStateFS() {
      //read strip state from FS JSON
      updateFS = true;
      //if (resetsettings) { SPIFFS.begin(); SPIFFS.remove("/config.json"); SPIFFS.format(); delay(1000);}
      if (SPIFFS.exists("/stripstate.json")) {
        //file exists, reading and loading
        DBG_OUTPUT_PORT.print("Read cfg: ");
        File configFile = SPIFFS.open("/stripstate.json", "r");
        if (configFile) {
          size_t size = configFile.size();
          // Allocate a buffer to store contents of the file.
          std::unique_ptr<char[]> buf(new char[size]);
          configFile.readBytes(buf.get(), size);
          const size_t bufferSize = JSON_OBJECT_SIZE(5) + JSON_ARRAY_SIZE(12) + 500;
          DynamicJsonDocument jsonBuffer(bufferSize);
          DeserializationError error = deserializeJson(jsonBuffer, buf.get());
          if (!error) {
            JsonObject json = jsonBuffer.as<JsonObject>();
            serializeJson(json, DBG_OUTPUT_PORT);
            mode = static_cast<MODE>((int) json["mode"]);
            ws2812fx_mode = json["ws2812fx_mode"];
            ws2812fx_speed = json["speed"];
            brightness = json["brightness"];
            main_color.white = (uint8_t) json["color"][0];
            main_color.red = (uint8_t) json["color"][1];
            main_color.green = (uint8_t) json["color"][2];
            main_color.blue = (uint8_t) json["color"][3];
            back_color.white = (uint8_t) json["color"][4];
            back_color.red = (uint8_t) json["color"][5];
            back_color.green = (uint8_t) json["color"][6];
            back_color.blue = (uint8_t) json["color"][7];
            xtra_color.white = (uint8_t) json["color"][8];  
            xtra_color.red = (uint8_t) json["color"][9];
            xtra_color.green = (uint8_t) json["color"][10];
            xtra_color.blue = (uint8_t) json["color"][11];
            convertColors();
            strip.setMode(ws2812fx_mode);
            strip.setSpeed(convertSpeed(ws2812fx_speed));
            strip.setBrightness(brightness);
            strip.setColors(0, hex_colors);       
            updateFS = false;
            jsonBuffer.clear();
            return true;
          } else {
            DBG_OUTPUT_PORT.println("Failed to parse JSON!");
            jsonBuffer.clear();
          }
        } else {
          DBG_OUTPUT_PORT.println("Failed to open \"/stripstate.json\"");
        }
      } else {
        DBG_OUTPUT_PORT.println("Couldn't find \"/stripstate.json\"");
      }
      //end read
      updateFS = false;
      return false;
    }
  #endif

  #if ENABLE_STATE_SAVE == 0 
    // ***************************************************************************
    // EEPROM helper
    // ***************************************************************************
    String readEEPROM(int offset, int len) {
      String res = "";
      for (int i = 0; i < len; ++i)
      {
        res += char(EEPROM.read(i + offset));
        //DBG_OUTPUT_PORT.println(char(EEPROM.read(i + offset)));
      }
      DBG_OUTPUT_PORT.printf("readEEPROM(): %s\n", res.c_str());
      return res;
    }
    
    void writeEEPROM(int offset, int len, String value) {
      DBG_OUTPUT_PORT.printf("writeEEPROM(): %s\n", value.c_str());
      for (int i = 0; i < len; ++i)
      {
        if (i < value.length()) {
          EEPROM.write(i + offset, value[i]);
        } else {
          EEPROM.write(i + offset, 0);
        }
      }
    }
  #endif
#endif

#if defined(ENABLE_REMOTE)
// ***************************************************************************
// Request handler for IR remote support
// ***************************************************************************
void handleRemote() {
    if (irrecv.decode(&results)) {
      DBG_OUTPUT_PORT.print("IR Code: 0x");
      DBG_OUTPUT_PORT.print(uint64ToString(results.value, HEX));
      DBG_OUTPUT_PORT.println("");
      if (results.value == rmt_commands[REPEATCMD]) { //Repeat
        results.value = last_remote_cmd;
        chng = 5;
      } else {
        chng = 1;       
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
        if ((ws2812fx_mode < strip.getModeCount()-1) && (mode == HOLD)) {
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
