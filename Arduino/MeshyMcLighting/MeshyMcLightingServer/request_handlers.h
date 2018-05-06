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

String statusLEDs(bool haflag = false) {
  size_t bufferSize;
  if(haflag)
    bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6);
  else
    bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5);
  //StaticJsonBuffer<bufferSize> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  
  root["state"] = (stateOn) ? on_cmd : off_cmd;
  JsonObject& color = root.createNestedObject("color");
  color["r"] = main_color.red;
  color["g"] = main_color.green;
  color["b"] = main_color.blue;
  
  root["brightness"] = brightness;

  if(haflag) root["color_temp"] = color_temp;
  
  root["speed"] = ws2812fx_speed;
  
  char modeName[30];
  strncpy_P(modeName, (PGM_P)strip.getModeName(strip.getMode()), sizeof(modeName)); // copy from progmem
  root["effect"] = modeName;
  
  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  return String(buffer);
}

bool processJson(String &message) {
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 150;
  //StaticJsonBuffer<bufferSize> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(message);
  
  if (!root.success()) {
    DEBUG_PRINTLN("parseObject() failed");
    return false;
  }
  //DEBUG_PRINTLN("JSON ParseObject() done!");
  
  if (root.containsKey("state")) {
    const char* state_in = root["state"];
    if (strcmp(state_in, on_cmd) == 0) {
      stateOn = true;
      mode = SET_MODE;
    }
    else if (strcmp(state_in, off_cmd) == 0) {
      stateOn = false;
      animation_on = false;
      mode = OFF;
    }
  }
  
  if (root.containsKey("color")) {
    JsonObject& color = root["color"];
    main_color.red = (uint8_t) color["r"];
    main_color.green = (uint8_t) color["g"];
    main_color.blue = (uint8_t) color["b"];
  }
  
  if (root.containsKey("speed")) {
    int json_speed = constrain((int) root["speed"], SPEED_MIN, SPEED_MAX);
    if (json_speed != ws2812fx_speed) {
      ws2812fx_speed = json_speed;
    }
  }
  
  if (root.containsKey("brightness")) {
    const char * brightness_json = root["brightness"];
    uint8_t b = (uint8_t) strtol((const char *) &brightness_json[0], NULL, 10);
    brightness = constrain(b, 0, 255);
  }
  
  if (root.containsKey("effect")) {
    animation_on = true;
    String effectString = root["effect"].asString();
  
    for (uint8_t i = 0; i < strip.getModeCount(); i++) {
      if(String(strip.getModeName(i)) == effectString) {
        ws2812fx_mode = i;
        break;
      }
    }
  }
  DEBUG_PRINTF8("{\"mode\":%d, \"ws2812fx_mode\":%d, \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}\n", mode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  jsonBuffer.clear();
  return true;
}

#ifdef ENABLE_HOMEASSISTANT
bool processHAJson(char* message) {
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 150;
  //StaticJsonBuffer<bufferSize> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(message);

  if (!root.success()) {
    DEBUG_PRINTLN("parseObject() failed");
    return false;
  }
  //DEBUG_PRINTLN("JSON ParseObject() done!");

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
  DEBUG_PRINTF8("{\"mode\":%d, \"ws2812fx_mode\":%d, \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}", mode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  jsonBuffer.clear();
  return true;
}
#endif
  
uint16_t convertSpeed(uint8_t mcl_speed) {
  //long ws2812_speed = mcl_speed * 256;
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = constrain(SPEED_MAX - ws2812_speed, SPEED_MIN, SPEED_MAX);
  return ws2812_speed;
}

void handleSetMainColor(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);
  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);
  mode = SETCOLOR;
}

