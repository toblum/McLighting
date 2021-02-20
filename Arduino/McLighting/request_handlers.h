// ***************************************************************************
// Request handlers
// ***************************************************************************
#ifdef ENABLE_E131
void checkForRequests(void); //prototype

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

    uint16_t multipacketOffset = (universe - START_UNIVERSE) * 170; //if more than 170 LEDs (510 channels), client will send in next higher universe
    if (NUMLEDS <= multipacketOffset) return;
    uint16_t len = (170 + multipacketOffset > NUMLEDS) ? (NUMLEDS - multipacketOffset) : 170;
    for (uint16_t i = 0; i < len; i++){
      uint16_t j = i * 3;
      strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2]);
    }
    strip->show();
    checkForRequests();
  }
}
#endif

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
    ws2812fx_mode = constrain(server.arg("m").toInt(), 0, strip->getModeCount() - 1);
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
//  strip->setColor(main_color.red, main_color.green, main_color.blue);
  mode = SETCOLOR;
}

void handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

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

  DBG_OUTPUT_PORT.printf("led value: [%i]. Entry threshold: <= [%i] (=> %s)\n", led, strip->numPixels(), mypayload );
  if (led <= strip->numPixels()) {
    char redhex[3];
    char greenhex[3];
    char bluehex[3];
    strncpy (redhex, (const char *) &mypayload[2 + firstChar], 2 );
    strncpy (greenhex, (const char *) &mypayload[4 + firstChar], 2 );
    strncpy (bluehex, (const char *) &mypayload[6 + firstChar], 2 );
    redhex[2] = 0x00;
    greenhex[2] = 0x00;
    bluehex[2] = 0x00;
    ledstates[led].red =   strtol(redhex, NULL, 16);
    ledstates[led].green = strtol(greenhex, NULL, 16);
    ledstates[led].blue =  strtol(bluehex, NULL, 16);
    DBG_OUTPUT_PORT.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s]\n", redhex, greenhex, bluehex);
    DBG_OUTPUT_PORT.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16));
    DBG_OUTPUT_PORT.printf("WS: Set single led [%i] to [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, mypayload);


    strip->setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue);
    strip->show();
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

  strip->setMode(ws2812fx_mode);
  strip->setSpeed(convertSpeed(ws2812fx_speed));
  strip->setBrightness(brightness);
  strip->setColor(main_color.red, main_color.green, main_color.blue);
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

#ifdef ENABLE_E131
  void handleE131NamedMode(String str_mode) {
    exit_func = true;
    if (str_mode.startsWith("=e131") or str_mode.startsWith("/e131")) {
      if(strip->isRunning()) strip->stop();
      mode = E131;
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
    }
  }
#endif

void handleSetWS2812FXMode(uint8_t * mypayload) {
  mode = SET_MODE;
  uint8_t ws2812fx_mode_tmp = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode_tmp, 0, strip->getModeCount() - 1);
}

String listStatusJSON(void) {
  uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip->getMode();
  
  const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(6) + 500;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["mode"] = (uint8_t) mode;
  root["ws2812fx_mode"] = tmp_mode;
  root["ws2812fx_mode_name"] = strip->getModeName(tmp_mode);
  root["speed"] = ws2812fx_speed;
  root["brightness"] = brightness;
  JsonArray color = root.createNestedArray("color");
  color.add(main_color.red);
  color.add(main_color.green);
  color.add(main_color.blue);
  
  String json;
  serializeJson(root, json);
  
  return json;
}

void getStatusJSON() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", listStatusJSON() );
}

