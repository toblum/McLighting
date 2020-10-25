#include <ArduinoJson.h>         //https://github.com/bblanchon/ArduinoJson

#if defined(ARDUINOJSON_VERSION)
  #if !(ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR >= 8)
    #error "Install ArduinoJson v6.8.x or higher"
  #endif
#endif

char * listStateJSONfull() {
  const size_t bufferSize = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(19) + 250;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["segment"]    = State.segment;
  root["start"]      = segState.start;
  root["stop"]       = segState.stop;
  root["mode"]       = (uint8_t) State.mode;
  root["fx_mode"]    = segState.mode[State.segment];
  root["speed"]      = segState.speed[State.segment];
  root["brightness"] = State.brightness;
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
  root["ws_fxopt"]  = segState.options;
  root["hostname"] = HOSTNAME;
  #if defined(ENABLE_MQTT)
    root["mqtt_host"] = mqtt_host;
    root["mqtt_port"] = mqtt_port;
    root["mqtt_user"] = mqtt_user;
    root["mqtt_pass"] = mqtt_pass;
  #endif
  root["ws_seg"]    = Config.segments;
  root["ws_cnt"]    = Config.stripSize;
  root["ws_rgbo"]   = Config.RGBOrder;
  root["ws_pin"]    = Config.pin;
  root["ws_trans"]  = Config.transEffect;  
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

char * listStateJSON() {
  const size_t bufferSize = JSON_OBJECT_SIZE(3) + 25;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["segment"] = State.segment;
  root["mode"] = (uint8_t) State.mode;
  root["brightness"] = State.brightness;
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

char * listSegmentStateJSON(uint8_t _seg) {
  const size_t bufferSize = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(6) + 100;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["start"]   = strip->getSegment(_seg)->start;
  root["stop"]    = strip->getSegment(_seg)->stop;
  root["fx_mode"] = segState.mode[_seg];
  root["speed"]   = segState.speed[_seg];
  JsonArray color = root.createNestedArray("color");
  //color.add((strip->getColors(_seg)[0] >> 24) & 0xFF);
  color.add((segState.colors[_seg][0] >> 24) & 0xFF);
  color.add((segState.colors[_seg][0] >> 16) & 0xFF);
  color.add((segState.colors[_seg][0] >>  8) & 0xFF);
  color.add((segState.colors[_seg][0])  & 0xFF);
  color.add((segState.colors[_seg][1] >> 24) & 0xFF);
  color.add((segState.colors[_seg][1] >> 16) & 0xFF);
  color.add((segState.colors[_seg][1] >>  8) & 0xFF);
  color.add((segState.colors[_seg][1])  & 0xFF);
  color.add((segState.colors[_seg][2] >> 24) & 0xFF);
  color.add((segState.colors[_seg][2] >> 16) & 0xFF);
  color.add((segState.colors[_seg][2] >>  8) & 0xFF);
  color.add((segState.colors[_seg][2])  & 0xFF);
  root["ws_fxopt"] = strip->getOptions(_seg);
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getStateJSON() {
  char * buffer = listStateJSONfull();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

char * listConfigJSON() {
  #if defined(ENABLE_MQTT)
    const size_t bufferSize = JSON_OBJECT_SIZE(10) + 150;
  #else
    const size_t bufferSize = JSON_OBJECT_SIZE(6) + 100;
  #endif
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["hostname"] = HOSTNAME;
  #if defined(ENABLE_MQTT)
    root["mqtt_host"] = mqtt_host;
    root["mqtt_port"] = mqtt_port;
    root["mqtt_user"] = mqtt_user;
    root["mqtt_pass"] = mqtt_pass;
  #endif
  root["ws_seg"]    = Config.segments;
  root["ws_cnt"]    = Config.stripSize;
  root["ws_rgbo"]   = Config.RGBOrder;
  root["ws_pin"]    = Config.pin;
  root["ws_trans"]  = Config.transEffect;
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getConfigJSON() {
  char * buffer = listConfigJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

char * listModesJSON() {
  const size_t bufferSize = JSON_ARRAY_SIZE(strip->getModeCount() + 3) + (strip->getModeCount() + 1)*JSON_OBJECT_SIZE(2) + 2000;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonArray root = jsonBuffer.to<JsonArray>();
  JsonObject objecttoggle = root.createNestedObject();
  objecttoggle["mode"] = "toggle";
  objecttoggle["name"] = "TOGGLE";
  JsonObject objecton = root.createNestedObject();
  objecton["mode"] = "on";
  objecton["name"] = "ON";
  JsonObject objectoff = root.createNestedObject();
  objectoff["mode"] = "off";
  objectoff["name"] = "OFF";
  for (uint8_t i = 0; i < strip->getModeCount(); i++) {
    JsonObject object = root.createNestedObject();
    object["mode"] = i;
    object["name"] = strip->getModeName(i);
  }
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}

void getModesJSON() {
  char * buffer = listModesJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send ( 200, "application/json", buffer);
  free (buffer);
}

char * listESPStateJSON() {
  const size_t bufferSize = JSON_OBJECT_SIZE(31) + 1500;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["HOSTNAME"] = HOSTNAME;
  root["version"] = SKETCH_VERSION;
  root["heap"] = ESP.getFreeHeap();
  root["sketch_size"] = ESP.getSketchSize();
  root["free_sketch_space"] = ESP.getFreeSketchSpace();
  root["flash_chip_size"] = ESP.getFlashChipSize();
  root["flash_chip_real_size"] = ESP.getFlashChipRealSize();
  root["flash_chip_speed"] = ESP.getFlashChipSpeed();
  root["sdk_version"] = ESP.getSdkVersion();
  root["core_version"] = ESP.getCoreVersion();
  root["cpu_freq"] = ESP.getCpuFreqMHz();
  root["chip_id"] = ESP.getFlashChipId();
  #if defined(USE_WS2812FX_DMA)
    #if USE_WS2812FX_DMA == 0
      root["animation_lib"] = "WS2812FX_DMA";
    #endif
    #if USE_WS2812FX_DMA == 1
      root["animation_lib"] = "WS2812FX_UART1";
    #endif
    #if USE_WS2812FX_DMA == 2
      root["animation_lib"] = "WS2812FX_UART2";
    #endif
  #else
    root["animation_lib"] = "WS2812FX";
  #endif
  root["ws2812_pin"]  = Config.pin;
  root["led_count"] = Config.stripSize;
  root["rgb_order"] = Config.RGBOrder;
  if (strstr(Config.RGBOrder, "W") != NULL) {
    root["rgbw_mode"] = "ON";
  } else {
    root["rgbw_mode"] = "OFF";
  }
  #if defined(ENABLE_BUTTON)
    root["button_mode"] = "ON";
    root["button_pin"] = ENABLE_BUTTON;
  #else
    root["button_mode"] = "OFF";
  #endif
  #if defined(ENABLE_BUTTON_GY33)
    root["button_gy33"] = "ON";
    root["gy33_pin"] = ENABLE_BUTTON_GY33;
  #else
    root["button_gy33"] = "OFF";
  #endif
  #if defined(ENABLE_REMOTE)
    root["ir_remote"] = "ON";
    root["tsop_ir_pin"] = ENABLE_REMOTE;
  #else
    root["ir_remote"] = "OFF";
  #endif
  #if defined(ENABLE_MQTT)
    #if ENABLE_MQTT == 0
      root["mqtt"] = "MQTT";
    #endif
    #if ENABLE_MQTT == 1
      root["mqtt"] = "AMQTT";
    #endif
  #else
    root["mqtt"] = "OFF";
  #endif
  #if defined(ENABLE_HOMEASSISTANT)
    root["home_assistant"] = "ON";
  #else
    root["home_assistant"] = "OFF";
  #endif
  #if defined(ENABLE_OTA)
    #if ENABLE_OTA == 0
      root["ota"] = "ARDUINO";
    #endif
    #if ENABLE_OTA == 1
      root["ota"] = "HTTP";
    #endif
  #else
    root["ota"] = "OFF";
  #endif
  #if defined(ENABLE_STATE_SAVE)
    root["state_save"] = "SPIFFS";
  #else
    root["state_save"] = "OFF";
  #endif
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}
void getESPStateJSON() {
  char * buffer = listESPStateJSON();
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buffer);
  free (buffer);
}
