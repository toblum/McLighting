#include "definitions.h"

// ***************************************************************************
// Load libraries for: WebServer / WiFiManager / WebSockets
// ***************************************************************************
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// needed for library WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <EEPROM.h>

#include <WebSockets.h>           //https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsServer.h>

// OTA
#ifdef ENABLE_OTA
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

//SPIFFS Save
#if !defined(ENABLE_HOMEASSISTANT) and defined(ENABLE_STATE_SAVE_SPIFFS)
  #include <ArduinoJson.h>        //https://github.com/bblanchon/ArduinoJson
#endif

// MQTT
#ifdef ENABLE_MQTT
  #include <PubSubClient.h>
  #ifdef ENABLE_HOMEASSISTANT
    #include <ArduinoJson.h>     //https://github.com/bblanchon/ArduinoJson
  #endif

  WiFiClient espClient;
  PubSubClient mqtt_client(espClient);
#endif

#ifdef ENABLE_AMQTT
  #include <AsyncMqttClient.h>    //https://github.com/marvinroger/async-mqtt-client
                                  //https://github.com/me-no-dev/ESPAsyncTCP
  #ifdef ENABLE_HOMEASSISTANT
    #include <ArduinoJson.h>
  #endif

  AsyncMqttClient amqttClient;
  WiFiEventHandler wifiConnectHandler;
  WiFiEventHandler wifiDisconnectHandler;
#endif


// ***************************************************************************
// Instanciate HTTP(80) / WebSockets(81) Server
// ***************************************************************************
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

#ifdef HTTP_OTA
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
#endif

#ifdef USE_NEOANIMATIONFX
// ***************************************************************************
// Load libraries / Instanciate NeoAnimationFX library
// ***************************************************************************
// https://github.com/debsahu/NeoAnimationFX
#include <NeoAnimationFX.h>
#define NEOMETHOD NeoPBBGRB800

NEOMETHOD neoStrip(NUMLEDS);
NeoAnimationFX<NEOMETHOD> strip(neoStrip);

// Uses Pin RX / GPIO3 (Only pin that is supported, due to hardware limitations)
// NEOMETHOD NeoPBBGRB800 uses GRB config 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEOMETHOD NeoPBBGRB400 uses GRB config 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// NEOMETHOD NeoPBBRGB800 uses RGB config 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEOMETHOD NeoPBBRGB400 uses RGB config 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)

// Uses Pin D4 / GPIO2 (Only pin that is supported, due to hardware limitations)
// NEOMETHOD NeoPBBGRBU800 uses GRB config 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEOMETHOD NeoPBBGRBU400 uses GRB config 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
// NEOMETHOD NeoPBBRGBU800 uses RGB config 800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
// NEOMETHOD NeoPBBRGBU400 uses RGB config 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)

#endif

#ifdef USE_WS2812FX
// ***************************************************************************
// Load libraries / Instanciate WS2812FX library
// ***************************************************************************
// https://github.com/kitesurfer1404/WS2812FX
#include <WS2812FX.h>
WS2812FX strip = WS2812FX(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
#endif

// ***************************************************************************
// Load library "ticker" for blinking status led
// ***************************************************************************
#include <Ticker.h>
Ticker ticker;
#ifdef ENABLE_HOMEASSISTANT
  Ticker ha_send_data;
#endif
#ifdef ENABLE_AMQTT
  Ticker mqttReconnectTimer;
  Ticker wifiReconnectTimer;
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
  Ticker spiffs_save_state;
#endif
void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

#ifdef ENABLE_STATE_SAVE_EEPROM
  // ***************************************************************************
  // EEPROM helper
  // ***************************************************************************
  String readEEPROM(int offset, int len) {
    String res = "";
    for (int i = 0; i < len; ++i)
    {
      res += char(EEPROM.read(i + offset));
      //DBG_OUTPUT_PORT.println(char(EEPROM.read(i + offset)));
    }
    DBG_OUTPUT_PORT.printf("readEEPROM(): %s\n", res.c_str());
    return res;
  }
  
  void writeEEPROM(int offset, int len, String value) {
    DBG_OUTPUT_PORT.printf("writeEEPROM(): %s\n", value.c_str());
    for (int i = 0; i < len; ++i)
    {
      if (i < value.length()) {
        EEPROM.write(i + offset, value[i]);
      } else {
        EEPROM.write(i + offset, NULL);
      }
    }
  }
#endif

// ***************************************************************************
// Saved state handling
// ***************************************************************************
// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, int index)
{
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

// ***************************************************************************
// Callback for WiFiManager library when config mode is entered
// ***************************************************************************
//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  DBG_OUTPUT_PORT.println("Entered config mode");
  DBG_OUTPUT_PORT.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DBG_OUTPUT_PORT.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);

  uint16_t i;
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 255);
  }
  strip.show();
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  DBG_OUTPUT_PORT.println("Should save config");
  shouldSaveConfig = true;
}

