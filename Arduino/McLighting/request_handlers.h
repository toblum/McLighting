// ***************************************************************************
// Request handlers
// ***************************************************************************
void getArgs() {
  if (server.arg("rgb") != "") {
    uint32_t rgb = (uint32_t) strtol(server.arg("rgb").c_str(), NULL, 16);
    main_color.red = ((rgb >> 16) & 0xFF);
    main_color.green = ((rgb >> 8) & 0xFF);
    main_color.blue = ((rgb >> 0) & 0xFF);
  } else {
    main_color.red = server.arg("r").toInt();
    main_color.green = server.arg("g").toInt();
    main_color.blue = server.arg("b").toInt();
  }
  ws2812fx_speed = constrain(server.arg("s").toInt(), 0, 255);
  if (server.arg("s") == "") {
    ws2812fx_speed = 196;
  }

  if (server.arg("m") != "") {
    ws2812fx_mode = constrain(server.arg("m").toInt(), 0, strip.getModeCount() - 1);
  }

  main_color.red = constrain(main_color.red, 0, 255);
  main_color.green = constrain(main_color.green, 0, 255);
  main_color.blue = constrain(main_color.blue, 0, 255);

  DBG_OUTPUT_PORT.print("Mode: ");
  DBG_OUTPUT_PORT.print(mode);
  DBG_OUTPUT_PORT.print(", Color: ");
  DBG_OUTPUT_PORT.print(main_color.red);
  DBG_OUTPUT_PORT.print(", ");
  DBG_OUTPUT_PORT.print(main_color.green);
  DBG_OUTPUT_PORT.print(", ");
  DBG_OUTPUT_PORT.print(main_color.blue);
  DBG_OUTPUT_PORT.print(", Speed:");
  DBG_OUTPUT_PORT.print(ws2812fx_speed);
  DBG_OUTPUT_PORT.print(", Brightness:");
  DBG_OUTPUT_PORT.println(brightness);
}


long convertSpeed(int mcl_speed) {
  long ws2812_speed = mcl_speed * 256;
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
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
}

void handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, main_color.red, main_color.green, main_color.blue);
  }
  strip.show();
  DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  exit_func = true;
  mode = ALL;
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
    strncpy (redhex, (const char *) &mypayload[2 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[6 + firstChar], 2 );
    ledstates[led].red =   strtol(redhex, NULL, 16);
    ledstates[led].green = strtol(greenhex, NULL, 16);
    ledstates[led].blue =  strtol(bluehex, NULL, 16);
    DBG_OUTPUT_PORT.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s]\n", redhex, greenhex, bluehex);
    DBG_OUTPUT_PORT.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16));
    DBG_OUTPUT_PORT.printf("WS: Set single led [%i] to [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, mypayload);


    strip.setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue);
    strip.show();
  }
  exit_func = true;
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
    char colorval[7] = { 0, 0, 0, 0, 0, 0, 0 };
    strncpy ( startled, (const char *) &nextCommand[0], 2 );
    strncpy ( endled, (const char *) &nextCommand[2], 2 );
    strncpy ( colorval, (const char *) &nextCommand[4], 6 );
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

  DBG_OUTPUT_PORT.printf("ws2812fx_mode: %d\n", ws2812fx_mode);
  DBG_OUTPUT_PORT.printf("ws2812fx_speed: %d\n", ws2812fx_speed);
  DBG_OUTPUT_PORT.printf("brightness: %d\n", brightness);
  DBG_OUTPUT_PORT.printf("main_color.red: %d\n", main_color.red);
  DBG_OUTPUT_PORT.printf("main_color.green: %d\n", main_color.green);
  DBG_OUTPUT_PORT.printf("main_color.blue: %d\n", main_color.blue);

  strip.setMode(ws2812fx_mode);
  strip.setSpeed(convertSpeed(ws2812fx_speed));
  strip.setBrightness(brightness);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
}

void handleSetNamedMode(String str_mode) {
  exit_func = true;

  if (str_mode.startsWith("=off")) {
    mode = OFF;
  }
  if (str_mode.startsWith("=all")) {
    mode = ALL;
  }
  if (str_mode.startsWith("=wipe")) {
    mode = WIPE;
  }
  if (str_mode.startsWith("=rainbow")) {
    mode = RAINBOW;
  }
  if (str_mode.startsWith("=rainbowCycle")) {
    mode = RAINBOWCYCLE;
  }
  if (str_mode.startsWith("=theaterchase")) {
    mode = THEATERCHASE;
  }
  if (str_mode.startsWith("=twinkleRandom")) {
    mode = TWINKLERANDOM;
  }
  if (str_mode.startsWith("=theaterchaseRainbow")) {
    mode = THEATERCHASERAINBOW;
  }
  if (str_mode.startsWith("=tv")) {
    mode = TV;
  }
}

