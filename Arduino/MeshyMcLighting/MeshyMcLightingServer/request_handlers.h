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

String statusLEDs(void) {
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

  return String(buffer);
}

bool processJson(String message) {
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + 150;
  //StaticJsonBuffer<bufferSize> jsonBuffer;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.parseObject(message);
  
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return false;
  }
  //Serial.println("JSON ParseObject() done!");
  
  if (root.containsKey("state")) {
    const char* state_in = root["state"];
    if (strcmp(state_in, on_cmd) == 0 and !(animation_on)) {
      stateOn = true;
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
  }
  
  if (root.containsKey("speed")) {
    uint8_t json_speed = constrain((uint8_t) root["speed"], 0, 255);
    if (json_speed != ws2812fx_speed) {
      ws2812fx_speed = json_speed;
    }
  }
  
  if (root.containsKey("color_temp")) {
    //temp comes in as mireds, need to convert to kelvin then to RGB
    color_temp = (uint16_t) root["color_temp"];
    unsigned int kelvin  = 1000000 / color_temp;
    main_color = temp2rgb(kelvin);
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
    Serial.println("parseObject() failed");
    return false;
  }
  //Serial.println("JSON ParseObject() done!");

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

  Serial.printf("WS: Set all leds to main color: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
  ws2812fx_mode = FX_MODE_STATIC;
  mode = SET_MODE;
}

void handleSetSingleLED(uint8_t * mypayload, uint8_t firstChar = 0) {
  // decode led index
  char templed[3];
  strncpy (templed, (const char *) &mypayload[firstChar], 2 );
  uint8_t led = atoi(templed);

  Serial.printf("led value: [%i]. Entry threshold: <= [%i] (=> %s)\n", led, strip.numPixels(), mypayload );
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
    Serial.printf("rgb.red: [%s] rgb.green: [%s] rgb.blue: [%s]\n", redhex, greenhex, bluehex);
    Serial.printf("rgb.red: [%i] rgb.green: [%i] rgb.blue: [%i]\n", strtol(redhex, NULL, 16), strtol(greenhex, NULL, 16), strtol(bluehex, NULL, 16));
    Serial.printf("WS: Set single led [%i] to [%i] [%i] [%i] (%s)!\n", led, ledstates[led].red, ledstates[led].green, ledstates[led].blue, mypayload);


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
    Serial.printf("Setting RANGE from [%i] to [%i] as color [%s] \n", rangebegin, rangeend, colorval);

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
  snprintf(json, sizeof(json), "{\"mode\":%d, \"ws2812fx_mode\":%d, \"ws2812fx_mode_name\":\"%s\", \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}",
           mode, tmp_mode, modeName, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
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
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *payload, size_t len){
  if(type == WS_EVT_CONNECT){
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if(type == WS_EVT_DISCONNECT){
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if(type == WS_EVT_ERROR){
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)payload);
  } else if(type == WS_EVT_PONG){
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)payload:"");
  } else if(type == WS_EVT_DATA){
    AwsFrameInfo * info = (AwsFrameInfo*)arg;

    if(info->opcode == WS_TEXT) {
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), "text", info->len);
      
      if(payload[0] == '#') { // # ==> Set main color
        stateOn = true;
        handleSetMainColor(payload);
        Serial.printf("Set main color to: [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
        Serial.printf("WS: Set speed to: [%u]\n", ws2812fx_speed);
        mode = SETSPEED;
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
        Serial.printf("WS: Set brightness to: [%u]\n", brightness);
        mode = BRIGHTNESS;
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
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
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
              
      } else if (payload[0] == '+') { // + ==> Set multiple LED in the given colors
        
        handleSetDifferentColors(payload);
        client->text("OK");

        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
                
      } else if (payload[0] == 'R') { // R ==> Set range of LEDs in the given color
      
        handleRangeDifferentColors(payload);
        client->text("OK");

        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        
      } else if (payload[0] == '$') { // $ ==> Get status Info.
      
        Serial.printf("Get status info.");
        String json = listStatusJSON();
        Serial.println(json);
        client->text(json);
        
      } else if (payload[0] == '~') { // ~ ==> Get WS2812 modes.
      
        Serial.printf("Get WS2812 modes.");
        String json = listModesJSON();
        Serial.println(json);
        client->text(json);
        
      } else if (payload[0] == '/') { // / ==> Set WS2812 mode.
      
        handleSetWS2812FXMode(payload);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_AMQTT
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
        #endif
        #ifdef ENABLE_HOMEASSISTANT
          if(!taskSendHAState.isEnabled()) taskSendHAState.enableDelayed(TASK_SECOND * 4);
        #endif
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
        
      } else if (strcmp((char *)payload, "start") == 0 ) { // start auto cycling
        //handleAutoStart();
        client->text("OK");
        
      } else if (strcmp((char *)payload, "stop") == 0 ) { // stop auto cycling
        
        //handleAutoStop();
        client->text("OK");
        
      }
    }
  }
}

