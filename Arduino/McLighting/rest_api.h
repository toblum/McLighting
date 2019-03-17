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
    const size_t bufferSize = JSON_OBJECT_SIZE(31) + 1500;
    DynamicJsonDocument jsonBuffer(bufferSize);
    JsonObject json = jsonBuffer.to<JsonObject>();
    json["HOSTNAME"] = HOSTNAME;
    json["version"] = SKETCH_VERSION;
    json["heap"] = ESP.getFreeHeap();
    json["sketch_size"] = ESP.getSketchSize();
    json["free_sketch_space"] = ESP.getFreeSketchSpace();
    json["flash_chip_size"] = ESP.getFlashChipSize();
    json["flash_chip_real_size"] = ESP.getFlashChipRealSize();
    json["flash_chip_speed"] = ESP.getFlashChipSpeed();
    json["sdk_version"] = ESP.getSdkVersion();
    json["core_version"] = ESP.getCoreVersion();
    json["cpu_freq"] = ESP.getCpuFreqMHz();
    json["chip_id"] = ESP.getFlashChipId();
    #if defined(USE_WS2812FX_DMA)
      #if USE_WS2812FX_DMA == 0
        json["animation_lib"] = "WS2812FX_DMA";
      #endif
      #if USE_WS2812FX_DMA == 1
        json["animation_lib"] = "WS2812FX_UART1";
      #endif
      #if USE_WS2812FX_DMA == 2
        json["animation_lib"] = "WS2812FX_UART2";
      #endif
    #else
      json["animation_lib"] = "WS2812FX";
    #endif
    json["ws2812_pin"]  = WS2812FXStripSettings.pin;
    json["led_count"] = WS2812FXStripSettings.stripSize;
    json["rgb_order"] = WS2812FXStripSettings.RGBOrder;
    if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
      json["rgbw_mode"] = "ON";
    } else {
      json["rgbw_mode"] = "OFF";
    }
    #if defined(ENABLE_BUTTON)
      json["button_mode"] = "ON";
      json["button_pin"] = ENABLE_BUTTON;
    #else
      json["button_mode"] = "OFF";
    #endif
    #if defined(ENABLE_BUTTON_GY33)
      json["button_gy33"] = "ON";
      json["gy33_pin"] = ENABLE_BUTTON_GY33;
    #else
      json["button_gy33"] = "OFF";
    #endif
    #if defined(ENABLE_REMOTE)
      json["ir_remote"] = "ON";
      json["tsop_ir_pin"] = ENABLE_REMOTE;
    #else
      json["ir_remote"] = "OFF";
    #endif
    #if defined(ENABLE_MQTT)
      #if ENABLE_MQTT == 0
        json["mqtt"] = "MQTT";
      #endif
      #if ENABLE_MQTT == 1
        json["mqtt"] = "AMQTT";
      #endif
    #else
      json["mqtt"] = "OFF";
    #endif
    #if defined(ENABLE_HOMEASSISTANT)
      json["home_assistant"] = "ON";
    #else
      json["home_assistant"] = "OFF";
    #endif
    #if defined(ENABLE_LEGACY_ANIMATIONS)
      json["legacy_animations"] = "ON";
    #else
      json["legacy_animations"] = "OFF";
    #endif
    #if defined(ENABLE_TV)
      json["tv_animation"] = "ON";
    #else
      json["tv_animation"] = "OFF";
    #endif
    #if defined(ENABLE_E131)
      json["e131_animations"] = "ON";
    #else
      json["e131_animations"] = "OFF";
    #endif
    #if defined(ENABLE_OTA)
      #if ENABLE_OTA == 0
        json["ota"] = "ARDUINO";
      #endif
      #if ENABLE_OTA == 1
        json["ota"] = "HTTP";
      #endif
    #else
      json["ota"] = "OFF";
    #endif
    #if defined(ENABLE_STATE_SAVE)
      #if ENABLE_STATE_SAVE == 1
        json["state_save"] = "SPIFFS";
      #endif
      #if ENABLE_STATE_SAVE == 0
        json["state_save"] = "EEPROM";
      #endif
    #else
      json["state_save"] = "OFF";
    #endif
    
    String json_str;
    serializeJson(json, json_str);
    jsonBuffer.clear();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json_str);
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
    server.send(200, "text/plain", "Formatting SPIFFS ..." );
    SPIFFS.format();
  });

  server.on("/set_brightness", []() {
    getArgs();
    mode = SET_BRIGHTNESS;    
    getStatusJSON();
  });

  server.on("/get_brightness", []() {
    char str_brightness[4];
    snprintf(str_brightness, sizeof(str_brightness), "%i", (int) (brightness / 2.55));
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", str_brightness );
    DBG_OUTPUT_PORT.printf("/get_brightness: %i\r\n", (int) (brightness / 2.55));
  });

  server.on("/set_speed", []() {
    getArgs();
    mode = SET_SPEED;
    getStatusJSON();
  });

  server.on("/get_speed", []() {
    char str_speed[4];
    snprintf(str_speed, sizeof(str_speed), "%i", ws2812fx_speed);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", str_speed );
    DBG_OUTPUT_PORT.printf("/get_speed: %i\r\n", ws2812fx_speed);
  });

  server.on("/get_switch", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", (mode == OFF) ? "0" : "1" );
    DBG_OUTPUT_PORT.printf("/get_switch: %s\r\n", (mode == OFF) ? "0" : "1");
  });

  server.on("/get_color", []() {
    char rgbcolor[10];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", main_color.white, main_color.red, main_color.green, main_color.blue);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });
  
  server.on("/get_color2", []() {
    char rgbcolor[10];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", back_color.white, back_color.red, back_color.green, back_color.blue);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color2: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });

  server.on("/get_color3", []() {
    char rgbcolor[10];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", xtra_color.white, xtra_color.red, xtra_color.green, xtra_color.blue);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color3: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });


  server.on("/status", []() {
    getStatusJSON();
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

    bool updateFSE = false;
    if(server.hasArg("ws_cnt")){
      uint16_t pixelCt = server.arg("ws_cnt").toInt();
      if (pixelCt > 0) {
        WS2812FXStripSettings.stripSize = pixelCt;
        updateFSE = true;
      }
    }
    if(server.hasArg("ws_rgbo")){
      char tmp_rgbOrder[5];
      snprintf(tmp_rgbOrder, sizeof(tmp_rgbOrder), "%s", server.arg("ws_rgbo").c_str());
      checkRGBOrder(tmp_rgbOrder);
      updateFSE = true;
    }
    
#if !defined(USE_WS2812FX_DMA)    
    if(server.hasArg("wspin")){
      if (checkPin(server.arg("wspin").toInt()) {
        updateFSE = true;
        DBG_OUTPUT_PORT.println(WS2812FXStripSettings.pin);
      } else {
        DBG_OUTPUT_PORT.println("invalid input!");
      }
    }
#endif
    
    if(server.hasArg("ws_fxopt")){
      WS2812FXStripSettings.fxoptions = server.arg("ws_fxopt").toInt();
      updateFSE = true;
    }

    if(updateFSE) {
      mode = INIT_STRIP;
    }
    
    if(server.hasArg("hostname")){
      snprintf(HOSTNAME, sizeof(HOSTNAME), "%s", server.arg("hostname").c_str());
      updateFSE = true;
    }
    
#if defined(ENABLE_MQTT)   
    if(server.hasArg("mqtt_host")){
      snprintf(mqtt_host, sizeof(mqtt_host), "%s", server.arg("mqtt_host").c_str());
      updateFSE = true;
    }
    if(server.hasArg("mqtt_port")){
      if ((server.arg("mqtt_port").toInt() >= 0) && (server.arg("mqtt_port").toInt() <=65535)) {
        mqtt_port = server.arg("mqttport").toInt();
        updateFSE = true;
      }    
    }
    if(server.hasArg("mqtt_user")){
      snprintf(mqtt_user, sizeof(mqtt_user), "%s", server.arg("mqtt_user").c_str());
      updateFSE = true;
    }
    if(server.hasArg("mqtt_pass")){
      snprintf(mqtt_pass, sizeof(mqtt_pass), "%s", server.arg("mqtt_pass").c_str());
      updateFSE = true;
    } 
#endif

#if defined(ENABLE_STATE_SAVE)
  #if ENABLE_STATE_SAVE == 1  
    (writeConfigFS(updateFSE)) ? DBG_OUTPUT_PORT.println("Config FS Save success!"): DBG_OUTPUT_PORT.println("Config FS Save failure!");
  #endif
  #if ENABLE_STATE_SAVE == 0 
    if (updateFSE) {
      char last_conf[223];
    #if defined(ENABLE_MQTT)
      snprintf(last_conf, sizeof(last_conf), "CNF|%64s|%64s|%5d|%32s|%32s|%4d|%2d|%4s|%3d", HOSTNAME, mqtt_host, mqtt_port, mqtt_user, mqtt_pass, WS2812FXStripSettings.stripSize, WS2812FXStripSettings.pin, WS2812FXStripSettings.RGBOrder, WS2812FXStripSettings.fxoptions);
    #else
      snprintf(last_conf, sizeof(last_conf), "CNF|%64s|%64s|%5d|%32s|%32s|%4d|%2d|%4s|%3d", HOSTNAME, "", "", "", "", WS2812FXStripSettings.stripSize, WS2812FXStripSettings.pin, WS2812FXStripSettings.RGBOrder, WS2812FXStripSettings.fxoptions);
    #endif
      writeEEPROM(0, 222, last_conf);
      EEPROM.commit();
    }
  #endif
#endif
    getConfigJSON();
    delay(500);
  
#if defined(ENABLE_MQTT)
    if (updateFSE) {
      initMqtt();
    }  
#endif

    updateFSE = false;
  });
  
  server.on("/off", []() {
    mode = OFF;
    getStatusJSON();
  });

    server.on("/auto", []() {
    mode = AUTO;
    getStatusJSON();
  });

  server.on("/all", []() {
    getArgs();
    ws2812fx_mode = FX_MODE_STATIC;
    mode = SET_ALL;
    getStatusJSON();
  });

  #if defined(ENABLE_LEGACY_ANIMATIONS)
    server.on("/wipe", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_COLOR_WIPE;
      mode = SET_ALL;
      getStatusJSON();
    });
  
    server.on("/rainbow", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_RAINBOW;
      mode = SET_ALL;
      getStatusJSON();
    });
  
    server.on("/rainbowcycle", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_RAINBOW_CYCLE;
      mode = SET_ALL;
      getStatusJSON();
    });
  
    server.on("/theaterchase", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_THEATER_CHASE;
      mode = SET_ALL;
      getStatusJSON();
    });
  
    server.on("/twinklerandom", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_TWINKLE_RANDOM;
      mode = SET_ALL;
      getStatusJSON();
    });
    
    server.on("/theaterchaserainbow", []() {
      getArgs();
      ws2812fx_mode = FX_MODE_THEATER_CHASE_RAINBOW;
      mode = SET_ALL;
      getStatusJSON();
    });
  #endif
  
  #if defined(ENABLE_E131)
    server.on("/e131", []() {
      mode = E131;
      getStatusJSON();
    });
  #endif
  
  #if defined(ENABLE_TV)
    server.on("/tv", []() {
      mode = TV;
      getStatusJSON();
    });
  #endif

  server.on("/get_modes", []() {
    getModesJSON();
  });

  server.on("/set_mode", []() {
    getArgs();
    mode = SET_MODE;
    getStatusJSON();
  });