// ***************************************************************************
// Include: Webserver
// ***************************************************************************
#include "spiffs_webserver.h"

// ***************************************************************************
// Include: Request handlers
// ***************************************************************************
#include "request_handlers.h"

// ***************************************************************************
// Include: Color modes
// ***************************************************************************
#ifdef ENABLE_LEGACY_ANIMATIONS
  #include "colormodes.h"
#endif

// ***************************************************************************
// MAIN
// ***************************************************************************
void setup() {
//  system_update_cpu_freq(160);

  DBG_OUTPUT_PORT.begin(115200);
  EEPROM.begin(512);

  // set builtin led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // button pin setup
#ifdef ENABLE_BUTTON
  pinMode(BUTTON,INPUT_PULLUP);
#endif
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.5, tick);

  // ***************************************************************************
  // Setup: SPIFFS
  // ***************************************************************************
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    DBG_OUTPUT_PORT.printf("FS Usage: %d/%d bytes\n\n", fs_info.usedBytes, fs_info.totalBytes);
  }

  wifi_station_set_hostname(const_cast<char*>(HOSTNAME));

  // ***************************************************************************
  // Setup: Neopixel
  // ***************************************************************************
  strip.init();
  strip.setBrightness(brightness);
  strip.setSpeed(convertSpeed(ws2812fx_speed));
  //strip.setMode(FX_MODE_RAINBOW_CYCLE);
  strip.setColor(main_color.red, main_color.green, main_color.blue);
  strip.start();

  // ***************************************************************************
  // Setup: WiFiManager
  // ***************************************************************************
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  #if defined(ENABLE_MQTT) or defined(ENABLE_AMQTT)
    #if defined(ENABLE_STATE_SAVE_SPIFFS) and (defined(ENABLE_MQTT) or defined(ENABLE_AMQTT))
      (readConfigFS()) ? DBG_OUTPUT_PORT.println("WiFiManager config FS Read success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Read failure!");
    #else
      String settings_available = readEEPROM(134, 1);
      if (settings_available == "1") {
        readEEPROM(0, 64).toCharArray(mqtt_host, 64);   // 0-63
        readEEPROM(64, 6).toCharArray(mqtt_port, 6);    // 64-69
        readEEPROM(70, 32).toCharArray(mqtt_user, 32);  // 70-101
        readEEPROM(102, 32).toCharArray(mqtt_pass, 32); // 102-133
        DBG_OUTPUT_PORT.printf("MQTT host: %s\n", mqtt_host);
        DBG_OUTPUT_PORT.printf("MQTT port: %s\n", mqtt_port);
        DBG_OUTPUT_PORT.printf("MQTT user: %s\n", mqtt_user);
        DBG_OUTPUT_PORT.printf("MQTT pass: %s\n", mqtt_pass);
      }
    #endif
    WiFiManagerParameter custom_mqtt_host("host", "MQTT hostname", mqtt_host, 64);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 32);
    WiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32);
  #endif

  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  #if defined(ENABLE_MQTT) or defined(ENABLE_AMQTT)
    //set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    //add all your parameters here
    wifiManager.addParameter(&custom_mqtt_host);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
  #endif

  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(HOSTNAME)) {
    DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  #if defined(ENABLE_MQTT) or defined(ENABLE_AMQTT)
    //read updated parameters
    strcpy(mqtt_host, custom_mqtt_host.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pass, custom_mqtt_pass.getValue());

    //save the custom parameters to FS
    #if defined(ENABLE_STATE_SAVE_SPIFFS) and (defined(ENABLE_MQTT) or defined(ENABLE_AMQTT))
      (writeConfigFS(shouldSaveConfig)) ? DBG_OUTPUT_PORT.println("WiFiManager config FS Save success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Save failure!");
    #else if defined(ENABLE_STATE_SAVE_EEPROM)
      if (shouldSaveConfig) {
        DBG_OUTPUT_PORT.println("Saving WiFiManager config");

        writeEEPROM(0, 64, mqtt_host);   // 0-63
        writeEEPROM(64, 6, mqtt_port);   // 64-69
        writeEEPROM(70, 32, mqtt_user);  // 70-101
        writeEEPROM(102, 32, mqtt_pass); // 102-133
        writeEEPROM(134, 1, "1");        // 134 --> always "1"
        EEPROM.commit();
      }
    #endif
  #endif
  
  #ifdef ENABLE_AMQTT
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  #endif

  //if you get here you have connected to the WiFi
  DBG_OUTPUT_PORT.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);


  // ***************************************************************************
  // Configure OTA
  // ***************************************************************************
  #ifdef ENABLE_OTA
    DBG_OUTPUT_PORT.println("Arduino OTA activated.");

    // Port defaults to 8266
    ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    ArduinoOTA.setHostname(HOSTNAME);

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
      DBG_OUTPUT_PORT.println("Arduino OTA: Start updating");
    });
    ArduinoOTA.onEnd([]() {
      DBG_OUTPUT_PORT.println("Arduino OTA: End");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      DBG_OUTPUT_PORT.printf("Arduino OTA Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      DBG_OUTPUT_PORT.printf("Arduino OTA Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DBG_OUTPUT_PORT.println("Arduino OTA: Auth Failed");
      else if (error == OTA_BEGIN_ERROR) DBG_OUTPUT_PORT.println("Arduino OTA: Begin Failed");
      else if (error == OTA_CONNECT_ERROR) DBG_OUTPUT_PORT.println("Arduino OTA: Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) DBG_OUTPUT_PORT.println("Arduino OTA: Receive Failed");
      else if (error == OTA_END_ERROR) DBG_OUTPUT_PORT.println("Arduino OTA: End Failed");
    });

    ArduinoOTA.begin();
    DBG_OUTPUT_PORT.println("");
  #endif


  // ***************************************************************************
  // Configure MQTT
  // ***************************************************************************
  #ifdef ENABLE_MQTT
    if (mqtt_host != "" && atoi(mqtt_port) > 0) {
      snprintf(mqtt_intopic, sizeof mqtt_intopic, "%s/in", HOSTNAME);
      snprintf(mqtt_outtopic, sizeof mqtt_outtopic, "%s/out", HOSTNAME);

      DBG_OUTPUT_PORT.printf("MQTT active: %s:%d\n", mqtt_host, String(mqtt_port).toInt());

      mqtt_client.setServer(mqtt_host, atoi(mqtt_port));
      mqtt_client.setCallback(mqtt_callback);
    }
  #endif

  #ifdef ENABLE_AMQTT
    if (mqtt_host != "" && atoi(mqtt_port) > 0) {
      amqttClient.onConnect(onMqttConnect);
      amqttClient.onDisconnect(onMqttDisconnect);
      amqttClient.onMessage(onMqttMessage);
      amqttClient.setServer(mqtt_host, atoi(mqtt_port));
      if (mqtt_user != "" or mqtt_pass != "") amqttClient.setCredentials(mqtt_user, mqtt_pass);
      amqttClient.setClientId(mqtt_clientid);

      connectToMqtt();
    }
  #endif

  // #ifdef ENABLE_HOMEASSISTANT
  //   ha_send_data.attach(5, tickerSendState); // Send HA data back only every 5 sec
  // #endif

  // ***************************************************************************
  // Setup: MDNS responder
  // ***************************************************************************
  bool mdns_result = MDNS.begin(HOSTNAME);

  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(WiFi.localIP());
  DBG_OUTPUT_PORT.println("/ to open McLighting.");

  DBG_OUTPUT_PORT.print("Use http://");
  DBG_OUTPUT_PORT.print(HOSTNAME);
  DBG_OUTPUT_PORT.println(".local/ when you have Bonjour installed.");

  DBG_OUTPUT_PORT.print("New users: Open http://");
  DBG_OUTPUT_PORT.print(WiFi.localIP());
  DBG_OUTPUT_PORT.println("/upload to upload the webpages first.");

  DBG_OUTPUT_PORT.println("");


  // ***************************************************************************
  // Setup: WebSocket server
  // ***************************************************************************
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // ***************************************************************************
  // Setup: SPIFFS Webserver handler
  // ***************************************************************************
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);
  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/esp_status", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    // json += ", \"analog\":" + String(analogRead(A0));
    // json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });


  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      handleNotFound();
  });

  server.on("/upload", handleMinimalUpload);

  server.on("/restart", []() {
    DBG_OUTPUT_PORT.printf("/restart\n");
    server.send(200, "text/plain", "restarting..." );
    ESP.restart();
  });

  server.on("/reset_wlan", []() {
    DBG_OUTPUT_PORT.printf("/reset_wlan\n");
    server.send(200, "text/plain", "Resetting WLAN and restarting..." );
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.restart();
  });

  server.on("/start_config_ap", []() {
    DBG_OUTPUT_PORT.printf("/start_config_ap\n");
    server.send(200, "text/plain", "Starting config AP ..." );
    WiFiManager wifiManager;
    wifiManager.startConfigPortal(HOSTNAME);
  });


  // ***************************************************************************
  // Setup: SPIFFS Webserver handler
  // ***************************************************************************
  server.on("/set_brightness", []() {
    getArgs();
	mode = BRIGHTNESS;
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK %") + String(brightness)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK %") + String(brightness)).c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
      if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
    getStatusJSON();
  });

  server.on("/get_brightness", []() {
    String str_brightness = String((int) (brightness / 2.55));
    server.send(200, "text/plain", str_brightness );
    DBG_OUTPUT_PORT.print("/get_brightness: ");
    DBG_OUTPUT_PORT.println(str_brightness);
  });

  server.on("/set_speed", []() {
    if (server.arg("d").toInt() >= 0) {
      ws2812fx_speed = server.arg("d").toInt();
      ws2812fx_speed = constrain(ws2812fx_speed, 0, 255);
      strip.setSpeed(convertSpeed(ws2812fx_speed));
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String(String("OK ?") + String(ws2812fx_speed)).c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK ?") + String(ws2812fx_speed)).c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
      #endif
    }

    getStatusJSON();
  });

  server.on("/get_speed", []() {
    String str_speed = String(ws2812fx_speed);
    server.send(200, "text/plain", str_speed );
    DBG_OUTPUT_PORT.print("/get_speed: ");
    DBG_OUTPUT_PORT.println(str_speed);
  });

  server.on("/get_switch", []() {
    server.send(200, "text/plain", (mode == OFF) ? "0" : "1" );
    DBG_OUTPUT_PORT.printf("/get_switch: %s\n", (mode == OFF) ? "0" : "1");
  });

  server.on("/get_color", []() {
    char rgbcolor[7];
    snprintf(rgbcolor, sizeof(rgbcolor), "%02X%02X%02X", main_color.red, main_color.green, main_color.blue);
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });

  server.on("/status", []() {
    getStatusJSON();
  });

  server.on("/off", []() {
    #ifdef ENABLE_LEGACY_ANIMATIONS
      exit_func = true;
    #endif
    mode = OFF;
    getArgs();
    getStatusJSON();
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String("OK =off").c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =off").c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = false;
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
  });

  server.on("/all", []() {
    #ifdef ENABLE_LEGACY_ANIMATIONS
      exit_func = true;
    #endif
    ws2812fx_mode = FX_MODE_STATIC;
    mode = SET_MODE;
    //mode = ALL;
    getArgs();
    getStatusJSON();
    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String("OK =all").c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =all").c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
  });

  #ifdef ENABLE_LEGACY_ANIMATIONS
    server.on("/wipe", []() {
      exit_func = true;
      mode = WIPE;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =wipe").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =wipe").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  
    server.on("/rainbow", []() {
      exit_func = true;
      mode = RAINBOW;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =rainbow").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =rainbow").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  
    server.on("/rainbowCycle", []() {
      exit_func = true;
      mode = RAINBOWCYCLE;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =rainbowCycle").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =rainbowCycle").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  
    server.on("/theaterchase", []() {
      exit_func = true;
      mode = THEATERCHASE;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =theaterchase").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =theaterchase").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  
    server.on("/twinkleRandom", []() {
      exit_func = true;
      mode = TWINKLERANDOM;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =twinkleRandom").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =twinkleRandom").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
    
    server.on("/theaterchaseRainbow", []() {
      exit_func = true;
      mode = THEATERCHASERAINBOW;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =theaterchaseRainbow").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =theaterchaseRainbow").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  
    server.on("/tv", []() {
      exit_func = true;
      mode = TV;
      getArgs();
      getStatusJSON();
      #ifdef ENABLE_MQTT
      mqtt_client.publish(mqtt_outtopic, String("OK =tv").c_str());
      #endif
      #ifdef ENABLE_AMQTT
      amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String("OK =tv").c_str());
      #endif
      #ifdef ENABLE_HOMEASSISTANT
        stateOn = true;
      #endif
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
      #endif
    });
  #endif

  server.on("/get_modes", []() {
    getModesJSON();
  });

  server.on("/set_mode", []() {
    getArgs();
    mode = SET_MODE;
    getStatusJSON();

    #ifdef ENABLE_MQTT
    mqtt_client.publish(mqtt_outtopic, String(String("OK /") + String(ws2812fx_mode)).c_str());
    #endif
    #ifdef ENABLE_AMQTT
    amqttClient.publish(mqtt_outtopic.c_str(), qospub, false, String(String("OK /") + String(ws2812fx_mode)).c_str());
    #endif
    #ifdef ENABLE_HOMEASSISTANT
      stateOn = true;
      if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
    #endif
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!spiffs_save_state.active()) spiffs_save_state.once(3, tickerSpiffsSaveState);
    #endif
  });

  #ifdef HTTP_OTA
    httpUpdater.setup(&server, "/update");
  #endif

  server.begin();

  // Start MDNS service
  if (mdns_result) {
    MDNS.addService("http", "tcp", 80);
  }
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    (readStateFS()) ? DBG_OUTPUT_PORT.println(" Success!") : DBG_OUTPUT_PORT.println(" Failure!");
  #endif
  #ifdef ENABLE_STATE_SAVE_EEPROM
    // Load state string from EEPROM
    String saved_state_string = readEEPROM(256, 32);
    String chk = getValue(saved_state_string, '|', 0);
    if (chk == "STA") {
      DBG_OUTPUT_PORT.printf("Found saved state: %s\n", saved_state_string.c_str());
      setModeByStateString(saved_state_string);
    }
    sprintf(last_state, "STA|%2d|%3d|%3d|%3d|%3d|%3d|%3d", mode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);
  #endif
}


void loop() {
  #ifdef ENABLE_BUTTON
    button();
  #endif
  server.handleClient();
  webSocket.loop();

  #ifdef ENABLE_OTA
    ArduinoOTA.handle();
  #endif

  #ifdef ENABLE_MQTT
    if (WiFi.status() != WL_CONNECTED) {
      #ifdef ENABLE_HOMEASSISTANT
         ha_send_data.detach();
      #endif
      DBG_OUTPUT_PORT.println("WiFi disconnected, reconnecting!");
      WiFi.disconnect();
      WiFi.setSleepMode(WIFI_NONE_SLEEP);
      WiFi.mode(WIFI_STA);
      WiFi.begin();
    } else {
      if (mqtt_host != "" && String(mqtt_port).toInt() > 0 && mqtt_reconnect_retries < MQTT_MAX_RECONNECT_TRIES) {
        if (!mqtt_client.connected()) {
          #ifdef ENABLE_HOMEASSISTANT
           ha_send_data.detach();
          #endif
          DBG_OUTPUT_PORT.println("MQTT disconnected, reconnecting!");
          mqtt_reconnect();
        } else {
          mqtt_client.loop();
        }
      }
    }
  #endif
  #ifdef ENABLE_HOMEASSISTANT
//   if(!ha_send_data.active())  ha_send_data.once(5, tickerSendState);
   if (new_ha_mqtt_msg) sendState();
  #endif
          
  // Simple statemachine that handles the different modes
  if (mode == SET_MODE) {
    DBG_OUTPUT_PORT.printf("SET_MODE: %d %d\n", ws2812fx_mode, mode);
    strip.setMode(ws2812fx_mode);
    prevmode = SET_MODE;
    mode = SETCOLOR;
  }
  if (mode == OFF) {
    if(strip.isRunning()) strip.stop(); //should clear memory
    // mode = HOLD;
  }
  if (mode == SETCOLOR) {
    strip.setColor(main_color.red, main_color.green, main_color.blue);
    mode = (prevmode == SET_MODE) ? SETSPEED : HOLD;
  }
  if (mode == SETSPEED) {
    strip.setSpeed(convertSpeed(ws2812fx_speed));
    mode = (prevmode == SET_MODE) ? BRIGHTNESS : HOLD;
  }
  if (mode == BRIGHTNESS) {
    strip.setBrightness(brightness);
    if (prevmode == SET_MODE) prevmode == HOLD;
    mode = HOLD;
  }
  #ifdef ENABLE_LEGACY_ANIMATIONS
    if (mode == WIPE) {
      strip.setColor(main_color.red, main_color.green, main_color.blue);
      strip.setMode(FX_MODE_COLOR_WIPE);
      mode = HOLD;
    }
    if (mode == RAINBOW) {
      strip.setMode(FX_MODE_RAINBOW);
      mode = HOLD;
    }
    if (mode == RAINBOWCYCLE) {
      strip.setMode(FX_MODE_RAINBOW_CYCLE);
      mode = HOLD;
    }
    if (mode == THEATERCHASE) {
      strip.setColor(main_color.red, main_color.green, main_color.blue);
      strip.setMode(FX_MODE_THEATER_CHASE);
      mode = HOLD;
    }
    if (mode == TWINKLERANDOM) {
      strip.setColor(main_color.red, main_color.green, main_color.blue);
      strip.setMode(FX_MODE_TWINKLE_RANDOM);
      mode = HOLD;
    }
    if (mode == THEATERCHASERAINBOW) {
      strip.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
      mode = HOLD;
    }
  #endif
  if (mode == HOLD || mode == CUSTOM) {
    if(!strip.isRunning()) strip.start();
    #ifdef ENABLE_LEGACY_ANIMATIONS
      if (exit_func) {
        exit_func = false;
      }
    #endif
  }
  #ifdef ENABLE_LEGACY_ANIMATIONS
    if (mode == TV) {
      if(!strip.isRunning()) strip.start();
      tv();
    }
  #endif

  // Only for modes with WS2812FX functionality
  #ifdef ENABLE_LEGACY_ANIMATIONS
  if (mode != TV && mode != CUSTOM) {
  #else
  if (mode != CUSTOM) {
  #endif
    strip.service();
  }

  #ifdef ENABLE_STATE_SAVE_SPIFFS
    if (updateStateFS) {
      (writeStateFS()) ? DBG_OUTPUT_PORT.println(" Success!") : DBG_OUTPUT_PORT.println(" Failure!");
    }
  #endif

  #ifdef ENABLE_STATE_SAVE_EEPROM
    // Check for state changes
    sprintf(current_state, "STA|%2d|%3d|%3d|%3d|%3d|%3d|%3d", mode, strip.getMode(), ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue);

    if (strcmp(current_state, last_state) != 0) {
      // DBG_OUTPUT_PORT.printf("STATE CHANGED: %s / %s\n", last_state, current_state);
      strcpy(last_state, current_state);
      time_statechange = millis();
      state_save_requested = true;
    }
    if (state_save_requested && time_statechange + timeout_statechange_save <= millis()) {
      time_statechange = 0;
      state_save_requested = false;
      writeEEPROM(256, 32, last_state); // 256 --> last_state (reserved 32 bytes)
      EEPROM.commit();
    }
  #endif
}
