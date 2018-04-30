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
    
  root["speed"] = ws2812fx_speed;
  
  char modeName[30];
  strncpy_P(modeName, (PGM_P)strip.getModeName(strip.getMode()), sizeof(modeName)); // copy from progmem
  root["effect"] = modeName;
  
  char buffer[root.measureLength() + 1];
  root.printTo(buffer, sizeof(buffer));

  return String(buffer);
}

bool processJson(String &message) {
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

#ifdef ENABLE_WEBSERVER
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
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '*') { // * ==> Set main color and light all LEDs (Shortcut)
        
        handleSetAllMode(payload);
        client->text("OK");

        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
      
      } else if (payload[0] == '!') { // ! ==> Set single LED in given color
        
        handleSetSingleLED(payload, 1);
        client->text("OK");
              
      } else if (payload[0] == '+') { // + ==> Set multiple LED in the given colors
        
        handleSetDifferentColors(payload);
        client->text("OK");
                
      } else if (payload[0] == 'R') { // R ==> Set range of LEDs in the given color
      
        handleRangeDifferentColors(payload);
        client->text("OK");
        
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
#endif

#ifdef ENABLE_STATE_SAVE_SPIFFS
bool updateFS = false;

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
#endif

#ifdef ENABLE_STATE_SAVE_SPIFFS
void fnSpiffsSaveState(void){
  DEBUG_PRINTLN((writeStateFS()) ? " Success!" : " Failure!");
  taskSpiffsSaveState.disable();
}
#endif

void modes_setup(void) {
//  modes = "<li><a href='#' class='m' id='0'>OFF</a></li>";
//  uint8_t num_modes = sizeof(myModes) > 0 ? sizeof(myModes) + 1 : strip.getModeCount() + 1;
//  for(uint8_t i=1; i < num_modes; i++) {
//    uint8_t m = sizeof(myModes) > 0 ? myModes[i-1] : i;
//    modes += "<li><a href='#' class='m' id='";
//    modes += (m);
//    modes += "'>";
//    modes += strip.getModeName(m-1);
//    modes += "</a></li>";
//  }
  modes = "<li><a href='#' class='m' id='0'>OFF</a></li>";
  for(uint8_t i=1; i < strip.getModeCount() + 1; i++) {
    modes += "<li><a href='#' class='m' id='";
    modes += (i);
    modes += "'>";
    modes += strip.getModeName(i-1);
    modes += "</a></li>";
  }
}

void autoTick(void) {
  main_color.red =   ((autoParams[autoCount][0] >> 16) & 0xFF);
  main_color.green = ((autoParams[autoCount][0] >>  8) & 0xFF);
  main_color.blue =  ((autoParams[autoCount][0] >>  0) & 0xFF);
  ws2812fx_speed = convertSpeed((uint8_t) autoParams[autoCount][1]);
  ws2812fx_mode = (uint8_t) autoParams[autoCount][2];

  autoModeTask.delay((float) autoParams[autoCount][3]);
  
  DEBUG_PRINTF("autoTick %d\n", autoCount);
  taskSendMessage.enableIfNot();
  taskSendMessage.forceNextIteration();
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
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
    } else {
      mode = OFF;
      buttonState = false;
      stateOn = false;
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
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
    if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
    #endif
  }

  // called when button is kept pressed for 2 seconds or more
  void longKeyPress() {
    DEBUG_PRINT("Long button press\n");
    setModeByStateString(BTN_MODE_LONG);
    stateOn = true;
    if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
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
  DEBUG_PRINTF3("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if(!processJson(msg)) return;
  else DEBUG_PRINTF("Processed incoming message from %u successfully!\n", from);
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
}