#ifdef ENABLE_AMQTT
  void connectToMqtt(void) {
    Serial.println("MQTT connecting...");
    mqttClient.connect();
  }
  
  void onMqttConnect(bool sessionPresent) {
    taskConnecttMqtt.disable();
    Serial.println("MQTT connected!");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
    char * message = new char[18 + strlen(HOSTNAME) + 1];
    strcpy(message, "McLighting ready: ");
    strcat(message, HOSTNAME);
    mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, message);
    //Subscribe
    uint16_t packetIdSub1 = mqttClient.subscribe((char *)mqtt_intopic.c_str(), qossub);
    Serial.printf("Subscribing at QoS %d, packetId: ", qossub); Serial.println(packetIdSub1);
    #ifdef ENABLE_HOMEASSISTANT
      uint16_t packetIdSub2 = mqttClient.subscribe((char *)mqtt_ha_state_in.c_str(), qossub);
      Serial.printf("Subscribing at QoS %d, packetId: ", qossub); Serial.println(packetIdSub2);
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
        Serial.println(buffer);
        mqttClient.publish(String("homeassistant/light/" + String(HOSTNAME) + "/config").c_str(), qospub, true, buffer);
      #endif
    #endif
  }
  
  
  void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.print("Disconnected from MQTT, reason: ");
    if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT) {
      Serial.println("Bad server fingerprint.");
    } else if (reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED) {
      Serial.println("TCP Disconnected.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION) {
      Serial.println("Bad server fingerprint.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_IDENTIFIER_REJECTED) {
      Serial.println("MQTT Identifier rejected.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_SERVER_UNAVAILABLE) {
      Serial.println("MQTT server unavailable.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_MALFORMED_CREDENTIALS) {
      Serial.println("MQTT malformed credentials.");
    } else if (reason == AsyncMqttClientDisconnectReason::MQTT_NOT_AUTHORIZED) {
      Serial.println("MQTT not authorized.");
    } else if (reason == AsyncMqttClientDisconnectReason::ESP8266_NOT_ENOUGH_SPACE) {
      Serial.println("Not enough space on esp8266.");
    }
    
    //if (WiFi.isConnected()) {
    if(mesh.getStationIP() != IPAddress(0,0,0,0)) {
      //taskConnecttMqtt.enableDelayed(TASK_SECOND * 2);
      taskConnecttMqtt.enable();
    } else {
      taskConnecttMqtt.disable();
    }
  }
  
  void onMqttMessage(char* topic, char* payload_in, AsyncMqttClientMessageProperties properties, size_t length, size_t index, size_t total) {
      Serial.print("MQTT: Recieved ["); Serial.print(topic);
      uint8_t * payload = (uint8_t *) malloc(length + 1);
      memcpy(payload, payload_in, length);
      payload[length] = NULL;
      Serial.printf("]: %s\n", payload);
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
          Serial.printf("MQTT: Set main color to [%u] [%u] [%u]\n", main_color.red, main_color.green, main_color.blue);
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
          Serial.printf("MQTT: Set speed to [%u]\n", ws2812fx_speed);
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
          Serial.printf("MQTT: Set brightness to [%u]\n", brightness);
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
          Serial.printf("MQTT: Set main color and light all LEDs [%s]\n", payload);
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
          Serial.printf("MQTT: Set single LED in given color [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == '+') { // + ==> Set multiple LED in the given colors
          
          handleSetDifferentColors(payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == 'R') { // R ==> Set range of LEDs in the given colors
          
          handleRangeDifferentColors(payload);
          Serial.printf("MQTT: Set range of LEDS to single color: [%s]\n", payload);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ") + String((char *)payload)).c_str());
          
        } else if (payload[0] == '$') { // $ ==> Get status Info.
  
          String json = String(listStatusJSON());
          Serial.printf("MQTT: Get status info.\n");
          Serial.println("MQTT: Out: " + json);
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, json.c_str());
          
        } else if (payload[0] == '~') { // ~ ==> Get WS2812 modes.
          
          Serial.printf("MQTT: Get WS2812 modes.\n");
          mqttClient.publish(mqtt_outtopic.c_str(), qospub, false, listModesJSON().c_str());
          
        } else if (payload[0] == '/') { // / ==> Set WS2812 mode.
          
          stateOn = true;
          handleSetWS2812FXMode(payload);
          Serial.printf("MQTT: Set WS2812 mode [%s]\n", payload);
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
bool updateFS = false;
#ifdef ENABLE_AMQTT
// Write configuration to FS JSON
bool writeConfigFS(bool saveConfig){
  if (saveConfig) {
    //FS save
    updateFS = true;
    Serial.print("Saving config: ");
    DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(4));
//    StaticJsonBuffer<JSON_OBJECT_SIZE(4)> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["mqtt_host"] = mqtt_host;
    json["mqtt_port"] = mqtt_port;
    json["mqtt_user"] = mqtt_user;
    json["mqtt_pass"] = mqtt_pass;
  
//      SPIFFS.remove("/config.json") ? Serial.println("removed file") : Serial.println("failed removing file");
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) Serial.println("failed to open config file for writing");

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    updateFS = false;
    return true;
    //end save
  } else {
    Serial.println("SaveConfig is False!");
    return false;
  }
}

// Read search_str to FS
bool readConfigFS() {
  //read configuration from FS JSON
  updateFS = true;
  if (SPIFFS.exists("/config.json")) {
    //file exists, reading and loading
    Serial.print("Reading config file... ");
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
      Serial.println("Opened!");
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(4)+300);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(4)+300> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      Serial.print("Config: ");
      json.printTo(Serial);
      if (json.success()) {
        Serial.println(" Parsed!");
        strcpy(mqtt_host, json["mqtt_host"]);
        strcpy(mqtt_port, json["mqtt_port"]);
        strcpy(mqtt_user, json["mqtt_user"]);
        strcpy(mqtt_pass, json["mqtt_pass"]);
        updateFS = false;
        return true;
      } else {
        Serial.println("Failed to load json config");
      }
    } else {
      Serial.println("Failed to open /config.json");
    }
  } else {
    Serial.println("Coudnt find config.json");
  }
  //end read
  updateFS = false;
  return false;
}
#endif

bool writeStateFS(){
  updateFS = true;
  //save the strip state to FS JSON
  Serial.print("Saving cfg: ");
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

//      SPIFFS.remove("/state.json") ? Serial.println("removed file") : Serial.println("failed removing file");
  File configFile = SPIFFS.open("/stripstate.json", "w");
  if (!configFile) {
    Serial.println("Failed!");
    updateFS = false;
    return false;
  }
  json.printTo(Serial);
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
    Serial.print("Read cfg: ");
    File configFile = SPIFFS.open("/stripstate.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(7)+200);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(7)+200> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
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
        Serial.println("Failed to parse JSON!");
      }
    } else {
      Serial.println("Failed to open \"/stripstate.json\"");
    }
  } else {
    Serial.println("Coudnt find \"/stripstate.json\"");
  }
  //end read
  updateFS = false;
  return false;
}
#endif

