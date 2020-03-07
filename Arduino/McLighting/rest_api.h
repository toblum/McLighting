// ***************************************************************************
// Setup: Webserver handler
// ***************************************************************************
//list directory
server.on("/list", HTTP_GET, handleFileList);
//create file
server.on("/edit", HTTP_PUT, handleFileCreate);
//delete file
server.on("/edit", HTTP_DELETE, handleFileDelete);
//first callback is called after the request has ended with all parsed arguments
//second callback handles file uploads at that location
server.on("/edit", HTTP_POST, []() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "");
}, handleFileUpload);

// ***************************************************************************
// Setup: SPIFFS Webserver handler
// ***************************************************************************

server.on("/", HTTP_GET, [&](){
  #if defined(USE_HTML_MIN_GZ)
    server.sendHeader("Content-Encoding", "gzip", true);
    server.send_P(200, PSTR("text/html"), index_htm_gz, index_htm_gz_len);
  #else
    if (!handleFileRead(server.uri()))
      handleNotFound();
  #endif
});

server.on("/material.woff2", HTTP_GET, [&](){
  #if defined(USE_HTML_MIN_GZ)
    server.send_P(200, PSTR("text/plain"), material_icons_woff2, material_icons_woff2_len);
  #else
    if (!handleFileRead(server.uri()))
      handleNotFound();
  #endif
});

server.on("/favicon.ico", HTTP_GET, [&](){
  #if defined(USE_HTML_MIN_GZ)
    server.sendHeader("Content-Encoding", "gzip", true);
    server.send_P(200, PSTR("text/plain"), fav_icon, fav_icon_len);
  #else
    if (!handleFileRead(server.uri()))
      handleNotFound();
  #endif
});

server.on("/apple-touch-icon.png", HTTP_GET, [&](){
  #if defined(USE_HTML_MIN_GZ)
    server.send_P(200, PSTR("text/plain"), apple_touch_icon_png, apple_touch_icon_png_len);
  #else
    if (!handleFileRead(server.uri()))
      handleNotFound();
  #endif
});

server.on("/edit", HTTP_GET, [&](){
  #if defined(USE_HTML_MIN_GZ)
    server.sendHeader("Content-Encoding", "gzip", true);
    server.send_P(200, PSTR("text/html"), edit_htm_gz, edit_htm_gz_len);
  #else
    if (!handleFileRead("/edit.htm"))
      handleNotFound();
  #endif
});


//called when the url is not defined here
//use it to load content from SPIFFS
server.onNotFound([]() {
  if (!handleFileRead(server.uri()))
    handleNotFound();
});

server.on("/upload", handleMinimalUpload);

server.on("/esp_status", HTTP_GET, []() { //get heap status, analog input value and all GPIO statuses in one json call 
  getESPStateJSON();
});

server.on("/restart", []() {
  DBG_OUTPUT_PORT.printf("/restart\r\n");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "restarting..." );
  ESP.restart();
});

server.on("/reset_wlan", []() {
  DBG_OUTPUT_PORT.printf("/reset_wlan\r\n");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Resetting WLAN and restarting..." );
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
});

server.on("/start_config_ap", []() {
  DBG_OUTPUT_PORT.printf("/start_config_ap\r\n");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Starting config AP ..." );
  WiFiManager wifiManager;
  wifiManager.startConfigPortal(HOSTNAME);
});

server.on("/format_spiffs", []() {
  DBG_OUTPUT_PORT.printf("/format_spiffs\r\n");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Formatting SPIFFS ..." );
  SPIFFS.format();
});

