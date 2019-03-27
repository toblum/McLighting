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
    root["ws2812_pin"]  = WS2812FXStripSettings.pin;
    root["led_count"] = WS2812FXStripSettings.stripSize;
    root["rgb_order"] = WS2812FXStripSettings.RGBOrder;
    if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
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
    #if defined(ENABLE_LEGACY_ANIMATIONS)
      root["legacy_animations"] = "ON";
    #else
      root["legacy_animations"] = "OFF";
    #endif
    #if defined(ENABLE_TV)
      root["tv_animation"] = "ON";
    #else
      root["tv_animation"] = "OFF";
    #endif
    #if defined(ENABLE_E131)
      root["e131_animations"] = "ON";
    #else
      root["e131_animations"] = "OFF";
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
      #if ENABLE_STATE_SAVE == 1
        root["state_save"] = "SPIFFS";
      #endif
      #if ENABLE_STATE_SAVE == 0
        root["state_save"] = "EEPROM";
      #endif
    #else
      root["state_save"] = "OFF";
    #endif
    uint16_t msg_len = measureJson(root) + 1;
    char * buffer = (char *) malloc(msg_len);
    serializeJson(root, buffer, msg_len);
    jsonBuffer.clear();
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", buffer);
    free (buffer);
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
    mode = SET_BRIGHTNESS;
    getArgs();   
    getStatusJSON();
  });

  server.on("/get_brightness", []() {
    char str_brightness[4];
    snprintf(str_brightness, sizeof(str_brightness), "%i", (int) (brightness / 2.55));
    str_brightness[sizeof(str_brightness) - 1] = 0x00;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", str_brightness );
    DBG_OUTPUT_PORT.printf("/get_brightness: %i\r\n", (int) (brightness / 2.55));
  });

  server.on("/set_speed", []() {
    mode = SET_SPEED;
    getArgs();
    getStatusJSON();
  });

  server.on("/get_speed", []() {
    char str_speed[4];
    snprintf(str_speed, sizeof(str_speed), "%i", ws2812fx_speed);
    str_speed[sizeof(str_speed) - 1] = 0x00;
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
    rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });
  
  server.on("/get_color2", []() {
    char rgbcolor[10];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", back_color.white, back_color.red, back_color.green, back_color.blue);
    rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color2: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });

  server.on("/get_color3", []() {
    char rgbcolor[10];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X%02X", xtra_color.white, xtra_color.red, xtra_color.green, xtra_color.blue);
    rgbcolor[sizeof(rgbcolor) - 1] = 0x00;
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

    bool updateStrip = false;
    bool updateConf  = false;
    if(server.hasArg("ws_cnt")){
      uint16_t pixelCt = server.arg("ws_cnt").toInt();
      if (pixelCt > 0) {
        WS2812FXStripSettings.stripSize = constrain(pixelCt, 1, MAXLEDS);
        updateStrip = true;
      }
    }
    if(server.hasArg("ws_rgbo")){
      char tmp_rgbOrder[5];
      snprintf(tmp_rgbOrder, sizeof(tmp_rgbOrder), "%s", server.arg("ws_rgbo").c_str());
      tmp_rgbOrder[sizeof(tmp_rgbOrder) - 1] = 0x00;
      checkRGBOrder(tmp_rgbOrder);
      updateStrip = true;
      updateConf = true;
    }
    
#if !defined(USE_WS2812FX_DMA)    
    if(server.hasArg("ws_pin")){
      if (checkPin(server.arg("ws_pin").toInt())) {
        updateStrip = true;
        DBG_OUTPUT_PORT.println(WS2812FXStripSettings.pin);
      } else {
        DBG_OUTPUT_PORT.println("invalid input!");
      }
    }
#endif
    
    if(server.hasArg("ws_fxopt")){
      WS2812FXStripSettings.fxoptions = ((constrain(server.arg("ws_fxopt").toInt(), 0, 255)>>1)<<1);
      updateStrip = true;
    }

    if(updateStrip) {
      mode = INIT_STRIP;
    }
    
    if(server.hasArg("hostname")){
      snprintf(HOSTNAME, sizeof(HOSTNAME), "%s", server.arg("hostname").c_str());
      HOSTNAME[sizeof(HOSTNAME) - 1] = 0x00;
      updateConf = true;
    }
    
#if defined(ENABLE_MQTT)   
    if(server.hasArg("mqtt_host")){
      snprintf(mqtt_host, sizeof(mqtt_host), "%s", server.arg("mqtt_host").c_str());
      mqtt_host[sizeof(mqtt_host) - 1] = 0x00;
      updateConf = true;
    }
    if(server.hasArg("mqtt_port")){
      if ((server.arg("mqtt_port").toInt() >= 0) && (server.arg("mqtt_port").toInt() <=65535)) {
        mqtt_port = server.arg("mqttport").toInt();
        updateConf = true;
      }    
    }
    if(server.hasArg("mqtt_user")){
      snprintf(mqtt_user, sizeof(mqtt_user), "%s", server.arg("mqtt_user").c_str());
      mqtt_user[sizeof(mqtt_user) - 1] = 0x00;
      updateConf = true;
    }
    if(server.hasArg("mqtt_pass")){
      snprintf(mqtt_pass, sizeof(mqtt_pass), "%s", server.arg("mqtt_pass").c_str());
      mqtt_pass[sizeof(mqtt_pass) - 1] = 0x00;
      updateConf = true;
    }
    if (updateConf) {
      initMqtt();
    }  
#endif

#if defined(ENABLE_STATE_SAVE)
    if (updateStrip || updateConf) {
      if(!settings_save_conf.active()) settings_save_conf.once(3, tickerSaveConfig);
    }
#endif
    updateStrip = false;
    updateConf = false;
    getConfigJSON();
  });
  
  server.on("/off", []() {
    mode = OFF;
    getStatusJSON();
  });

    server.on("/auto", []() {
    mode = AUTO;
    getArgs();
    getStatusJSON();
  });

  server.on("/all", []() {
    ws2812fx_mode = FX_MODE_STATIC;
    mode = SET_ALL;
    getArgs();
    getStatusJSON();
  });

  #if defined(ENABLE_LEGACY_ANIMATIONS)
    server.on("/wipe", []() {
      ws2812fx_mode = FX_MODE_COLOR_WIPE;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
  
    server.on("/rainbow", []() {
      ws2812fx_mode = FX_MODE_RAINBOW;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
  
    server.on("/rainbowcycle", []() {
      ws2812fx_mode = FX_MODE_RAINBOW_CYCLE;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
  
    server.on("/theaterchase", []() {
      ws2812fx_mode = FX_MODE_THEATER_CHASE;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
  
    server.on("/twinklerandom", []() {
      ws2812fx_mode = FX_MODE_TWINKLE_RANDOM;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
    
    server.on("/theaterchaserainbow", []() {
      ws2812fx_mode = FX_MODE_THEATER_CHASE_RAINBOW;
      mode = SET_ALL;
      getArgs();
      getStatusJSON();
    });
  #endif
  
  #if defined(ENABLE_E131)
    server.on("/e131", []() {
      mode = E131;
      getArgs();
      getStatusJSON();
    });
  #endif
  
  #if defined(ENABLE_TV)
    server.on("/tv", []() {
      mode = TV;
      getArgs();
      getStatusJSON();
    });
  #endif

  server.on("/get_modes", []() {
    getModesJSON();
  });

  server.on("/set_mode", []() {
    mode = SET_MODE;
    getArgs();
    getStatusJSON();
  });