#ifdef ENABLE_AMQTT
void async_mqtt_setup(void){
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(mqtt_host, atoi(mqtt_port));
  if (mqtt_user != "" or mqtt_pass != "") mqttClient.setCredentials(mqtt_user, mqtt_pass);
  mqttClient.setClientId(HOSTNAME);  
}
#endif

#ifdef ENABLE_HOMEASSISTANT
void fnSendHAState(void){
  String json = statusLEDs();
  mqttClient.publish(mqtt_ha_state_out.c_str(), 1, true, json.c_str());
  Serial.printf("MQTT: Send [%s]: %s\n", mqtt_ha_state_out.c_str(), json.c_str());
  taskSendHAState.disable();
}
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
void fnSpiffsSaveState(void){
  Serial.println((writeStateFS()) ? " Success!" : " Failure!");
  taskSpiffsSaveState.disable();
}
#endif

///////// Mesh Stuff ///////////

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}


void modes_setup() {
  modes = "<li><a href='#' class='m' id='0'>OFF</a></li>";
  uint8_t num_modes = sizeof(myModes) > 0 ? sizeof(myModes) + 1 : strip.getModeCount() + 1;
  for(uint8_t i=1; i < num_modes; i++) {
    uint8_t m = sizeof(myModes) > 0 ? myModes[i-1] : i;
    modes += "<li><a href='#' class='m' id='";
    modes += (m);
    modes += "'>";
    modes += strip.getModeName(m-1);
    modes += "</a></li>";
  }
}

void sendMessage() {
  String msg = statusLEDs();
  mesh.sendBroadcast(msg);
  taskSendMessage.disable();
  Serial.println("Broadcasting!");
}

void receivedCallback(uint32_t from, String & msg) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if(!processJson(msg)) return;
  else Serial.printf("Processed incoming message from %u successfully!", from);
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
}

void newConnectionCallback(uint32_t nodeId) {
  String msg = statusLEDs();
  mesh.sendSingle(nodeId, msg);
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();

  if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
}

void mesh_setup() {
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname("MeshyMcLighting");
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  mesh.setHostname(HOSTNAME);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, STATION_WIFI_CHANNEL );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}
