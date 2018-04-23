// ***************************************************************************
// Request handlers
// ***************************************************************************
#ifdef ENABLE_HOMEASSISTANT
void tickerSendState(){
  new_ha_mqtt_msg = true;
}
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
void tickerSpiffsSaveState(){
  updateStateFS = true;
}
#endif

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
  
  if (server.arg("c").toInt() > 0) {
    brightness = constrain((int) server.arg("c").toInt() * 2.55, 0, 255);
  } else if (server.arg("p").toInt() > 0) {
    brightness = constrain(server.arg("p").toInt(), 0, 255);
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
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
//  strip.setColor(main_color.red, main_color.green, main_color.blue);
  mode = SETCOLOR;
}

void handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

//  for (int i = 0; i < strip.numPixels(); i++) {
//    strip.setPixelColor(i, main_color.red, main_color.green, main_color.blue);
//  }
//  strip.show();
  DBG_OUTPUT_PORT.printf("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  #ifdef ENABLE_LEGACY_ANIMATIONS
    exit_func = true;
  #endif
  ws2812fx_mode = FX_MODE_STATIC;
  mode = SET_MODE;
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
  #ifdef ENABLE_LEGACY_ANIMATIONS
    exit_func = true;
  #endif
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

#ifdef ENABLE_LEGACY_ANIMATIONS
  void handleSetNamedMode(String str_mode) {
    exit_func = true;
  
    if (str_mode.startsWith("=off")) {
      mode = OFF;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = false;
      #endif
    }
    if (str_mode.startsWith("=all")) {
      ws2812fx_mode = FX_MODE_STATIC;
      mode = SET_MODE;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=wipe")) {
      mode = WIPE;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=rainbow")) {
      mode = RAINBOW;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=rainbowCycle")) {
      mode = RAINBOWCYCLE;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=theaterchase")) {
      mode = THEATERCHASE;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=twinkleRandom")) {
      mode = TWINKLERANDOM;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=theaterchaseRainbow")) {
      mode = THEATERCHASERAINBOW;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
    if (str_mode.startsWith("=tv")) {
      mode = TV;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
  }
#endif

void handleSetWS2812FXMode(uint8_t * mypayload) {
  mode = SET_MODE;
  uint8_t ws2812fx_mode_tmp = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode_tmp, 0, strip.getModeCount() - 1);
}

char* listStatusJSON() {
  char json[255];
  char modeName[30];
  uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip.getMode();
  
  strncpy_P(modeName, (PGM_P)strip.getModeName(tmp_mode), sizeof(modeName)); // copy from progmem
  snprintf(json, sizeof(json), "{\"mode\":%d, \"ws2812fx_mode\":%d, \"ws2812fx_mode_name\":\"%s\", \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}",
           mode, tmp_mode, modeName, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
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
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // ? ==> Set speed
      if (payload[0] == '?') {
        uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        ws2812fx_speed = constrain(d, 0, 255);
        strip.setSpeed(convertSpeed(ws2812fx_speed));
        DBG_OUTPUT_PORT.printf("WS: Set speed to: [%u]\n", ws2812fx_speed);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_HOMEASSISTANT
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // % ==> Set brightness
      if (payload[0] == '%') {
        uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        brightness = ((b >> 0) & 0xFF);
        DBG_OUTPUT_PORT.printf("WS: Set brightness to: [%u]\n", brightness);
        strip.setBrightness(brightness);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // * ==> Set main color and light all LEDs (Shortcut)
      if (payload[0] == '*') {
        handleSetAllMode(payload);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // ! ==> Set single LED in given color
      if (payload[0] == '!') {
        handleSetSingleLED(payload, 1);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // + ==> Set multiple LED in the given colors
      if (payload[0] == '+') {
        handleSetDifferentColors(payload);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // + ==> Set range of LEDs in the given color
      if (payload[0] == 'R') {
        handleRangeDifferentColors(payload);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      #ifdef ENABLE_LEGACY_ANIMATIONS
        // = ==> Activate named mode
        if (payload[0] == '=') {
          // we get mode data
          String str_mode = String((char *) &payload[0]);
  
          handleSetNamedMode(str_mode);
  
          DBG_OUTPUT_PORT.printf("Activated mode [%u]!\n", mode);
          webSocket.sendTXT(num, "OK");
          #ifdef ENABLE_MQTT
          mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
          #endif
          #ifdef ENABLE_AMQTT
          amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          #endif
          #ifdef ENABLE_HOMEASSISTANT
            if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
          #endif
        }
      #endif

      // $ ==> Get status Info.
      if (payload[0] == '$') {
        DBG_OUTPUT_PORT.printf("Get status info.");

        String json = listStatusJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, listStatusJSON());
        #endif
        #ifdef ENABLE_AMQTT
        String liststat = (String) listStatusJSON();
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, liststat.c_str());
        #endif
      }

      // ~ ==> Get WS2812 modes.
      if (payload[0] == '~') {
        DBG_OUTPUT_PORT.printf("Get WS2812 modes.");

        String json = listModesJSON();
        DBG_OUTPUT_PORT.println(json);
        webSocket.sendTXT(num, json);
        #ifdef ENABLE_MQTT
        DBG_OUTPUT_PORT.printf("Error: Not implemented. Message too large for pubsubclient.");
        mqtt_client.publish(mqtt_outtopic, "ERROR: Not implemented. Message too large for pubsubclient.");
        //String json_modes = listModesJSON();
        //DBG_OUTPUT_PORT.printf(json_modes.c_str());

        //int res = mqtt_client.publish(mqtt_outtopic, json_modes.c_str(), json_modes.length());
        //DBG_OUTPUT_PORT.printf("Result: %d / %d", res, json_modes.length());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("ERROR: Not implemented. Message too large for AsyncMQTT.").c_str());
        #endif
      }

      // / ==> Set WS2812 mode.
      if (payload[0] == '/') {
        handleSetWS2812FXMode(payload);
        webSocket.sendTXT(num, "OK");
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
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

#ifdef ENABLE_LEGACY_ANIMATIONS
  void checkForRequests() {
    webSocket.loop();
    server.handleClient();
    #ifdef ENABLE_MQTT
    mqtt_client.loop();
    #endif
  }
#endif


// ***************************************************************************
// MQTT callback / connection handler
// ***************************************************************************
#if defined(ENABLE_MQTT) or defined(ENABLE_AMQTT)

  #ifdef ENABLE_HOMEASSISTANT

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
      const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6);
      //StaticJsonBuffer<bufferSize> jsonBuffer;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.createObject();

      root["state"] = (stateOn) ? on_cmd : off_cmd;
      JsonObject& color = root.createNestedObject("color");
      color["r"] = main_color.red;
      color["g"] = main_color.green;
      color["b"] = main_color.blue;

      root["brightness"] = brightness;

      root["color_temp"] = color_temp;

      root["speed"] = ws2812fx_speed;

      char modeName[30];
      strncpy_P(modeName, (PGM_P)strip.getModeName(strip.getMode()), sizeof(modeName)); // copy from progmem
      root["effect"] = modeName;

      char buffer[root.measureLength() + 1];
      root.printTo(buffer, sizeof(buffer));

      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_ha_state_out.c_str(), buffer, true);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out.c_str(), buffer);
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_ha_state_out.c_str(), 1, true, buffer);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out.c_str(), buffer);
      #endif
      new_ha_mqtt_msg = false;
      ha_send_data.detach();
      DBG_OUTPUT_PORT.printf("Heap size: %u\n", ESP.getFreeHeap());
    }

    bool processJson(char* message) {
      const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 150;
      //StaticJsonBuffer<bufferSize> jsonBuffer;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(message);

      if (!root.success()) {
        DBG_OUTPUT_PORT.println("parseObject() failed");
        return false;
      }
      //DBG_OUTPUT_PORT.println("JSON ParseObject() done!");

      if (root.containsKey("state")) {
        const char* state_in = root["state"];
        if (strcmp(state_in, on_cmd) == 0 and !(animation_on)) {
          stateOn = true;
          ws2812fx_mode = FX_MODE_STATIC;
          mode = SET_MODE;
        }
        else if (strcmp(state_in, off_cmd) == 0) {
          stateOn = false;
          animation_on = false;
          mode = OFF;
          return true;
        }
      }

      if (root.containsKey("color")) {
        JsonObject& color = root["color"];
        main_color.red = (uint8_t) color["r"];
        main_color.green = (uint8_t) color["g"];
        main_color.blue = (uint8_t) color["b"];
        mode = SETCOLOR;
      }

      if (root.containsKey("speed")) {
        uint8_t json_speed = constrain((uint8_t) root["speed"], 0, 255);
        if (json_speed != ws2812fx_speed) {
          ws2812fx_speed = json_speed;
          if(stateOn) mode = SETSPEED;
        }
      }

      if (root.containsKey("color_temp")) {
        //temp comes in as mireds, need to convert to kelvin then to RGB
        color_temp = (uint16_t) root["color_temp"];
        unsigned int kelvin  = 1000000 / color_temp;
        main_color = temp2rgb(kelvin);
        mode = SETCOLOR;
      }

      if (root.containsKey("brightness")) {
        const char * brightness_json = root["brightness"];
        uint8_t b = (uint8_t) strtol((const char *) &brightness_json[0], NULL, 10);
        brightness = constrain(b, 0, 255);
        mode = BRIGHTNESS;
      }

      if (root.containsKey("effect")) {
        animation_on = true;
        String effectString = root["effect"].asString();

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

  #ifdef ENABLE_AMQTT
    void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
    DBG_OUTPUT_PORT.print("MQTT: Recieved ["); DBG_OUTPUT_PORT.print(topic);
//    DBG_OUTPUT_PORT.print("]: "); DBG_OUTPUT_PORT.println(payload_in);
    uint8_t * payload = (uint8_t *) malloc(length + 1);
    memcpy(payload, payload_in, length);
    payload[length] = NULL;
    DBG_OUTPUT_PORT.printf("]: %s\n", payload);
  #endif

  #ifdef ENABLE_MQTT
  void mqtt_callback(char* topic, byte* payload_in, unsigned int length) {
    uint8_t * payload = (uint8_t *)malloc(length + 1);
    memcpy(payload, payload_in, length);
    payload[length] = NULL;
    DBG_OUTPUT_PORT.printf("MQTT: Message arrived [%s]\n", payload);
  #endif
    #ifdef ENABLE_HOMEASSISTANT
      if (strcmp(topic, mqtt_ha_state_in.c_str()) == 0) {
        if (!processJson((char*)payload)) {
          return;
        }
        if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
        #ifdef ENABLE_MQTT
        } else if (strcmp(topic, (char *)mqtt_intopic) == 0) {
        #endif
        #ifdef ENABLE_AMQTT
        } else if (strcmp(topic, mqtt_intopic.c_str()) == 0) {
      #endif
    #endif

      // # ==> Set main color
      if (payload[0] == '#') {
        handleSetMainColor(payload);
        DBG_OUTPUT_PORT.printf("MQTT: Set main color to [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // ? ==> Set speed
      if (payload[0] == '?') {
        uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        ws2812fx_speed = constrain(d, 0, 255);
        strip.setSpeed(convertSpeed(ws2812fx_speed));
        DBG_OUTPUT_PORT.printf("MQTT: Set speed to [%u]\n", ws2812fx_speed);
        #ifdef ENABLE_HOMEASSISTANT
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // % ==> Set brightness
      if (payload[0] == '%') {
        uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        brightness = constrain(b, 0, 255);
        strip.setBrightness(brightness);
        DBG_OUTPUT_PORT.printf("MQTT: Set brightness to [%u]\n", brightness);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // * ==> Set main color and light all LEDs (Shortcut)
      if (payload[0] == '*') {
        handleSetAllMode(payload);
        DBG_OUTPUT_PORT.printf("MQTT: Set main color and light all LEDs [%s]\n", payload);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

      // ! ==> Set single LED in given color
      if (payload[0] == '!') {
        handleSetSingleLED(payload, 1);
        DBG_OUTPUT_PORT.printf("MQTT: Set single LED in given color [%s]\n", payload);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // + ==> Set multiple LED in the given colors
      if (payload[0] == '+') {
        handleSetDifferentColors(payload);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      // R ==> Set range of LEDs in the given colors
      if (payload[0] == 'R') {
        handleRangeDifferentColors(payload);
        DBG_OUTPUT_PORT.printf("MQTT: Set range of LEDS to single color: [%s]\n", payload);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
      }

      #ifdef ENABLE_LEGACY_ANIMATIONS
        // = ==> Activate named mode
        if (payload[0] == '=') {
          String str_mode = String((char *) &payload[0]);
          handleSetNamedMode(str_mode);
          DBG_OUTPUT_PORT.printf("MQTT: Activate named mode [%s]\n", payload);
          #ifdef ENABLE_MQTT
          mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
          #endif
          #ifdef ENABLE_AMQTT
          amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          #endif
          #ifdef ENABLE_HOMEASSISTANT
            if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
          #endif
        }
      #endif

      // $ ==> Get status Info.
      if (payload[0] == '$') {
        DBG_OUTPUT_PORT.printf("MQTT: Get status info.\n");
        DBG_OUTPUT_PORT.println("MQTT: Out: " + String(listStatusJSON()));
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, listStatusJSON());
        #endif
        #ifdef ENABLE_AMQTT
        String liststat = (String) listStatusJSON();
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, liststat.c_str());
        #endif
      }

      // ~ ==> Get WS2812 modes.
      // TODO: Fix this, doesn't return anything. Too long?
      // Hint: https://github.com/knolleary/pubsubclient/issues/110
      if (payload[0] == '~') {
        DBG_OUTPUT_PORT.printf("MQTT: Get WS2812 modes.\n");
        #ifdef ENABLE_MQTT
        DBG_OUTPUT_PORT.printf("Error: Not implemented. Message too large for pubsubclient.");
        mqtt_client.publish(mqtt_outtopic, "ERROR: Not implemented. Message too large for pubsubclient.");
        //String json_modes = listModesJSON();
        //DBG_OUTPUT_PORT.printf(json_modes.c_str());

        //int res = mqtt_client.publish(mqtt_outtopic, json_modes.c_str(), json_modes.length());
        //DBG_OUTPUT_PORT.printf("Result: %d / %d", res, json_modes.length());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("ERROR: Not implemented. Message too large for AsyncMQTT.").c_str());
        #endif
      }

      // / ==> Set WS2812 mode.
      if (payload[0] == '/') {
        handleSetWS2812FXMode(payload);
        DBG_OUTPUT_PORT.printf("MQTT: Set WS2812 mode [%s]\n", payload);
        #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          stateOn = true;
          if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
        #endif
      }

    #ifdef ENABLE_HOMEASSISTANT
    }
    #endif
    free(payload);
  }

  #ifdef ENABLE_MQTT
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
        #ifdef ENABLE_HOMEASSISTANT
          ha_send_data.detach();
          mqtt_client.subscribe(mqtt_ha_state_in.c_str(), qossub);
          #ifdef MQTT_HOME_ASSISTANT_SUPPORT
            DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(strip.getModeCount()) + JSON_OBJECT_SIZE(11));
            JsonObject& json = jsonBuffer.createObject();
            json["name"] = HOSTNAME;
            json["platform"] = "mqtt_json";
            json["state_topic"] = mqtt_ha_state_out;
            json["command_topic"] = mqtt_ha_state_in;
            json["on_command_type"] = "first";
            json["brightness"] = "true";
            json["rgb"] = "true";
            json["optimistic"] = "false";
            json["color_temp"] = "true";
            json["effect"] = "true";
            JsonArray& effect_list = json.createNestedArray("effect_list");
            for (uint8_t i = 0; i < strip.getModeCount(); i++) {
              effect_list.add(strip.getModeName(i));
            }
            char buffer[json.measureLength() + 1];
            json.printTo(buffer, sizeof(buffer));
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
  #ifdef ENABLE_AMQTT
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
      #ifdef ENABLE_HOMEASSISTANT
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
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, message);
      //Subscribe
      uint16_t packetIdSub1 = amqttClient.subscribe((char *)mqtt_intopic.c_str(), qossub);
      DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub1);
      #ifdef ENABLE_HOMEASSISTANT
        ha_send_data.detach();
        uint16_t packetIdSub2 = amqttClient.subscribe((char *)mqtt_ha_state_in.c_str(), qossub);
        DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub2);
        #ifdef MQTT_HOME_ASSISTANT_SUPPORT
          DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(strip.getModeCount()) + JSON_OBJECT_SIZE(11));
          JsonObject& json = jsonBuffer.createObject();
          json["name"] = HOSTNAME;
          json["platform"] = "mqtt_json";
          json["state_topic"] = mqtt_ha_state_out;
          json["command_topic"] = mqtt_ha_state_in;
          json["on_command_type"] = "first";
          json["brightness"] = "true";
          json["rgb"] = "true";
          json["optimistic"] = "false";
          json["color_temp"] = "true";
          json["effect"] = "true";
          JsonArray& effect_list = json.createNestedArray("effect_list");
          for (uint8_t i = 0; i < strip.getModeCount(); i++) {
            effect_list.add(strip.getModeName(i));
          }
          char buffer[json.measureLength() + 1];
          json.printTo(buffer, sizeof(buffer));
          DBG_OUTPUT_PORT.println(buffer);
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
#ifdef ENABLE_BUTTON
  void shortKeyPress() {
    DBG_OUTPUT_PORT.printf("Short button press\n");
    if (buttonState == false) {
      setModeByStateString(BTN_MODE_SHORT);
      buttonState = true;
      #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String("OK =static white").c_str());
      #endif
    } else {
      mode = OFF;
      buttonState = false;
      #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String("OK =off").c_str());
      #endif
      #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =off").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = false;
        if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    }
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    DBG_OUTPUT_PORT.printf("Medium button press\n");
    setModeByStateString(BTN_MODE_MEDIUM);
    #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =fire flicker").c_str());
    #endif
    #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =fire flicker").c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
      if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DBG_OUTPUT_PORT.printf("Long button press\n");
    setModeByStateString(BTN_MODE_LONG);
    #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =fireworks random").c_str());
    #endif
    #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =fireworks random").c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
     stateOn = true;
     if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
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

#ifdef ENABLE_STATE_SAVE_SPIFFS
bool updateFS = false;
#if defined(ENABLE_MQTT) or defined(ENABLE_AMQTT)
// Write configuration to FS JSON
bool writeConfigFS(bool saveConfig){
  if (saveConfig) {
    //FS save
    updateFS = true;
    DBG_OUTPUT_PORT.print("Saving config: ");
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(4));
//    StaticJsonBuffer<JSON_OBJECT_SIZE(4)> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_host"] = mqtt_host;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;
  
//      SPIFFS.remove("/config.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) DBG_OUTPUT_PORT.println("failed to open config file for writing");

    json.printTo(DBG_OUTPUT_PORT);
    json.printTo(configFile);
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
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(4)+300);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(4)+300> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      DBG_OUTPUT_PORT.print("Config: ");
      json.printTo(DBG_OUTPUT_PORT);
      if (json.success()) {
        DBG_OUTPUT_PORT.println(" Parsed!");
        strcpy(mqtt_host, json["mqtt_host"]);
        strcpy(mqtt_port, json["mqtt_port"]);
        strcpy(mqtt_user, json["mqtt_user"]);
        strcpy(mqtt_pass, json["mqtt_pass"]);
        updateFS = false;
        return true;
      } else {
        DBG_OUTPUT_PORT.println("Failed to load json config");
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
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7));
//    StaticJsonBuffer<JSON_OBJECT_SIZE(7)> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["mode"] = static_cast<int>(mode);
  json["strip_mode"] = (int) strip.getMode();
  json["brightness"] = brightness;
  json["speed"] = ws2812fx_speed;
  json["red"] = main_color.red;
  json["green"] = main_color.green;
  json["blue"] = main_color.blue;

//      SPIFFS.remove("/state.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
  File configFile = SPIFFS.open("/stripstate.json", "w");
  if (!configFile) {
    DBG_OUTPUT_PORT.println("Failed!");
    updateFS = false;
    spiffs_save_state.detach();
    updateStateFS = false;
    return false;
  }
  json.printTo(DBG_OUTPUT_PORT);
  json.printTo(configFile);
  configFile.close();
  updateFS = false;
  spiffs_save_state.detach();
  updateStateFS = false;
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
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7)+200);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(7)+200> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(DBG_OUTPUT_PORT);
      if (json.success()) {
        mode = static_cast<MODE>((int) json["mode"]);
        ws2812fx_mode = json["strip_mode"];
        brightness = json["brightness"];
        ws2812fx_speed = json["speed"];
        main_color.red = json["red"];
        main_color.green = json["green"];
        main_color.blue = json["blue"];

        strip.setMode(ws2812fx_mode);
        strip.setSpeed(convertSpeed(ws2812fx_speed));
        strip.setBrightness(brightness);
        strip.setColor(main_color.red, main_color.green, main_color.blue);
        
        updateFS = false;
        return true;
      } else {
        DBG_OUTPUT_PORT.println("Failed to parse JSON!");
      }
    } else {
      DBG_OUTPUT_PORT.println("Failed to open \"/stripstate.json\"");
    }
  } else {
    DBG_OUTPUT_PORT.println("Coudnt find \"/stripstate.json\"");
  }
  //end read
  updateFS = false;
  return false;
}
#endif