server.on("/get_brightness", []() {
  char str_brightness[4];
  snprintf(str_brightness, sizeof(str_brightness), "%i", (int) (State.brightness / 2.55));
  str_brightness[sizeof(str_brightness) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", str_brightness );
  DBG_OUTPUT_PORT.printf("/get_brightness: %i\r\n", (int) (State.brightness / 2.55));
});

server.on("/get_speed", []() {
  char str_speed[4];
  snprintf(str_speed, sizeof(str_speed), "%i", segState.speed[State.segment]);
  str_speed[sizeof(str_speed) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", str_speed );
  DBG_OUTPUT_PORT.printf("/get_speed: %i\r\n", segState.speed[State.segment]);
});

server.on("/get_switch", []() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", (State.mode == OFF) ? "0" : "1" );
  DBG_OUTPUT_PORT.printf("/get_switch: %s\r\n", (State.mode == OFF) ? "0" : "1");
});

server.on("/get_color", []() {
  char rgbcolor[7];
  snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X", main_color.red, main_color.green, main_color.blue);
  rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", rgbcolor);
  DBG_OUTPUT_PORT.print("/get_color: ");
  DBG_OUTPUT_PORT.println(rgbcolor);
});

server.on("/get_color1", []() {
  char rgbcolor[9];
  snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", main_color.white, main_color.red, main_color.green, main_color.blue);
  rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", rgbcolor );
  DBG_OUTPUT_PORT.print("/get_color1: ");
  DBG_OUTPUT_PORT.println(rgbcolor);
});

server.on("/get_color2", []() {
  char rgbcolor[9];
  snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", back_color.white, back_color.red, back_color.green, back_color.blue);
  rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", rgbcolor );
  DBG_OUTPUT_PORT.print("/get_color2: ");
  DBG_OUTPUT_PORT.println(rgbcolor);
});

server.on("/get_color3", []() {
  char rgbcolor[9];
  snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", xtra_color.white, xtra_color.red, xtra_color.green, xtra_color.blue);
  rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", rgbcolor );
  DBG_OUTPUT_PORT.print("/get_color3: ");
  DBG_OUTPUT_PORT.println(rgbcolor);
});

server.on("/get_modes", []() {
  getModesJSON();
});

server.on("/status", []() {
  getStateJSON();
});

server.on("/config", []() {

  /*

  // This will be used later when web-interface is ready and HTTP_GET will not be allowed to update the Strip Settings

  if(server.args() == 0 and server.method() != HTTP_POST)
  {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Only HTTP POST method is allowed and check the number of arguments!");
    return;
  }

  */
  // ToDo do not save if no change
  bool _updateStrip = false;
  bool _updateConfig  = false;
  bool _updateState = false;
  if(server.hasArg("ws_seg")){
    uint8_t _ws_seg = server.arg("ws_seg").toInt();
    _ws_seg = constrain(_ws_seg, 1, MAX_NUM_SEGMENTS - 1);
    if (_ws_seg != Config.segments){
      Config.segments = _ws_seg;
      _updateStrip = true;
      if (State.segment >=  Config.segments) {
        State.segment = Config.segments - 1;
        _updateState = true;
      }
    }
  }
  if(server.hasArg("ws_cnt")){
    uint16_t _stripSize = server.arg("ws_cnt").toInt();
    if (_stripSize > 0) {
      _stripSize = constrain(_stripSize, 1, MAXLEDS);
      if (_stripSize != Config.stripSize) {
        Config.stripSize = _stripSize;
        _updateStrip = true;   
      }
    }
  }
  if(server.hasArg("ws_rgbo")){
    char _ws_rgbo[5];
    snprintf(_ws_rgbo, sizeof(_ws_rgbo), "%s", server.arg("ws_rgbo").c_str());
    _ws_rgbo[sizeof(_ws_rgbo) - 1] = 0x00;
    checkRGBOrder(_ws_rgbo);
    _updateStrip = true;
  }
  
#if !defined(USE_WS2812FX_DMA)    
  if(server.hasArg("ws_pin")){
    if (checkPin(server.arg("ws_pin").toInt())) {
      _updateStrip = true;
      DBG_OUTPUT_PORT.print("Pin was set to: ");
      DBG_OUTPUT_PORT.println(Config.pin);
    } else {
      DBG_OUTPUT_PORT.println("invalid input or same value!");
    }
  }
#endif
  
  if(_updateStrip) {
    initStrip();
  }
  
  if(server.hasArg("hostname")){
    char _hostname[sizeof(HOSTNAME)]; 
    snprintf(_hostname, sizeof(_hostname), "%s", server.arg("hostname").c_str());
    _hostname[sizeof(_hostname) - 1] = 0x00;
    if (strcmp(HOSTNAME, _hostname) != 0) {
      strcpy(HOSTNAME, _hostname);
      _updateConfig = true;
    }
  }
  
#if defined(ENABLE_MQTT)   
  if(server.hasArg("mqtt_host")){
    char _mqtt_host[sizeof(mqtt_host)];
    snprintf(_mqtt_host, sizeof(_mqtt_host), "%s", server.arg("mqtt_host").c_str());
    _mqtt_host[sizeof(_mqtt_host) - 1] = 0x00;
    if (strcmp(mqtt_host, _mqtt_host) != 0) {
      strcpy(mqtt_host, _mqtt_host);
      _updateConfig = true;
    }
  }
  if(server.hasArg("mqtt_port")){
    uint16_t _mqtt_port = constrain(server.arg("mqtt_port").toInt(), 1, 65535);
    if (_mqtt_port != mqtt_port) {
      mqtt_port = _mqtt_port;
      _updateConfig = true;
    }    
  }
  if(server.hasArg("mqtt_user")){
    char _mqtt_user[sizeof(mqtt_user)];
    snprintf(_mqtt_user, sizeof(_mqtt_user), "%s", server.arg("mqtt_user").c_str());
    _mqtt_user[sizeof(mqtt_user) - 1] = 0x00;
    if (strcmp(mqtt_user, _mqtt_user) != 0) {    
      strcpy(mqtt_user, _mqtt_user);
      _updateConfig = true;
    }
  }
  if(server.hasArg("mqtt_pass")){
    char _mqtt_pass[sizeof(mqtt_pass)];
    snprintf(_mqtt_pass, sizeof(_mqtt_pass), "%s", server.arg("mqtt_pass").c_str());
    _mqtt_pass[sizeof(_mqtt_pass) - 1] = 0x00;
    if (strcmp(mqtt_pass, _mqtt_pass) != 0) {    
      strcpy(mqtt_pass, _mqtt_pass);
      _updateConfig = true;
    }
  }
  if (_updateConfig) {
    initMqtt();
  }
#endif

  if(server.hasArg("trans_effect")){
    Config.transEffect = server.arg("trans_effect").toInt();
    _updateConfig = true;
  }

#if defined(ENABLE_STATE_SAVE)
  if (_updateStrip || _updateConfig) {
    if(save_conf.active()) save_conf.detach();
    save_conf.once(3, tickerSaveConfig);
  }
  if (_updateState) {
    if(save_state.active()) save_state.detach();
    save_state.once(3, tickerSaveState);
  }
#endif
  _updateStrip = false;
  _updateConfig = false;
  _updateState = false;
  getConfigJSON();
});

server.on("/off", []() {
  if (State.mode == OFF) { State.mode = SET; } else { State.mode = OFF; };
  getACK("OK");
  #if defined(ENABLE_STATE_SAVE)
    if(save_state.active()) save_state.detach();
    save_state.once(3, tickerSaveState);
  #endif
});

server.on("/on", []() {
  if (prevmode == OFF) {
    State.mode = SET;
    getACK("OK");
    #if defined(ENABLE_STATE_SAVE)
      if(save_state.active()) save_state.detach();
      save_state.once(3, tickerSaveState);
    #endif
  } else {
    getACK("NOK"); 
  }
});

server.on("/set", []() {
  prevmode = HOLD;
  boolean _updateState = false;
  boolean _updateSegState = false;
  // Segment
  if ((server.arg("seg") != "") && (server.arg("seg").toInt() >= 0) && (server.arg("seg").toInt() <  Config.segments)) { 
      uint8_t _seg = server.arg("seg").toInt();
      if (prevsegment != _seg) {
        prevsegment = State.segment;
        State.segment = _seg;
        getSegmentParams(State.segment);
        //memcpy(hexcolors_trans, segState.colors[State.segment], sizeof(hexcolors_trans));     
        State.mode = SET;
        _updateState = true;
      }      
  }
  if ((server.arg("start") != "") && (server.arg("start").toInt() >= 0) && (server.arg("start").toInt() <= segState.stop)) { 
      uint16_t _seg_start = server.arg("start").toInt();  
      _seg_start = constrain(segState.start, 0, Config.stripSize -1);
      if (_seg_start != segState.start) {
        segState.start = _seg_start;
        setSegmentSize();
        _updateSegState = true;
      }
  }
  if ((server.arg("stop") != "") && (server.arg("stop").toInt() >= segState.start) && (server.arg("stop").toInt() <= Config.stripSize)) { 
      uint16_t _seg_stop = server.arg("stop").toInt();
      _seg_stop = constrain(_seg_stop, segState.start, Config.stripSize - 1);
      if (_seg_stop != segState.stop) {
        segState.stop = _seg_stop;
        setSegmentSize();
        _updateSegState = true;
      }
  }

  if(server.hasArg("fxopt")){
    uint8_t _fx_options = ((constrain(server.arg("fxopt").toInt(), 0, 255)>>1)<<1);
    if (_fx_options != segState.options) {
      segState.options = _fx_options;
      strip->setOptions(State.segment, segState.options);
      _updateSegState = true;
    }
  }
  //color wrgb
  if (server.arg("rgb") != "") {
    uint32_t rgb = (uint32_t) strtoul(server.arg("rgb").c_str(), NULL, 16);
    main_color.white = ((rgb >> 24) & 0xFF);
    main_color.red = ((rgb >> 16) & 0xFF);
    main_color.green = ((rgb >> 8) & 0xFF);
    main_color.blue = ((rgb >> 0) & 0xFF);
    _updateSegState = true;
  } else {
    if ((server.arg("r") != "") && (server.arg("r").toInt() >= 0) && (server.arg("r").toInt() <= 255)) { 
      main_color.red = server.arg("r").toInt();
      _updateSegState = true;
    }
    if ((server.arg("g") != "") && (server.arg("g").toInt() >= 0) && (server.arg("g").toInt() <= 255)) {
      main_color.green = server.arg("g").toInt();
      _updateSegState = true;
    }
    if ((server.arg("b") != "") && (server.arg("b").toInt() >= 0) && (server.arg("b").toInt() <= 255)) {
      main_color.blue = server.arg("b").toInt();
      _updateSegState = true;
    }
    if ((server.arg("w") != "") && (server.arg("w").toInt() >= 0) && (server.arg("w").toInt() <= 255)){
      main_color.white = server.arg("w").toInt();
      _updateSegState = true;
    }
  } 
  if (server.arg("rgb2") != "") {
    uint32_t rgb2 = (uint32_t) strtoul(server.arg("rgb2").c_str(), NULL, 16);
    back_color.white = ((rgb2 >> 24) & 0xFF);
    back_color.red = ((rgb2 >> 16) & 0xFF);
    back_color.green = ((rgb2 >> 8) & 0xFF);
    back_color.blue = ((rgb2 >> 0) & 0xFF);
    _updateSegState = true;
  } else {
    if ((server.arg("r2") != "") && (server.arg("r2").toInt() >= 0) && (server.arg("r2").toInt() <= 255)) { 
      back_color.red = server.arg("r2").toInt();
      _updateSegState = true;
    }
    if ((server.arg("g2") != "") && (server.arg("g2").toInt() >= 0) && (server.arg("g2").toInt() <= 255)) {
      back_color.green = server.arg("g2").toInt();
      _updateSegState = true;
    }
    if ((server.arg("b2") != "") && (server.arg("b2").toInt() >= 0) && (server.arg("b2").toInt() <= 255)) {
      back_color.blue = server.arg("b2").toInt();
      _updateSegState = true;
    }
    if ((server.arg("w2") != "") && (server.arg("w2").toInt() >= 0) && (server.arg("w2").toInt() <= 255)){
      back_color.white = server.arg("w2").toInt();
      _updateSegState = true;
    }
  }
  if (server.arg("rgb3") != "") {
    uint32_t rgb3 = (uint32_t) strtoul(server.arg("rgb3").c_str(), NULL, 16);
    xtra_color.white = ((rgb3 >> 24) & 0xFF);
    xtra_color.red = ((rgb3 >> 16) & 0xFF);
    xtra_color.green = ((rgb3 >> 8) & 0xFF);
    xtra_color.blue = ((rgb3 >> 0) & 0xFF);
    _updateSegState = true;
  } else {
    if ((server.arg("r3") != "") && (server.arg("r3").toInt() >= 0) && (server.arg("r3").toInt() <= 255)) { 
      xtra_color.red = server.arg("r3").toInt();
      _updateSegState = true;
    }
    if ((server.arg("g3") != "") && (server.arg("g3").toInt() >= 0) && (server.arg("g3").toInt() <= 255)) {
      xtra_color.green = server.arg("g3").toInt();
      _updateSegState = true;
    }
    if ((server.arg("b3") != "") && (server.arg("b3").toInt() >= 0) && (server.arg("b3").toInt() <= 255)) {
      xtra_color.blue = server.arg("b3").toInt();
      _updateSegState = true;
    }
    if ((server.arg("w3") != "") && (server.arg("w3").toInt() >= 0) && (server.arg("w3").toInt() <= 255)){
      xtra_color.white = server.arg("w3").toInt();
      _updateSegState = true;
    }
  }
  main_color.red = constrain(main_color.red, 0, 255);
  main_color.green = constrain(main_color.green, 0, 255);
  main_color.blue = constrain(main_color.blue, 0, 255);
  main_color.white = constrain(main_color.white, 0, 255);
  back_color.red = constrain(back_color.red, 0, 255);
  back_color.green = constrain(back_color.green, 0, 255);
  back_color.blue = constrain(back_color.blue, 0, 255);
  back_color.white = constrain(back_color.white, 0, 255);
  xtra_color.red = constrain(xtra_color.red, 0, 255);
  xtra_color.green = constrain(xtra_color.green, 0, 255);
  xtra_color.blue = constrain(xtra_color.blue, 0, 255);
  xtra_color.white = constrain(xtra_color.white, 0, 255);
  
  
  // Speed
  if ((server.arg("s") != "") && (server.arg("s").toInt() >= 0) && (server.arg("s").toInt() <= 255)) {
    segState.speed[State.segment] = constrain(server.arg("s").toInt(), 0, 255);
    _updateSegState = true;
  }
  //Mode
  if ((server.arg("m") != "") && (server.arg("m").toInt() >= 0) && (server.arg("m").toInt() <= strip->getModeCount())) {
    fx_mode = constrain(server.arg("m").toInt(), 0, strip->getModeCount() - 1);
    if (fx_mode !=  segState.mode[State.segment]) {
      _updateSegState = true;
    }
  }
  
  // Brightness
  if ((server.arg("c") != "") && (server.arg("c").toInt() >= 0) && (server.arg("c").toInt() <= 100)) { 
    State.brightness = constrain((int) server.arg("c").toInt() * 2.55, 0, 255);
  } else if ((server.arg("p") != "") && (server.arg("p").toInt() >= 0) && (server.arg("p").toInt() <= 255)) {
    State.brightness = constrain(server.arg("p").toInt(), 0, 255);
  }
  if (strip->getBrightness() != State.brightness) {
    State.mode = SET;
    _updateState = true;
  }
  //DBG_OUTPUT_PORT.printf("Get Args: %s\r\n", listStateJSONfull()); //possibly causing heap problems
  getACK("OK");
  
#if defined(ENABLE_STATE_SAVE)
  if (_updateState) {
    if(save_state.active()) save_state.detach();
    save_state.once(3, tickerSaveState);
  }
  if (_updateSegState) {
    State.mode = SET;
    if(save_seg_state.active()) save_seg_state.detach();
    save_seg_state.once(3, tickerSaveSegmentState);
  }
#endif
  _updateState = false;
  _updateSegState = false;
});