void handleSetAllMode(uint8_t * mypayload) {
  // decode rgb data
  uint32_t rgb = (uint32_t) strtol((const char *) &mypayload[1], NULL, 16);

  main_color.red = ((rgb >> 16) & 0xFF);
  main_color.green = ((rgb >> 8) & 0xFF);
  main_color.blue = ((rgb >> 0) & 0xFF);

  DEBUG_PRINTF4("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  ws2812fx_mode = FX_MODE_STATIC;
  mode = SET_MODE;
}

void handleSetSingleLED(uint8_t * mypayload, uint8_t firstChar = 0) {
  // decode led index
  char templed[3];
  strncpy (templed, (const char *) &mypayload[firstChar], 2 );
  uint8_t led = atoi(templed);

  DEBUG_PRINTF4("led value: [%i]. Entry threshold: <= [%i] (=> %s)\n", led, strip.numPixels(), mypayload );
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
    DEBUG_PRINTF4("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s]\n", redhex, greenhex, bluehex);
    DEBUG_PRINTF4("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16));
    DEBUG_PRINTF6("WS: Set single led [%i] to [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, mypayload);


    strip.setPixelColor(led, ledstates[led].red, ledstates[led].green, ledstates[led].blue);
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
    char colorval[7] = { 0, 0, 0, 0, 0, 0, 0 };
    strncpy ( startled, (const char *) &nextCommand[0], 2 );
    strncpy ( endled, (const char *) &nextCommand[2], 2 );
    strncpy ( colorval, (const char *) &nextCommand[4], 6 );
    int rangebegin = atoi(startled);
    int rangeend = atoi(endled);
    DEBUG_PRINTF4("Setting RANGE from [%i] to [%i] as color [%s] \n", rangebegin, rangeend, colorval);

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

void handleSetWS2812FXMode(uint8_t * mypayload) {
  mode = SET_MODE;
  uint8_t ws2812fx_mode_tmp = (uint8_t) strtol((const char *) &mypayload[1], NULL, 10);
  ws2812fx_mode = constrain(ws2812fx_mode_tmp, 0, strip.getModeCount() - 1);
}

char* listStatusJSON(void) {
  char json[255];
  char modeName[30];
  uint8_t tmp_mode = (mode == SET_MODE) ? (uint8_t) ws2812fx_mode : strip.getMode();
  
  strncpy_P(modeName, (PGM_P)strip.getModeName(tmp_mode), sizeof(modeName)); // copy from progmem
  
  //snprintf(json, sizeof(json), "{\"mode\":%d, \"ws2812fx_mode\":%d, \"ws2812fx_mode_name\":\"%s\", \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}",
  //         mode, tmp_mode, modeName, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
    
  const size_t bufferSize = JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  root["mode"] = (uint8_t) mode;
  root["ws2812fx_mode"] = tmp_mode;
  root["ws2812fx_mode_name"] = modeName;
  root["speed"] = ws2812fx_speed;
  root["brightness"] = brightness;
  JsonArray& color = root.createNestedArray("color");
  color.add(main_color.red);
  color.add(main_color.green);
  color.add(main_color.blue);
  
  root.printTo(json, root.measureLength() + 1);

  return json;
}

String listModesJSON(void) {
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

//  const size_t bufferSize = JSON_ARRAY_SIZE(strip.getModeCount()) + strip.getModeCount()*JSON_OBJECT_SIZE(2);
//  DynamicJsonBuffer jsonBuffer(bufferSize);
//  JsonArray& json = jsonBuffer.createArray();
//  JsonObject&<strip.getModeCount()> object[strip.getModeCount()];
//  for (uint8_t i = 0; i < strip.getModeCount(); i++) {
//    object[i].add("mode", i);
//    object[i].add("name", strip.getModeName(i));
//    json.add(object[i]);
//  }
//  char json[root.measureLength() + 1];
//  root.printTo(json, sizeof(json));
//
//  return String(json);
}

void modes_write_to_spiffs_mclighting(bool overWrite = false) {
  if( !SPIFFS.exists("/modes_mclighting") or overWrite) {
    DEBUG_PRINT("Writing Modes to SPIFFs ");
    String modes = listModesJSON();
    File modeFile = SPIFFS.open("/modes_mclighting", "w");
    if (!modeFile) DEBUG_PRINTLN("failed to open mode file for writing");
    modeFile.print(modes);
    modeFile.close();
    modes=String();
    DEBUG_PRINTLN(" done!");
  } else {
    DEBUG_PRINTLN("'/modes_mclighting' already exists on SPIFFs, not overwriting");
  }
}

// automatic cycling

void handleAutoStart(void) {
  autoCount = 0;
  autoModeTask.enableIfNot();
}

void handleAutoStop(void) {
  autoModeTask.disable();
  stateOn = false;
  taskSendMessage.enableIfNot();
  taskSendMessage.forceNextIteration();
  mode = OFF;
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *payload, size_t len){
  if(type == WS_EVT_CONNECT){
    DEBUG_PRINTF3("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    DEBUG_PRINTF3("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    DEBUG_PRINTF5("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)payload);
  } else if(type == WS_EVT_PONG){
    DEBUG_PRINTF5("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)payload:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;

    if(info->opcode == WS_TEXT) {
      DEBUG_PRINTF5("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), "text", info->len);
      
      if(payload[0] == '#') { // # ==> Set main color
        stateOn = true;
        handleSetMainColor(payload);
        DEBUG_PRINTF4("Set main color to: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
         #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '?') { // ? ==> Set speed
        
        uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        ws2812fx_speed = convertSpeed(constrain(d, 0, 255));
        DEBUG_PRINTF("WS: Set speed to: [%u]\n", ws2812fx_speed);
        mode = SETSPEED;
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '%') { // % ==> Set brightness
      
        uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
        brightness = ((b >> 0) & 0xFF);
        DEBUG_PRINTF("WS: Set brightness to: [%u]\n", brightness);
        mode = BRIGHTNESS;
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '*') { // * ==> Set main color and light all LEDs (Shortcut)
        
        handleSetAllMode(payload);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '!') { // ! ==> Set single LED in given color
        
        handleSetSingleLED(payload, 1);
        client->text("OK");

        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
              
      } else if (payload[0] == '+') { // + ==> Set multiple LED in the given colors
        
        handleSetDifferentColors(payload);
        client->text("OK");

        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
                
      } else if (payload[0] == 'R') { // R ==> Set range of LEDs in the given color
      
        handleRangeDifferentColors(payload);
        client->text("OK");

        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        
      } else if (payload[0] == '$') { // $ ==> Get status Info.
      
        DEBUG_PRINT("Get status info.");
        String json = listStatusJSON();
        DEBUG_PRINTLN(json);
        client->text(json);
        
      } else if (payload[0] == '~') { // ~ ==> Get WS2812 modes.
      
        DEBUG_PRINT("Get WS2812 modes.");
        String json = listModesJSON();
        DEBUG_PRINTLN(json);
        client->text(json);
        
      } else if (payload[0] == '/') { // / ==> Set WS2812 mode.
      
        handleSetWS2812FXMode(payload);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          if(mqttClient.connected()) mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
        
      } else if (strcmp((char *)payload, "start") == 0 ) { // start auto cycling
        handleAutoStart();
        client->text("OK");
        
      } else if (strcmp((char *)payload, "stop") == 0 ) { // stop auto cycling
        
        handleAutoStop();
        client->text("OK");
        
      }
    }
  }
}

#ifdef ENABLE_AMQTT
  void connectToMqtt(void) {
    DEBUG_PRINTLN(getlocalIP());
    DEBUG_PRINT("ConnectToMqtt() called: ");
    if(getlocalIP() != IPAddress(0,0,0,0)){
      DEBUG_PRINTLN("Connecting ... ");
      mqttClient.connect();
    } else {
      DEBUG_PRINTLN("WiFi disconnected!");
      taskConnecttMqtt.disable();
    }
  }
  
  void onMqttConnect(bool sessionPresent) {
    taskConnecttMqtt.disable();
    DEBUG_PRINT("MQTT connected!\n");
    DEBUG_PRINT("Session present: ");
    DEBUG_PRINTLN(sessionPresent);
    char * message = new char[18 + strlen(HOSTNAME) + 1];
    strcpy(message, "McLighting ready: ");
    strcat(message, HOSTNAME);
    mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, message);
    //Subscribe
    uint16_t packetIdSub1 = mqttClient.subscribe((char *)mqtt_intopic.c_str(), qossub);
    DEBUG_PRINTF("Subscribing at QoS %d, packetId: ", qossub); DEBUG_PRINTLN(packetIdSub1);
    #ifdef ENABLE_HOMEASSISTANT
      if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
      uint16_t packetIdSub2 = mqttClient.subscribe((char *)mqtt_ha_state_in.c_str(), qossub);
      DEBUG_PRINTF("Subscribing at QoS %d, packetId: ", qossub); DEBUG_PRINTLN(packetIdSub2);
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
        DEBUG_PRINTLN(buffer);
        mqttClient.publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), qospub, true, buffer);
      #endif
    #endif
  }
  
  
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    DEBUG_PRINT("Disconnected from MQTT, reason: ");
    if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
      DEBUG_PRINTLN("Bad server fingerprint.");
    } else if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
      DEBUG_PRINTLN("TCP Disconnected.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
      DEBUG_PRINTLN("Bad server fingerprint.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
      DEBUG_PRINTLN("MQTT Identifier rejected.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
      DEBUG_PRINTLN("MQTT server unavailable.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
      DEBUG_PRINTLN("MQTT malformed credentials.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
      DEBUG_PRINTLN("MQTT not authorized.");
    } else if (reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE) {
      DEBUG_PRINTLN("Not enough space on esp8266.");
    }
    
    if (getlocalIP() != IPAddress(0,0,0,0)) {
      DEBUG_PRINTLN("WiFi is connected: Calling ConnectToMqtt() Task");
      taskConnecttMqtt.enableIfNot();
      //taskConnecttMqtt.enable();
    } else {
      DEBUG_PRINTLN("WiFi Disconnected!");
      taskConnecttMqtt.disable();
    }
  }
  
  void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
      DEBUG_PRINT("MQTT: Recieved ["); DEBUG_PRINT(topic);
      uint8_t * payload = (uint8_t *) malloc(length + 1);
      memcpy(payload, payload_in, length);
      payload[length] = NULL;
      DEBUG_PRINTF("]: %s\n", payload);
      #ifdef ENABLE_HOMEASSISTANT
        if (strcmp(topic, mqtt_ha_state_in.c_str()) == 0) {
          if (!processHAJson((char*) payload)) {
            return;
          }
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
          #endif
          } else if (strcmp(topic, mqtt_intopic.c_str()) == 0) {
      #endif
        // # ==> Set main color
        if (payload[0] == '#') {
          
          stateOn = true;
          handleSetMainColor(payload);
          DEBUG_PRINTF4("MQTT: Set main color to [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          #ifdef ENABLE_HOMEASSISTANT
            if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
          #endif
        } else if (payload[0] == '?') { // ? ==> Set speed
          
          uint8_t d = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
          ws2812fx_speed = convertSpeed(constrain(d, 0, 255));
          mode = SETSPEED;        
          DEBUG_PRINTF("MQTT: Set speed to [%u]\n", ws2812fx_speed);
          #ifdef ENABLE_HOMEASSISTANT
            if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #endif
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          
        } else if (payload[0] == '%') { // % ==> Set brightness
          
          stateOn = true;
          uint8_t b = (uint8_t) strtol((const char *) &payload[1], NULL, 10);
          brightness = constrain(b, 0, 255);
          mode = BRIGHTNESS;
          DEBUG_PRINTF("MQTT: Set brightness to [%u]\n", brightness);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          #ifdef ENABLE_HOMEASSISTANT
            if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
          #endif
          
        } else if (payload[0] == '*') { // * ==> Set main color and light all LEDs (Shortcut)
          
          stateOn = true;
          handleSetAllMode(payload);
          DEBUG_PRINTF("MQTT: Set main color and light all LEDs [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          #ifdef ENABLE_HOMEASSISTANT
            if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
          #endif
          
        } else if (payload[0] == '!') { // ! ==> Set single LED in given color
          
          handleSetSingleLED(payload, 1);
          DEBUG_PRINTF("MQTT: Set single LED in given color [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == '+') { // + ==> Set multiple LED in the given colors
          
          handleSetDifferentColors(payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == 'R') { // R ==> Set range of LEDs in the given colors
          
          handleRangeDifferentColors(payload);
          DEBUG_PRINTF("MQTT: Set range of LEDS to single color: [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == '$') { // $ ==> Get status Info.
  
          String json = String(listStatusJSON());
          DEBUG_PRINT("MQTT: Get status info.\n");
          DEBUG_PRINTLN("MQTT: Out: " + json);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, json.c_str());
          
        } else if (payload[0] == '~') { // ~ ==> Get WS2812 modes.
          
          DEBUG_PRINT("MQTT: Get WS2812 modes.\n");
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, listModesJSON().c_str());
          
        } else if (payload[0] == '/') { // / ==> Set WS2812 mode.
          
          stateOn = true;
          handleSetWS2812FXMode(payload);
          DEBUG_PRINTF("MQTT: Set WS2812 mode [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
          #ifdef ENABLE_HOMEASSISTANT
            if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
          #endif
          #ifdef ENABLE_STATE_SAVE_SPIFFS
            if(!taskSpiffsSaveState.isEnabled()) if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
          #endif
        }
        
  
      #ifdef ENABLE_HOMEASSISTANT
      }
      #endif
      free(payload);
  }
#endif

#ifdef ENABLE_STATE_SAVE_SPIFFS
bool writeStateFS(){
  updateFS = true;
  //save the strip state to FS JSON
  DEBUG_PRINT("Saving cfg: ");
  DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(8));
//    StaticJsonBuffer<JSON_OBJECT_SIZE(7)> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["stateOn"] = (stateOn) ? 1 : 0;
  json["mode"] = static_cast<int>(mode);
  json["strip_mode"] = (int) strip.getMode();
  json["brightness"] = brightness;
  json["speed"] = ws2812fx_speed;
  json["red"] = main_color.red;
  json["green"] = main_color.green;
  json["blue"] = main_color.blue;

//      SPIFFS.remove("/state.json") ? DEBUG_PRINTLN("removed file") : DEBUG_PRINTLN("failed removing file");
  File configFile = SPIFFS.open("/stripstate.json", "w");
  if (!configFile) {
    DEBUG_PRINTLN("Failed!");
    updateFS = false;
    return false;
  }
  #ifdef SERIALDEBUG
    json.printTo(Serial);
  #endif
  json.printTo(configFile);
  configFile.close();
  updateFS = false;
  return true;
  //end save
}

bool readStateFS() {
  //read strip state from FS JSON
  updateFS = true;
  //if (resetsettings) { SPIFFS.begin(); SPIFFS.remove("/config.json"); SPIFFS.format(); delay(1000);}
  if (SPIFFS.exists("/stripstate.json")) {
    //file exists, reading and loading
    DEBUG_PRINT("Read cfg: ");
    File configFile = SPIFFS.open("/stripstate.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(8)+200);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(7)+200> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      #ifdef SERIALDEBUG
        json.printTo(Serial);
      #endif
      if (json.success()) {
        stateOn = json["stateOn"];
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
        DEBUG_PRINTLN("Failed to parse JSON!");
      }
    } else {
      DEBUG_PRINTLN("Failed to open \"/stripstate.json\"");
    }
  } else {
    DEBUG_PRINTLN("Coudnt find \"/stripstate.json\"");
  }
  //end read
  updateFS = false;
  return false;
}

bool writeConfigFS(bool saveConfig){
  if (saveConfig) {
    //FS save
    updateFS = true;
    DEBUG_PRINT("Saving config: ");
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7));
    //StaticJsonBuffer<JSON_OBJECT_SIZE(7)> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    #ifdef ENABLE_AMQTT
      json["mqtt_host"] = mqtt_host;
      json["mqtt_port"] = mqtt_port;
      json["mqtt_user"] = mqtt_user;
      json["mqtt_pass"] = mqtt_pass;
    #endif
    json["wifi_ssid"] = wifi_ssid;
    json["wifi_pwd"]  = wifi_pwd;
    json["wifi_channel"] = wifi_channel;
  
    //SPIFFS.remove("/config.json") ? DEBUG_PRINTLN("removed file") : DEBUG_PRINTLN("failed removing file");
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) DEBUG_PRINTLN("failed to open config file for writing");

    #ifdef SERIALDEBUG
      json.printTo(Serial);
    #endif
    json.printTo(configFile);
    configFile.close();
    updateFS = false;
    return true;
    //end save
  } else {
    DEBUG_PRINTLN("SaveConfig is False!");
    return false;
  }
}

// Read search_str to FS
bool readConfigFS() {
  //read configuration from FS JSON
  updateFS = true;
  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    DEBUG_PRINT("Reading config file... ");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      DEBUG_PRINTLN("Opened!");
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7)+600);
      //StaticJsonBuffer<JSON_OBJECT_SIZE(7)+600> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      DEBUG_PRINT("Config: ");
      #ifdef SERIALDEBUG
        json.printTo(Serial);
      #endif
      if (json.success()) {
        DEBUG_PRINTLN(" Parsed!");
        #ifdef ENABLE_AMQTT
          strcpy(mqtt_host, json["mqtt_host"]);
          strcpy(mqtt_port, json["mqtt_port"]);
          strcpy(mqtt_user, json["mqtt_user"]);
          strcpy(mqtt_pass, json["mqtt_pass"]);
        #endif
        wifi_ssid = json["wifi_ssid"].as<String>();
        wifi_pwd  = json["wifi_pwd"].as<String>();
        wifi_channel = atoi(json["wifi_channel"]);
        updateFS = false;
        return true;
      } else {
        DEBUG_PRINTLN("Failed to load json config");
      }
    } else {
      DEBUG_PRINTLN("Failed to open /config.json");
    }
  } else {
    DEBUG_PRINTLN("Coudnt find config.json");
  }
  //end read
  updateFS = false;
  return false;
}
#endif

#ifdef ENABLE_AMQTT
void async_mqtt_setup(void){ 
  if (mqtt_host != "" && atoi(mqtt_port) > 0) {
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.setServer(mqtt_host, atoi(mqtt_port));
    if(mqtt_user != "" or mqtt_pass != "") mqttClient.setCredentials(mqtt_user, mqtt_pass);
    mqttClient.setClientId(mqtt_clientid);
  }
}
#endif

#ifdef ENABLE_HOMEASSISTANT
void fnSendHAState(void){
  String json = statusLEDs(true);
  if(mqttClient.connected()) {
    mqttClient.publish(mqtt_ha_state_out.c_str(), 1, true, json.c_str());
    DEBUG_PRINTF3("MQTT: Send [%s]: %s\n", mqtt_ha_state_out.c_str(), json.c_str());
  } else {
    DEBUG_PRINTLN("MQTT is disonnected, not sending HA status.");
  }
  taskSendHAState.disable();
}
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
void fnSpiffsSaveState(void){
  DEBUG_PRINTLN((writeStateFS()) ? " Success!" : " Failure!");
  taskSpiffsSaveState.disable();
}
#endif

void modes_write_to_spiffs(bool overWrite = false) {
  if( !SPIFFS.exists("/modes") or overWrite) {
    DEBUG_PRINT("Writing Modes to SPIFFs ");
    String modes = "<li><a href='#' class='m' id='0'>OFF</a></li>";
    for(uint8_t i=1; i < strip.getModeCount() + 1; i++) {
      modes += "<li><a href='#' class='m' id='";
      modes += (i);
      modes += "'>";
      modes += strip.getModeName(i-1);
      modes += "</a></li>";
      DEBUG_PRINT(".");
    }
  
    File modeFile = SPIFFS.open("/modes", "w");
    if (!modeFile) DEBUG_PRINTLN("failed to open mode file for writing");
    modeFile.print(modes);
    modeFile.close();
    modes=String();
    DEBUG_PRINTLN(" done!");
  } else {
    DEBUG_PRINTLN("'/modes' already exists on SPIFFs, not overwriting");
  }
}

void autoTick(void) {
  main_color.red =   ((autoParams[autoCount][0] >> 16) & 0xFF);
  main_color.green = ((autoParams[autoCount][0] >>  8) & 0xFF);
  main_color.blue =  ((autoParams[autoCount][0] >>  0) & 0xFF);
  ws2812fx_speed = convertSpeed((uint8_t) autoParams[autoCount][1]);
  ws2812fx_mode = (uint8_t) autoParams[autoCount][2];

  autoModeTask.delay(((int) autoParams[autoCount][3]) * TASK_SECOND);
  
  DEBUG_PRINTF("autoTick %d\n", autoCount);
  taskSendMessage.enableIfNot();
  taskSendMessage.forceNextIteration();
  #ifdef ENABLE_HOMEASSISTANT
    taskSendHAState.enableIfNot();
    taskSendHAState.forceNextIteration();
  #endif
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    taskSpiffsSaveState.enableIfNot();
    taskSpiffsSaveState.forceNextIteration();
  #endif
  //autoCount++;
  autoCount = (autoCount  + 1) % (sizeof(autoParams) / sizeof(autoParams[0]));
  
  mode = SET_MODE;
}

/////// Button Mode ///////////////
#ifdef ENABLE_BUTTON
  String getValue(String &data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length()-1;
  
    for(int i=0; i<=maxIndex && found<=index; i++){
      if(data.charAt(i)==separator || i==maxIndex){
          found++;
          strIndex[0] = strIndex[1]+1;
          strIndex[1] = (i == maxIndex) ? i+1 : i;
      }
    }
  
    return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
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
  
    DEBUG_PRINTF("ws2812fx_mode: %d\n", ws2812fx_mode);
    DEBUG_PRINTF("ws2812fx_speed: %d\n", ws2812fx_speed);
    DEBUG_PRINTF("brightness: %d\n", brightness);
    DEBUG_PRINTF("main_color.red: %d\n", main_color.red);
    DEBUG_PRINTF("main_color.green: %d\n", main_color.green);
    DEBUG_PRINTF("main_color.blue: %d\n", main_color.blue);
  
    mode = SET_MODE;
  }

  void shortKeyPress() {
    DEBUG_PRINT("Short button press\n");
    if (buttonState == false) {
      setModeByStateString(BTN_MODE_SHORT);
      buttonState = true;
      #ifdef ENABLE_AMQTT
        mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =static white").c_str());
      #endif
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_HOMEASSISTANT
        if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
    } else {
      mode = OFF;
      buttonState = false;
      stateOn = false;
      #ifdef ENABLE_AMQTT
        mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =off").c_str());
      #endif
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_HOMEASSISTANT
        if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
    }
  }

  // called when button is kept pressed for less than 2 seconds
  void mediumKeyPress() {
    DEBUG_PRINT("Medium button press\n");
    setModeByStateString(BTN_MODE_MEDIUM);
    stateOn = true;
    #ifdef ENABLE_AMQTT
      mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =fire flicker").c_str());
    #endif
    if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
    #ifdef ENABLE_HOMEASSISTANT
      if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
    #endif
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DEBUG_PRINT("Long button press\n");
    setModeByStateString(BTN_MODE_LONG);
    stateOn = true;
    #ifdef ENABLE_AMQTT
      mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =fireworks random").c_str());
    #endif
    if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
    #ifdef ENABLE_HOMEASSISTANT
      if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
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

///////// Mesh Stuff ///////////

IPAddress getlocalIP() {
  //return IPAddress(mesh.getStationIP()); //develop
  return IPAddress(mesh.getStationIP().addr);  //master
}

void sendMessage() {
  String msg = statusLEDs();
  mesh.sendBroadcast(msg);
  taskSendMessage.disable();
  DEBUG_PRINT("Broadcasting: ");
  DEBUG_PRINTLN(msg);
}

void receivedCallback(uint32_t from, String & msg) {
  DEBUG_PRINTF3(">>>>>>>>>> Received from %u msg=%s\n", from, msg.c_str());
  if(!processJson(msg)) return;
  else DEBUG_PRINTF("Processed incoming message from %u successfully!\n", from);
  #ifdef ENABLE_HOMEASSISTANT
    if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
  #endif
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
}

void newConnectionCallback(uint32_t nodeId) {
  String msg = statusLEDs();
  mesh.sendSingle(nodeId, msg);
  DEBUG_PRINTF("$$$$$$$$$$$ New Connection, nodeId = %u >>> Sending: ", nodeId);
  DEBUG_PRINTLN(msg);
}

void changedConnectionCallback() {
  DEBUG_PRINTF("~~~~~~~~~~~ Changed connections %s\n", mesh.subConnectionJson().c_str());
  nodes = mesh.getNodeList();
  DEBUG_PRINTF("Num nodes: %d\n", nodes.size());
  DEBUG_PRINT("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    DEBUG_PRINTF(" %u", *node);
    node++;
  }
  DEBUG_PRINTLN();

  //no need to re-broadcast data, taken care of in newConnectionCallback()
  if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
}