void handleSetWS2812FXMode(uint8_t * mypayload) {
  mode = HOLD;
  uint8_t ws2812fx_mode = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode, 0, 255);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
  strip.setMode(ws2812fx_mode);
  strip.start();
}

char* listStatusJSON() {
  char json[255];

  char modeName[30];
  strncpy_P(modeName, (PGM_P)strip.getModeName(strip.getMode()), sizeof(modeName)); // copy from progmem

  snprintf(json, sizeof(json), "{\"mode\":%d, \"ws2812fx_mode\":%d, \"ws2812fx_mode_name\":\"%s\", \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}",
           mode, strip.getMode(), modeName, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  return json;
}

void getStatusJSON() {
  server.send ( 200, "application/json", listStatusJSON() );
}

String listModesJSON() {
  String modes = "[";
  for (uint8_t i = 0; i < strip.getModeCount(); i++) {
    modes += "{\"mode\":";
    modes += i;
    modes += ", \"name\":\"";
    modes += strip.getModeName(i);
    modes += "\"},";
  }
  modes += "{}]";
  return modes;
}

void getModesJSON() {
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

// automatic cycling
Ticker autoTicker;
int autoCount = 0;

void autoTick() {
  strip.setColor(autoParams[autoCount][0]);
  strip.setSpeed(convertSpeed((uint8_t)autoParams[autoCount][1]));
  strip.setMode((uint8_t)autoParams[autoCount][2]);
  autoTicker.once((float)autoParams[autoCount][3], autoTick);
  DBG_OUTPUT_PORT.print("autoTick ");
  DBG_OUTPUT_PORT.println(autoCount);

  autoCount++;
  if (autoCount >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount = 0;
}

void handleAutoStart() {
  autoCount = 0;
  autoTick();
  strip.start();
}

void handleAutoStop() {
  autoTicker.detach();
  strip.stop();
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

      // # ==> Set main color
      if (payload[0] == '#') {
        handleSetMainColor(payload);
        DBG_OUTPUT_PORT.printf("Set main color to: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
        webSocket.sendTXT(num, "OK");
      }

      // ? ==> Set speed
      if (payload[0] == '?') {
        uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        ws2812fx_speed = constrain(d, 0, 255);
        strip.setSpeed(convertSpeed(ws2812fx_speed));
        DBG_OUTPUT_PORT.printf("WS: Set speed to: [%u]\n", ws2812fx_speed);
        webSocket.sendTXT(num, "OK");
      }

      // % ==> Set brightness
      if (payload[0] == '%') {
        uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        brightness = ((b >> 0) & 0xFF);
        DBG_OUTPUT_PORT.printf("WS: Set brightness to: [%u]\n", brightness);
        strip.setBrightness(brightness);
        webSocket.sendTXT(num, "OK");
      }

      // * ==> Set main color and light all LEDs (Shortcut)
      if (payload[0] == '*') {
        handleSetAllMode(payload);
        webSocket.sendTXT(num, "OK");
      }

      // ! ==> Set single LED in given color
      if (payload[0] == '!') {
        handleSetSingleLED(payload, 1);
        webSocket.sendTXT(num, "OK");
      }

      // + ==> Set multiple LED in the given colors
      if (payload[0] == '+') {
        handleSetDifferentColors(payload);
        webSocket.sendTXT(num, "OK");
      }

      // + ==> Set range of LEDs in the given color
      if (payload[0] == 'R') {
        handleRangeDifferentColors(payload);
        webSocket.sendTXT(num, "OK");
      }

      // = ==> Activate named mode
      if (payload[0] == '=') {
        // we get mode data
        String str_mode = String((char *) &payload[0]);

        handleSetNamedMode(str_mode);

        DBG_OUTPUT_PORT.printf("Activated mode [%u]!\n", mode);
        webSocket.sendTXT(num, "OK");
      }

      // $ ==> Get status Info.
      if (payload[0] == '$') {
        DBG_OUTPUT_PORT.printf("Get status info.");

        String json = listStatusJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
      }

      // ~ ==> Get WS2812 modes.
      if (payload[0] == '~') {
        DBG_OUTPUT_PORT.printf("Get WS2812 modes.");

        String json = listModesJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
      }

      // / ==> Set WS2812 mode.
      if (payload[0] == '/') {
        handleSetWS2812FXMode(payload);
        webSocket.sendTXT(num, "OK");
      }

      // start auto cycling
      if (strcmp((char *)payload, "start") == 0 ) {
        handleAutoStart();
        webSocket.sendTXT(num, "OK");
      }

      // stop auto cycling
      if (strcmp((char *)payload, "stop") == 0 ) {
        handleAutoStop();
        webSocket.sendTXT(num, "OK");
      }
      break;
  }
}

void checkForRequests() {
  webSocket.loop();
  server.handleClient();
}


// ***************************************************************************
// MQTT callback / connection handler
// ***************************************************************************
#ifdef ENABLE_MQTT
void mqtt_callback(char* topic, byte* payload_in, unsigned int length) {
  uint8_t * payload = (uint8_t *)malloc(length + 1);
  memcpy(payload, payload_in, length);
  payload[length] = NULL;
  String payload_str = (char*)payload;
  DBG_OUTPUT_PORT.printf("MQTT: Message arrived [%s]\n", payload);

  if ((strcmp(topic,mqtt_ha_state_in.c_str())==0)){
    if (payload_str=="true" and !(animation_on)){
        mode = ALL;
        strip.setColor(main_color.red, main_color.green, main_color.blue);
        strip.start();
        DBG_OUTPUT_PORT.printf("MQTT: ON\n");
        mqtt_client.publish(mqtt_ha_state_out.c_str(), String("true").c_str());

    } else if (payload_str=="false"){
        mode = OFF;
        animation_on = false;
        strip.start();
        DBG_OUTPUT_PORT.printf("MQTT: OFF\n");
        mqtt_client.publish(mqtt_ha_state_out.c_str(), String("false").c_str());
    }

  } else if (strcmp(topic,mqtt_ha_effect_in.c_str())==0){
    char * effect_mqtt;
    animation_on = true;
    if (payload_str=="Static"){
      effect_mqtt = "/0";
    } else if (payload_str=="Blink"){
      effect_mqtt = "/1";
    } else if (payload_str=="Breath"){
      effect_mqtt = "/2";
    } else if (payload_str=="ColorWipe"){
      effect_mqtt = "/3";
    } else if (payload_str=="ColorWipeInverse"){
      effect_mqtt = "/4";
    } else if (payload_str=="ColorWipeReverse"){
      effect_mqtt = "/5";
    } else if (payload_str=="ColorWipeReverseInverse"){
      effect_mqtt = "/6";
    } else if (payload_str=="ColorWipeRandom"){
      effect_mqtt = "/7";
    } else if (payload_str=="RandomColor"){
      effect_mqtt = "/8";
    } else if (payload_str=="SingleDynamic"){
      effect_mqtt = "/9";
    } else if (payload_str=="MultiDynamic"){
      effect_mqtt = "/10";
    } else if (payload_str=="Rainbow"){
      effect_mqtt = "/11";
    } else if (payload_str=="RainbowCycle"){
      effect_mqtt = "/12";
    } else if (payload_str=="Scan"){
      effect_mqtt = "/13";
    } else if (payload_str=="DualScan"){
      effect_mqtt = "/14";
    } else if (payload_str=="Fade"){
      effect_mqtt = "/15";
    } else if (payload_str=="TheaterChase"){
      effect_mqtt = "/16";
    } else if (payload_str=="TheaterChaseRainbow"){
      effect_mqtt = "/17";
    } else if (payload_str=="RunningLights"){
      effect_mqtt = "/18";
    } else if (payload_str=="Twinkle"){
      effect_mqtt = "/19";
    } else if (payload_str=="TwinkleRandom"){
      effect_mqtt = "/20";
    } else if (payload_str=="TwinkleFade"){
      effect_mqtt = "/21";
    } else if (payload_str=="TwinkleFadeRandom"){
      effect_mqtt = "/22";
    } else if (payload_str=="Sparkle"){
      effect_mqtt = "/23";
    } else if (payload_str=="FlashSparkle"){
      effect_mqtt = "/24";
    } else if (payload_str=="HyperSparkle"){
      effect_mqtt = "/25";
    } else if (payload_str=="Strobe"){
      effect_mqtt = "/26";
    } else if (payload_str=="StrobeRainbow"){
      effect_mqtt = "/27";
    } else if (payload_str=="MultiStrobe"){
      effect_mqtt = "/28";
    } else if (payload_str=="BlinkRainbow"){
      effect_mqtt = "/29";
    } else if (payload_str=="ChaseWhite"){
      effect_mqtt = "/30";
    } else if (payload_str=="ChaseColor"){
      effect_mqtt = "/31";
    } else if (payload_str=="ChaseRandom"){
      effect_mqtt = "/32";
    } else if (payload_str=="ChaseRainbow"){
      effect_mqtt = "/33";
    } else if (payload_str=="ChaseFlash"){
      effect_mqtt = "/34";
    } else if (payload_str=="ChaseFlashRandom"){
      effect_mqtt = "/35";
    } else if (payload_str=="ChaseRainbowWhite"){
      effect_mqtt = "/36";
    } else if (payload_str=="ChaseBlackout"){
      effect_mqtt = "/37";
    } else if (payload_str=="ChaseBlackoutRainbow"){
      effect_mqtt = "/38";
    } else if (payload_str=="ColorSweepRandom"){
      effect_mqtt = "/39";
    } else if (payload_str=="RunningColor"){
      effect_mqtt = "/40";
    } else if (payload_str=="RunningRedBlue"){
      effect_mqtt = "/41";
    } else if (payload_str=="RunningRandom"){
      effect_mqtt = "/42";
    } else if (payload_str=="LarsonScanner"){
      effect_mqtt = "/43";
    } else if (payload_str=="Comet"){
      effect_mqtt = "/44";
    } else if (payload_str=="Fireworks"){
      effect_mqtt = "/45";
    } else if (payload_str=="FireworksRandom"){
      effect_mqtt = "/46";
    } else if (payload_str=="MerryChristmas"){
      effect_mqtt = "/47";
    } else if (payload_str=="FireFlicker"){
      effect_mqtt = "/48";
    } else if (payload_str=="FireFlickerSoft"){
      effect_mqtt = "/49";
    } else if (payload_str=="FireFlickerIntense"){
      effect_mqtt = "/50";
    } else if (payload_str=="CircusCombustus"){
      effect_mqtt = "/51";
    } else if (payload_str=="Halloween"){
      effect_mqtt = "/52";
    } else if (payload_str=="BicolorChase"){
      effect_mqtt = "/53";
    } else if (payload_str=="TricolorChase"){
      effect_mqtt = "/54";
    } else if (payload_str=="ICU"){
      effect_mqtt = "/55";
    }
    handleSetWS2812FXMode((uint8_t *)effect_mqtt);
    DBG_OUTPUT_PORT.printf("MQTT: Set WS2812 mode [%s]\n", effect_mqtt);
    mqtt_client.publish(mqtt_ha_effect_out.c_str(), (char*)payload);
    
  } else if (strcmp(topic,mqtt_ha_brightness_in.c_str())==0){
      uint8_t b = (uint8_t) strtol((const char *) &payload[0], NULL, 10);
      brightness = constrain(b, 0, 255);
      strip.setBrightness(brightness);
      DBG_OUTPUT_PORT.printf("MQTT: Set brightness to [%u]\n", brightness);
      mqtt_client.publish(mqtt_ha_brightness_out.c_str(), String((char *)payload).c_str());

  } else if (strcmp(topic,mqtt_ha_rgb_in.c_str())==0){
      int firstCommaIndex = payload_str.indexOf(',');
      int secondCommaIndex = payload_str.indexOf(',', firstCommaIndex+1);
      int rm = payload_str.substring(0, firstCommaIndex).toInt();
      int gm = payload_str.substring(firstCommaIndex+1, secondCommaIndex).toInt();
      int bm = payload_str.substring(secondCommaIndex+1).toInt();

      main_color.red = constrain(rm, 0, 255);
      main_color.green = constrain(gm, 0, 255);
      main_color.blue = constrain(bm, 0, 255);
      strip.setColor(main_color.red, main_color.green, main_color.blue);
      strip.start();
      mqtt_client.publish(mqtt_ha_rgb_out.c_str(), String((char *)payload).c_str());
  } else if (strcmp(topic,mqtt_ha_speed.c_str())==0){
      uint8_t d = (uint8_t) strtol((const char *) &payload[0], NULL, 10);
      ws2812fx_speed = constrain(d, 0, 255);
      strip.setSpeed(convertSpeed(ws2812fx_speed));
  } else if (strcmp(topic,(char *)mqtt_intopic)==0){
  
  // # ==> Set main color
  if (payload[0] == '#') {
    handleSetMainColor(payload);
    DBG_OUTPUT_PORT.printf("MQTT: Set main color to [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // ? ==> Set speed
  if (payload[0] == '?') {
    uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    ws2812fx_speed = constrain(d, 0, 255);
    strip.setSpeed(convertSpeed(ws2812fx_speed));
    DBG_OUTPUT_PORT.printf("MQTT: Set speed to [%u]\n", ws2812fx_speed);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // % ==> Set brightness
  if (payload[0] == '%') {
    uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    brightness = constrain(b, 0, 255);
    strip.setBrightness(brightness);
    DBG_OUTPUT_PORT.printf("MQTT: Set brightness to [%u]\n", brightness);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // * ==> Set main color and light all LEDs (Shortcut)
  if (payload[0] == '*') {
    handleSetAllMode(payload);
    DBG_OUTPUT_PORT.printf("MQTT: Set main color and light all LEDs [%s]\n", payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // ! ==> Set single LED in given color
  if (payload[0] == '!') {
    handleSetSingleLED(payload, 1);
    DBG_OUTPUT_PORT.printf("MQTT: Set single LED in given color [%s]\n", payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // + ==> Set multiple LED in the given colors
  if (payload[0] == '+') {
    handleSetDifferentColors(payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // R ==> Set range of LEDs in the given colors
  if (payload[0] == 'R') {
    handleRangeDifferentColors(payload);
    DBG_OUTPUT_PORT.printf("MQTT: Set range of LEDS to single color: [%s]\n", payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // = ==> Activate named mode
  if (payload[0] == '=') {
    String str_mode = String((char *) &payload[0]);
    handleSetNamedMode(str_mode);
    DBG_OUTPUT_PORT.printf("MQTT: Activate named mode [%s]\n", payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  // $ ==> Get status Info.
  if (payload[0] == '$') {
    DBG_OUTPUT_PORT.printf("MQTT: Get status info.\n");
    //String json_str = listStatusJSON();
    //char* json_str = "";
    DBG_OUTPUT_PORT.println("MQTT: Out: "+String(listStatusJSON()));
    mqtt_client.publish(mqtt_outtopic, listStatusJSON());
  }

  // ~ ==> Get WS2812 modes.
  // TODO: Fix this, doesn't return anything. Too long?
  // Hint: https://github.com/knolleary/pubsubclient/issues/110
  if (payload[0] == '~') {
    DBG_OUTPUT_PORT.printf("MQTT: Get WS2812 modes.\n");
    DBG_OUTPUT_PORT.printf("Error: Not implemented. Message too large for pubsubclient.");
    mqtt_client.publish(mqtt_outtopic, "ERROR: Not implemented. Message too large for pubsubclient.");
    //String json_modes = listModesJSON();
    //DBG_OUTPUT_PORT.printf(json_modes.c_str());

    //int res = mqtt_client.publish(mqtt_outtopic, json_modes.c_str(), json_modes.length());
    //DBG_OUTPUT_PORT.printf("Result: %d / %d", res, json_modes.length());
  }

  // / ==> Set WS2812 mode.
  if (payload[0] == '/') {
    handleSetWS2812FXMode(payload);
    DBG_OUTPUT_PORT.printf("MQTT: Set WS2812 mode [%s]\n", payload);
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
  }

  }
  free(payload);
}

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
      mqtt_client.subscribe(mqtt_intopic);
      mqtt_client.subscribe(String(mqtt_ha + "#").c_str());

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


// ***************************************************************************
// Button management
// ***************************************************************************
#ifdef ENABLE_BUTTON
void shortKeyPress() {
  DBG_OUTPUT_PORT.printf("Short button press\n");
  if (buttonState == false) {
    setModeByStateString(BTN_MODE_SHORT);
    buttonState = true;
  } else {
    mode = OFF;
    buttonState = false;
  }
}

// called when button is kept pressed for less than 2 seconds
void mediumKeyPress() {
  DBG_OUTPUT_PORT.printf("Medium button press\n");
  setModeByStateString(BTN_MODE_MEDIUM);
}

// called when button is kept pressed for 2 seconds or more
void longKeyPress() {
  DBG_OUTPUT_PORT.printf("Long button press\n");
  setModeByStateString(BTN_MODE_LONG);
}

void button() {
  if (millis() - keyPrevMillis >= keySampleIntervalMs) {
    keyPrevMillis = millis();

    byte currKeyState = digitalRead(BUTTON);

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
