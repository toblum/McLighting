char * listStatusJSON2() {
  const size_t bufferSize = 10*JSON_ARRAY_SIZE(12) + 10*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(12) + 600;
  DynamicJsonDocument jsonBuffer(bufferSize);
  JsonObject root = jsonBuffer.to<JsonObject>();
  root["mode"] = (uint8_t) mode;
  root["brightness"] = brightness;
  for(uint8_t i=0; i<10; i++) {
    char int2char[3];
    itoa(i, int2char, 10);
    JsonObject segments = root.createNestedObject(int2char);
    segments["ws2812fx_mode"] = strip->getMode(i);
    segments["ws2812fx_mode_name"] = strip->getModeName(strip->getMode(i));
    segments["speed"] = ws2812fx_speed;
    JsonArray color = segments.createNestedArray("color");
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
  }
  uint16_t msg_len = measureJson(root) + 1;
  char * buffer = (char *) malloc(msg_len);
  serializeJson(root, buffer, msg_len);
  jsonBuffer.clear();
  return buffer;
}
