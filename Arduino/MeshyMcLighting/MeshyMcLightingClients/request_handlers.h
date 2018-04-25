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

uint16_t convertSpeed(uint8_t mcl_speed) {
  //long ws2812_speed = mcl_speed * 256;
  uint16_t ws2812_speed = 61760 * (exp(0.0002336 * mcl_speed) - exp(-0.03181 * mcl_speed));
  ws2812_speed = constrain(SPEED_MAX - ws2812_speed, SPEED_MIN, SPEED_MAX);
  return ws2812_speed;
}

#ifdef ENABLE_STATE_SAVE_SPIFFS
bool updateFS = false;

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

void fnSpiffsSaveState(void){
  Serial.println((writeStateFS()) ? " Success!" : " Failure!");
  taskSpiffsSaveState.disable();
}
#endif

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}
