#if defined(ENABLE_STATE_SAVE)
  
  Ticker save_state;
  Ticker save_seg_state;
  Ticker save_conf;

  bool updateState  = false;
  bool updateSegState  = false;

  void tickerSaveState(){
    updateState = true;
  }

  void tickerSaveConfig(){
    updateConfig = true;
  }

  void tickerSaveSegmentState(){
    updateSegState = true;
  }
  
  // Write configuration to FS JSON
  bool writeConfigFS(bool save){
    if (save) {
      //FS save
      DBG_OUTPUT_PORT.println("Saving config: ");    
      File configFile = SPIFFS.open("/config.json", "w");
      if (!configFile) {
        DBG_OUTPUT_PORT.println("Failed!");
        save_conf.detach();
        updateConfig = false;
        return false;
      }
      DBG_OUTPUT_PORT.println(listConfigJSON());
      configFile.print(listConfigJSON());
      configFile.close();
      save_conf.detach();
      updateConfig = false;
      return true;
      //end save
    } else {
      DBG_OUTPUT_PORT.println("SaveConfig is false!");
      return false;
    }
  }
 
  // Read search_str to FS
  bool readConfigFS() {
    //read configuration from FS JSON
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      DBG_OUTPUT_PORT.print("Reading config file... ");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        DBG_OUTPUT_PORT.println("Opened!");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();
     #if defined(ENABLE_MQTT)
        const size_t bufferSize = JSON_OBJECT_SIZE(11) + 150;
     #else
        const size_t bufferSize = JSON_OBJECT_SIZE(7) + 100;
     #endif
        DynamicJsonDocument jsonBuffer(bufferSize);
        DeserializationError error = deserializeJson(jsonBuffer, buf.get());
        DBG_OUTPUT_PORT.print("Config: ");
        if (!error) {
          DBG_OUTPUT_PORT.println("Parsed!");
          JsonObject root = jsonBuffer.as<JsonObject>();
          serializeJson(root, DBG_OUTPUT_PORT);
          DBG_OUTPUT_PORT.println("");
          strcpy(HOSTNAME, root["hostname"]);
        #if defined(ENABLE_MQTT)
          strcpy(mqtt_host, root["mqtt_host"]);
          mqtt_port = root["mqtt_port"].as<uint16_t>();
          strcpy(mqtt_user, root["mqtt_user"]);
          strcpy(mqtt_pass, root["mqtt_pass"]);
        #endif
          Config.segments = constrain(root["ws_seg"].as<uint8_t>(), 1, MAX_NUM_SEGMENTS - 1);
          Config.stripSize = constrain(root["ws_cnt"].as<uint16_t>(), 1, MAXLEDS);
          char _rgbOrder[5];
          strcpy(_rgbOrder, root["ws_rgbo"]);
          checkRGBOrder(_rgbOrder);
          uint8_t temp_pin;
          checkPin((uint8_t) root["ws_pin"]);
          Config.transEffect = root["ws_trans"].as<bool>();
          jsonBuffer.clear();
          return true;
        } else {
          DBG_OUTPUT_PORT.print("Failed to load json config: ");
          DBG_OUTPUT_PORT.println(error.c_str());
          jsonBuffer.clear();
        }
      } else {
        DBG_OUTPUT_PORT.println("Failed to open /config.json");
      }
    } else {
      DBG_OUTPUT_PORT.println("Coudnt find config.json");
      writeConfigFS(true);
    }
    //end read
    return false;
  }
  
  bool writeStateFS(bool save){
    if (save) {
      //save the strip state to FS JSON
      DBG_OUTPUT_PORT.print("Saving state: ");
      //SPIFFS.remove("/stripstate.json") ? DBG_OUTPUT_PORT.println("removed file") : DBG_OUTPUT_PORT.println("failed removing file");
      File configFile = SPIFFS.open("/stripstate.json", "w");
      if (!configFile) {
        DBG_OUTPUT_PORT.println("Failed!");
        save_state.detach();
        updateState = false;
        return false;
      }
      DBG_OUTPUT_PORT.println(listStateJSON());
      configFile.print(listStateJSON());
      configFile.close();
      char filename[28];
      save_state.detach();
      updateState = false;
      return true;
      //end save
    } else {
      DBG_OUTPUT_PORT.println("SaveState is false!");
      return false;
    }
  }
    
  bool readStateFS() {
    //read strip state from FS JSON
    if (SPIFFS.exists("/stripstate.json")) {
      //file exists, reading and loading
      DBG_OUTPUT_PORT.print("Reading state file... ");
      File configFile = SPIFFS.open("/stripstate.json", "r");
      if (configFile) {
        DBG_OUTPUT_PORT.println("Opened!");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();
        const size_t bufferSize = JSON_OBJECT_SIZE(3) + 50;
        DynamicJsonDocument jsonBuffer(bufferSize);
        DeserializationError error = deserializeJson(jsonBuffer, buf.get());
        DBG_OUTPUT_PORT.print("Config: ");
        if (!error) {
          DBG_OUTPUT_PORT.print("Parsed");
          JsonObject root = jsonBuffer.as<JsonObject>();
          serializeJson(root, DBG_OUTPUT_PORT);
          DBG_OUTPUT_PORT.println("");
          State.segment = root["segment"];
          State.mode = static_cast<MODE>(root["mode"].as<uint8_t>());
          State.brightness =  root["brightness"];
          jsonBuffer.clear();
          return true;
        } else {
          DBG_OUTPUT_PORT.print("Failed to load json config: ");
          DBG_OUTPUT_PORT.println(error.c_str());
          jsonBuffer.clear();
        }
      } else {
        DBG_OUTPUT_PORT.println("Failed to open \"/stripstate.json\"");
      }
    } else {
      DBG_OUTPUT_PORT.println("Couldn't find \"/stripstate.json\"");
      writeStateFS(true);
    }
    //end read
    return false;
  }
  
  bool writeSegmentStateFS(bool save, uint8_t seg){
    if (save) {
      //save the segment state to FS JSON
      DBG_OUTPUT_PORT.print("Saving segment state: ");
      char filename[28];
      snprintf(filename, 28, "/stripstate_segment_%02i.json", seg);
      filename[27] = 0x00;
      File configFile = SPIFFS.open(filename, "w");
      if (!configFile) {
        DBG_OUTPUT_PORT.println("Failed!");
        save_seg_state.detach();
        updateSegState = false;
        return false;
      }
      DBG_OUTPUT_PORT.println(listSegmentStateJSON(seg));
      configFile.print(listSegmentStateJSON(seg));
      configFile.close();
      save_seg_state.detach();
      updateSegState = false;
      return true;
      //end save
    } else {
      DBG_OUTPUT_PORT.println("SaveSegmentState is false!");
      return false;
    }
  }
 
  bool readSegmentStateFS(uint8_t _seg) {
    //read strip state from FS JSON
    char filename[28];
    snprintf(filename, 28, "/stripstate_segment_%02i.json", _seg);
    filename[27] = 0x00;
    if (SPIFFS.exists(filename)) {
      //file exists, reading and loading
      DBG_OUTPUT_PORT.printf("Reading segmentstate file: %s\r\n", filename);
      File configFile = SPIFFS.open(filename, "r");
      if (configFile) {
        DBG_OUTPUT_PORT.println("Opened!");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();
        const size_t bufferSize = JSON_ARRAY_SIZE(12) + JSON_OBJECT_SIZE(7) + 100;
        DynamicJsonDocument jsonBuffer(bufferSize);
        DeserializationError error = deserializeJson(jsonBuffer, buf.get());
        DBG_OUTPUT_PORT.print("Config: ");
        if (!error) {
          DBG_OUTPUT_PORT.print("Parsed");
          JsonObject root = jsonBuffer.as<JsonObject>();
          serializeJson(root, DBG_OUTPUT_PORT);
          DBG_OUTPUT_PORT.println("");
          segState.start = constrain(root["start"].as<uint16_t>(), 0, Config.stripSize - 1) ;
          segState.stop  = constrain(root["stop"].as<uint16_t>(), 0, Config.stripSize - 1);
          segState.mode[_seg]  = root["fx_mode"].as<uint8_t>();
          segState.speed[_seg] = root["speed"].as<uint8_t>();
          main_color.white     = root["color"][0].as<uint8_t>();
          main_color.red       = root["color"][1].as<uint8_t>();
          main_color.green     = root["color"][2].as<uint8_t>();
          main_color.blue      = root["color"][3].as<uint8_t>();
          back_color.white     = root["color"][4].as<uint8_t>();
          back_color.red       = root["color"][5].as<uint8_t>();
          back_color.green     = root["color"][6].as<uint8_t>();
          back_color.blue      = root["color"][7].as<uint8_t>();
          xtra_color.white     = root["color"][8].as<uint8_t>();
          xtra_color.red       = root["color"][9].as<uint8_t>();
          xtra_color.green     = root["color"][10].as<uint8_t>();
          xtra_color.blue      = root["color"][11].as<uint8_t>();
          segState.options = constrain(root["ws_fxopt"].as<uint8_t>(), 0, 255) & 0xFE;
          convertColors();
          jsonBuffer.clear();
          return true;
        } else {
          DBG_OUTPUT_PORT.print("Failed to load json config: ");
          DBG_OUTPUT_PORT.println(error.c_str());
          jsonBuffer.clear();
        }
      } else {
        DBG_OUTPUT_PORT.printf("Failed to open \"/%s\"\r\n", filename);
      }
    } else {
      DBG_OUTPUT_PORT.printf("Couldn't find \"/%s\"", filename);
      writeSegmentStateFS(true, _seg);
    }
    //end read
    return false;
  }
#endif
