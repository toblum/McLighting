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

bool processJson(String &message) {
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 150;
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
  Serial.printf("{\"mode\":%d, \"ws2812fx_mode\":%d, \"speed\":%d, \"brightness\":%d, \"color\":[%d, %d, %d]}\n", mode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  jsonBuffer.clear();
  return true;
}

uint16_t convertSpeed(uint8_t mcl_speed) {
  //long ws2812_speed = mcl_speed * 256;
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = constrain(SPEED_MAX - ws2812_speed, SPEED_MIN, SPEED_MAX);
  return ws2812_speed;
}

#ifdef ENABLE_WEBSERVER

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
#endif

#ifdef ENABLE_STATE_SAVE_SPIFFS
bool updateFS = false;

bool writeStateFS(){
  updateFS = true;
  //save the strip state to FS JSON
  Serial.print("Saving cfg: ");
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
      DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(8)+200);
//      StaticJsonBuffer<JSON_OBJECT_SIZE(7)+200> jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
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

void fnSpiffsSaveState(void){
  Serial.println((writeStateFS()) ? " Success!" : " Failure!");
  taskSpiffsSaveState.disable();
}
#endif

/////// Mesh Stuff //////

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
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if(!processJson(msg)) return;
  else Serial.printf("Processed incoming message from %u successfully!\n", from);
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
}

//void newConnectionCallback(uint32_t nodeId) {
//  String msg = statusLEDs();
//  mesh.sendSingle(nodeId, msg);
//  Serial.printf("New Connection, nodeId = %u\n", nodeId);
//}

void mesh_setup() {
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname("MeshyMcLighting");
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  mesh.setHostname(HOSTNAME);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, STATION_WIFI_CHANNEL );
  mesh.onReceive(&receivedCallback);
  //mesh.onNewConnection(&newConnectionCallback);
  
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.disable();
}