String listModesJSON(void) {
  const size_t bufferSize = JSON_ARRAY_SIZE(strip->getModeCount()+1) + strip->getModeCount()*JSON_OBJECT_SIZE(2) + 1000;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonArray json = jsonBuffer.to<JsonArray>();
  #ifdef ENABLE_E131
  JsonObject objecte131 = json.createNestedObject();
  objecte131["mode"] = "e131";
  objecte131["name"] = "E131";
  #endif
  for (uint8_t i = 0; i < strip->getModeCount(); i++) {
    JsonObject object = json.createNestedObject();
    object["mode"] = i;
    object["name"] = strip->getModeName(i);
  }
  JsonObject object = json.createNestedObject();
  
  String json_str;
  serializeJson(json, json_str);
  return json_str;
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

// automatic cycling
Ticker autoTicker;
int autoCount = 0;

void autoTick() {
  strip->setColor(autoParams[autoCount][0]);
  strip->setSpeed(convertSpeed((uint8_t)autoParams[autoCount][1]));
  strip->setMode((uint8_t)autoParams[autoCount][2]);
  autoTicker.once_ms((uint32_t)autoParams[autoCount][3], autoTick);
  DBG_OUTPUT_PORT.printf("autoTick[%d]: {0x%06x, %d, %d, %d}\n", autoCount, autoParams[autoCount][0], (uint8_t)autoParams[autoCount][1], (uint8_t)autoParams[autoCount][2], (uint32_t)autoParams[autoCount][3]);

  autoCount++;
  if (autoCount >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount = 0;
}

void handleAutoStart() {
  autoCount = 0;
  autoTick();
  strip->start();
}

void handleAutoStop() {
  autoTicker.detach();
  strip->stop();
}

void checkpayload(uint8_t * payload, bool mqtt = false, uint8_t num = 0) {
  // # ==> Set main color
  if (payload[0] == '#') {
    handleSetMainColor(payload);
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set main color to: R: [%u] G: [%u] B: [%u]\n",  main_color.red, main_color.green, main_color.blue);
    #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
    mode = SETSPEED;
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set speed to: [%u]\n", ws2812fx_speed);
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
      if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
    #endif
  }

  // % ==> Set brightness
  if (payload[0] == '%') {
    uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
    brightness = constrain(b, 0, 255);
    mode = BRIGHTNESS;
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("WS: Set brightness to: [%u]\n", brightness);
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set main color and light all LEDs [%s]\n", payload);
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set single LED in given color [%s]\n", payload);
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
    #endif
  }

  // + ==> Set multiple LED in the given colors
  if (payload[0] == '+') {
    handleSetDifferentColors(payload);
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set multiple LEDs in given color [%s]\n", payload);
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
    #endif
  }

  // + ==> Set range of LEDs in the given color
  if (payload[0] == 'R') {
    handleRangeDifferentColors(payload);
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set range of LEDs in given color [%s]\n", payload);
    webSocket.sendTXT(num, "OK");
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
    #endif
  }

  #ifdef ENABLE_LEGACY_ANIMATIONS
    // = ==> Activate named mode
    if (payload[0] == '=') {
      // we get mode data
      String str_mode = String((char *) &payload[0]);

      handleSetNamedMode(str_mode);
      #ifdef ENABLE_E131
      handleE131NamedMode(str_mode);
      #endif
      if (mqtt == true)  {
        DBG_OUTPUT_PORT.print("MQTT: "); 
      } else {
        DBG_OUTPUT_PORT.print("WS: ");
        webSocket.sendTXT(num, "OK");
      }
      DBG_OUTPUT_PORT.printf("Activated mode [%u]!\n", mode);
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
    String json = listStatusJSON();
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: ");
      #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, json.c_str());
      #endif
      #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic, qospub, false, json.c_str());
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
      #ifdef ENABLE_MQTT
        // TODO: Fix this, doesn't return anything. Too long?
        // Hint: https://github.com/knolleary/pubsubclient/issues/110
        DBG_OUTPUT_PORT.printf("Error: Not implemented. Message too large for pubsubclient.");
        mqtt_client.publish(mqtt_outtopic, "ERROR: Not implemented. Message too large for pubsubclient.");
        //String json_modes = listModesJSON();
        //DBG_OUTPUT_PORT.printf(json_modes.c_str());
    
        //int res = mqtt_client.publish(mqtt_outtopic, json_modes.c_str(), json_modes.length());
        //DBG_OUTPUT_PORT.printf("Result: %d / %d", res, json_modes.length());
      #endif
      #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic, qospub, false, json.c_str());
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
    #ifdef ENABLE_E131
    String str_mode = String((char *) &payload[0]);
    handleE131NamedMode(str_mode);
    #endif
    if (mqtt == true)  {
      DBG_OUTPUT_PORT.print("MQTT: "); 
    } else {
      DBG_OUTPUT_PORT.print("WS: ");
      webSocket.sendTXT(num, "OK");
    }
    DBG_OUTPUT_PORT.printf("Set WS2812 mode: [%s]\n", payload);
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic, qospub, false, String(String("OK ") + String((char *)payload)).c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
      if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
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
  #ifdef ENABLE_MQTT
  mqtt_client.loop();
  #endif
}


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
      const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 500;
      DynamicJsonDocument jsonBuffer(bufferSize);
      JsonObject root = jsonBuffer.to<JsonObject>();

      root["state"] = (stateOn) ? on_cmd : off_cmd;
      JsonObject color = root.createNestedObject("color");
      color["r"] = main_color.red;
      color["g"] = main_color.green;
      color["b"] = main_color.blue;

      root["brightness"] = brightness;

      root["color_temp"] = color_temp;

      root["speed"] = ws2812fx_speed;

      //char modeName[30];
      //strncpy_P(modeName, (PGM_P)strip->getModeName(strip->getMode()), sizeof(modeName)); // copy from progmem
      #if defined(ENABLE_E131) and defined(ENABLE_HOMEASSISTANT)
      if (mode == E131)
        root["effect"] = "E131";
      else
        root["effect"] = strip->getModeName(strip->getMode());
      #else
        root["effect"] = strip->getModeName(strip->getMode());
      #endif

      char buffer[measureJson(root) + 1];
      serializeJson(root, buffer, sizeof(buffer));

      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_ha_state_out, buffer, true);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out, buffer);
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_ha_state_out, 1, true, buffer);
      DBG_OUTPUT_PORT.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out, buffer);
      #endif
      new_ha_mqtt_msg = false;
      ha_send_data.detach();
      DBG_OUTPUT_PORT.printf("Heap size: %u\n", ESP.getFreeHeap());
    }

    bool processJson(char* message) {
      const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 150;
      DynamicJsonDocument jsonBuffer(bufferSize);
      DeserializationError error = deserializeJson(jsonBuffer, message);
      if (error) {
        DBG_OUTPUT_PORT.print("parseObject() failed: ");
        DBG_OUTPUT_PORT.println(error.c_str());
        return false;
      }
      //DBG_OUTPUT_PORT.println("JSON ParseObject() done!");
      JsonObject root = jsonBuffer.as<JsonObject>();
      
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
        JsonObject color = root["color"];
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
        brightness = constrain((uint8_t) root["brightness"], 0, 255); //fix #224
        mode = BRIGHTNESS;
      }

      if (root.containsKey("effect")) {
        animation_on = true;
        String effectString = root["effect"].as<String>();

        for (uint8_t i = 0; i < strip->getModeCount(); i++) {
          #if defined(ENABLE_E131) and defined(ENABLE_HOMEASSISTANT)
          if(effectString == "E131"){
            if(strip->isRunning()) strip->stop();
            mode = E131;
            break;
          }
          #endif
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
      if (strcmp(topic, mqtt_ha_state_in) == 0) {
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
        } else if (strcmp(topic, mqtt_intopic) == 0) {
      #endif
    #endif

    checkpayload(payload, true);

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
      if (mqtt_client.connect(mqtt_clientid, mqtt_user, mqtt_pass, mqtt_will_topic, 2, true, mqtt_will_payload, true)) {
        DBG_OUTPUT_PORT.println("MQTT connected!");
        // Once connected, publish an announcement...
        char message[18 + strlen(HOSTNAME) + 1];
        strcpy(message, "McLighting ready: ");
        strcat(message, HOSTNAME);
        mqtt_client.publish(mqtt_outtopic, message);
        // ... and resubscribe
        mqtt_client.subscribe(mqtt_intopic, qossub);
        if(mqtt_lwt_boot_flag)
        {
          mqtt_client.publish(mqtt_will_topic, "ONLINE");
          //mqtt_lwt_boot_flag = false;
        }
        #ifdef ENABLE_HOMEASSISTANT
          ha_send_data.detach();
          mqtt_client.subscribe(mqtt_ha_state_in, qossub);
          ha_send_data.once(5, tickerSendState);
          #ifdef MQTT_HOME_ASSISTANT_SUPPORT
            DynamicJsonDocument jsonBuffer(JSON_ARRAY_SIZE(strip->getModeCount()) + JSON_OBJECT_SIZE(12) + 1500);
            JsonObject json = jsonBuffer.to<JsonObject>();
            json["name"] = HOSTNAME;
            #ifdef MQTT_HOME_ASSISTANT_0_87_SUPPORT
            json["schema"] = "json";
            #else
            json["platform"] = "mqtt_json";
            #endif
            json["state_topic"] = mqtt_ha_state_out;
            json["command_topic"] = mqtt_ha_state_in;
            #ifndef MQTT_HOME_ASSISTANT_0_87_SUPPORT
            json["on_command_type"] = "first";
            #endif
            json["brightness"] = "true";
            json["rgb"] = "true";
            json["optimistic"] = "false";
            json["color_temp"] = "true";
            json["effect"] = "true";
            JsonArray effect_list = json.createNestedArray("effect_list");
            for (uint8_t i = 0; i < strip->getModeCount(); i++) {
              effect_list.add(strip->getModeName(i));
            }
            #if defined(ENABLE_E131) and defined(MQTT_HOME_ASSISTANT_SUPPORT)
              effect_list.add("E131");
            #endif
            // Following will never work for PubSubClient as message size > 1.6kB
            // char buffer[measureJson(json) + 1];
            // serializeJson(json, buffer, sizeof(buffer));
            // mqtt_client.publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), buffer, true);

            // Alternate way to publish large messages using PubSubClient
            unsigned int msg_len = measureJson(json) + 1;
            char buffer[msg_len];
            serializeJson(json, buffer, sizeof(buffer));
            DBG_OUTPUT_PORT.println(buffer);
            mqtt_client.beginPublish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), msg_len-1, true);
            mqtt_client.write((const uint8_t*)buffer, msg_len-1);
            mqtt_client.endPublish();
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
      char message[18 + strlen(HOSTNAME) + 1];
      strcpy(message, "McLighting ready: ");
      strcat(message, HOSTNAME);
      amqttClient.publish(mqtt_outtopic, qospub, false, message);
      //Subscribe
      uint16_t packetIdSub1 = amqttClient.subscribe(mqtt_intopic, qossub);
      DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub1);
      if(mqtt_lwt_boot_flag)
      {
        amqttClient.publish(mqtt_will_topic, qospub, false, "ONLINE");
        mqtt_lwt_boot_flag = false;
      }
      #ifdef ENABLE_HOMEASSISTANT
        ha_send_data.detach();
        uint16_t packetIdSub2 = amqttClient.subscribe((char *)mqtt_ha_state_in, qossub);
        DBG_OUTPUT_PORT.printf("Subscribing at QoS %d, packetId: ", qossub); DBG_OUTPUT_PORT.println(packetIdSub2);
        ha_send_data.once(5, tickerSendState);
        #ifdef MQTT_HOME_ASSISTANT_SUPPORT
          DynamicJsonDocument jsonBuffer(JSON_ARRAY_SIZE(strip->getModeCount()) + JSON_OBJECT_SIZE(12) + 1500);
          JsonObject json = jsonBuffer.to<JsonObject>();
          json["name"] = HOSTNAME;
          #ifdef MQTT_HOME_ASSISTANT_0_87_SUPPORT
          json["schema"] = "json";
          #else
          json["platform"] = "mqtt_json";
          #endif
          json["state_topic"] = mqtt_ha_state_out;
          json["command_topic"] = mqtt_ha_state_in;
          #ifndef MQTT_HOME_ASSISTANT_0_87_SUPPORT
          json["on_command_type"] = "first";
          #endif
          json["brightness"] = "true";
          json["rgb"] = "true";
          json["optimistic"] = "false";
          json["color_temp"] = "true";
          json["effect"] = "true";
          JsonArray effect_list = json.createNestedArray("effect_list");
          for (uint8_t i = 0; i < strip->getModeCount(); i++) {
            effect_list.add(strip->getModeName(i));
          }
          #if defined(ENABLE_E131) and defined(MQTT_HOME_ASSISTANT_SUPPORT)
            effect_list.add("E131");
          #endif
          char buffer[measureJson(json) + 1];
          serializeJson(json, buffer, sizeof(buffer));
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
      #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic, qospub, false, String("OK =static white").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
        if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    } else {
      mode = OFF;
      buttonState = false;
      #ifdef ENABLE_MQTT
        mqtt_client.publish(mqtt_outtopic, String("OK =off").c_str());
      #endif
      #ifdef ENABLE_AMQTT
        amqttClient.publish(mqtt_outtopic, qospub, false, String("OK =off").c_str());
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
      amqttClient.publish(mqtt_outtopic, qospub, false, String("OK =fire flicker").c_str());
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
      amqttClient.publish(mqtt_outtopic, qospub, false, String("OK =fireworks random").c_str());
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
    DynamicJsonDocument jsonBuffer(200);
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
      DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4)+300);
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
        return true;
      } else {
        DBG_OUTPUT_PORT.print("Failed to load json config: ");
        DBG_OUTPUT_PORT.println(error.c_str());
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
  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(7)+200);
  JsonObject json = jsonBuffer.to<JsonObject>();
  json["mode"] = static_cast<int>(mode);
  json["strip_mode"] = (int) strip->getMode();
  json["brightness"] = brightness;
  json["speed"] = ws2812fx_speed;
  json["red"] = main_color.red;
  json["green"] = main_color.green;
  json["blue"] = main_color.blue;

  //SPIFFS.remove("/stripstate.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
  File configFile = SPIFFS.open("/stripstate.json", "w");
  if (!configFile) {
    DBG_OUTPUT_PORT.println("Failed!");
    updateFS = false;
    spiffs_save_state.detach();
    updateStateFS = false;
    return false;
  }
  serializeJson(json, DBG_OUTPUT_PORT);
  serializeJson(json, configFile);
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
      DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(7)+200);
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      if (!error) {
        JsonObject json = jsonBuffer.as<JsonObject>();
        serializeJson(json, DBG_OUTPUT_PORT);
        mode = static_cast<MODE>(json["mode"].as<int>());
        ws2812fx_mode = json["strip_mode"].as<int>();
        brightness = json["brightness"].as<int>();
        ws2812fx_speed = json["speed"].as<int>();
        main_color.red = json["red"].as<int>();
        main_color.green = json["green"].as<int>();
        main_color.blue = json["blue"].as<int>();

        #ifdef ENABLE_HOMEASSISTANT
          if(mode != OFF) stateOn = true;
        #endif
        strip->setMode(ws2812fx_mode);
        strip->setSpeed(convertSpeed(ws2812fx_speed));
        strip->setBrightness(brightness);
        strip->setColor(main_color.red, main_color.green, main_color.blue);

        #ifdef ENABLE_E131
        if (mode == E131) {
          strip->stop();
        }
        #endif
        
        updateFS = false;
        return true;
      } else {
        DBG_OUTPUT_PORT.println("Failed to parse JSON!");
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

//Strip Config
char strip_size[4], led_pin[3]; //needed for WiFiManager Settings

struct
{
  uint16_t stripSize = NUMLEDS;
  uint8_t RGBOrder = NEO_GRB;
  #if defined(USE_WS2812FX_DMA) or defined(USE_WS2812FX_UART1) or defined(USE_WS2812FX_UART2)
    #ifdef USE_WS2812FX_DMA
      uint8_t pin = 3;
    #endif
    #ifdef USE_WS2812FX_UART1
      uint8_t pin = 2;
    #endif
    #ifdef USE_WS2812FX_UART2
      uint8_t pin = 1;
    #endif
  #else
    uint8_t pin = LED_PIN;
  #endif
} WS2812FXStripSettings;

Ticker saveWS2812FXStripSettings;

bool readStripConfigFS(void) {
  //read stripconfiguration from FS JSON
  updateFS = true;
  if (SPIFFS.exists("/neoconfig.json")) {
    //file exists, reading and loading
    DBG_OUTPUT_PORT.print("Reading neoconfig file... ");
    File configFile = SPIFFS.open("/neoconfig.json", "r");
    if (configFile) {
      DBG_OUTPUT_PORT.println("Opened!");
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4)+300);
      DeserializationError error = deserializeJson(jsonBuffer, buf.get());
      DBG_OUTPUT_PORT.print("neoconfig.json: ");
      if (!error) {
        DBG_OUTPUT_PORT.println("Parsed!");
        JsonObject json = jsonBuffer.as<JsonObject>();
        serializeJson(json, DBG_OUTPUT_PORT);
        WS2812FXStripSettings.stripSize = (json["pixel_count"].as<uint16_t>()) ? json["pixel_count"].as<uint16_t>() : NUMLEDS;
        WS2812FXStripSettings.RGBOrder = json["rgb_order"].as<int>();
        WS2812FXStripSettings.pin = json["pin"].as<int>();
        updateFS = false;
        return true;
      } else {
        DBG_OUTPUT_PORT.print("Failed to load json config: ");
        DBG_OUTPUT_PORT.println(error.c_str());
      }
    } else {
      DBG_OUTPUT_PORT.println("Failed to open /config.json");
    }
  } else {
    DBG_OUTPUT_PORT.println("Couldn't find config.json");
  }
  //end read
  updateFS = false;
  return false;
}

void writeStripConfigFS(void){
  updateFS = true;
  //save the strip config to FS JSON
  DBG_OUTPUT_PORT.print("Saving Strip cfg: ");
  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(4)+300);
  JsonObject json = jsonBuffer.to<JsonObject>();
  json["pixel_count"] = WS2812FXStripSettings.stripSize;
  json["rgb_order"] = WS2812FXStripSettings.RGBOrder;
  json["pin"] = WS2812FXStripSettings.pin;

  //SPIFFS.remove("/neoconfig.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
  File configFile = SPIFFS.open("/neoconfig.json", "w");
  if (!configFile) {
    DBG_OUTPUT_PORT.println("Failed!");
    updateFS = false;
  }
  serializeJson(json, DBG_OUTPUT_PORT);
  serializeJson(json, configFile);
  DBG_OUTPUT_PORT.println();
  configFile.close();
  updateFS = false;
  //end save
}
