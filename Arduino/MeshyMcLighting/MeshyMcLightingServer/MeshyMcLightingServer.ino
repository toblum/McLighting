#include <FS.h>
#include <ESP8266mDNS.h>
#include <painlessMesh.h>           //https://gitlab.com/painlessMesh/painlessMesh/tree/develop
#include <ArduinoJson.h>            //https://github.com/bblanchon/ArduinoJson
#include "definitions.h"            //https://github.com/arkhipenko/TaskScheduler
#include "version.h"

#ifdef ESP8266
#include <Hash.h>
#include <ESPAsyncTCP.h>            //https://github.com/me-no-dev/ESPAsyncTCP
#else
#include <AsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>      //https://github.com/me-no-dev/ESPAsyncWebServer
#ifdef ENABLE_AMQTT
  #include <AsyncMqttClient.h>      //https://github.com/marvinroger/async-mqtt-client
  AsyncMqttClient mqttClient;
#endif
#include <ESPAsyncWiFiManager.h>    //https://github.com/alanswx/ESPAsyncWiFiManager

// Prototypes
void autoTick(void);
void sendMessage(void);
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
bool readConfigFS();
bool writeConfigFS(bool saveConfig);
#ifdef ENABLE_AMQTT
  void connectToMqtt(void);
#endif
#ifdef ENABLE_HOMEASSISTANT
  void fnSendHAState(void);
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
  void fnSpiffsSaveState(void);
#endif
IPAddress getlocalIP();

painlessMesh  mesh;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dns;
const byte DNS_PORT = 53;          // Capture DNS requests on port 53

IPAddress myIP(0,0,0,0);
IPAddress myAPIP(0,0,0,0);
File fsUploadFile;

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
  WS2812FX strip = WS2812FX(NUMLEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
  
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

SimpleList<uint32_t> nodes;
Scheduler userScheduler; // to control your personal task
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage );
#ifdef ENABLE_AMQTT
  Task taskConnecttMqtt( TASK_SECOND * 5, MAX_MQTT_RETRIES, &connectToMqtt ); // max connect MAX_MQTT_RETRIES times to MQTT server and this Task can be restarted upon successful StationIP
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
  Task taskSpiffsSaveState(TASK_SECOND * 1, TASK_FOREVER, &fnSpiffsSaveState);
#endif
#ifdef ENABLE_HOMEASSISTANT
  Task taskSendHAState(TASK_SECOND * 3, TASK_FOREVER, &fnSendHAState);
#endif
Task autoModeTask(TASK_SECOND * (int) autoParams[0][3], TASK_FOREVER, &autoTick);

#include "request_handlers.h"
#include "WiFiHelper.h"

void setup(){
  #ifdef SERIALDEBUG
    Serial.begin(115200);
  #endif
  DEBUG_PRINTLN();
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Starting...");

  if(SPIFFS.begin()){
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DEBUG_PRINTF3("FS File: %s, size: %dB\n", fileName.c_str(), fileSize);
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    DEBUG_PRINTF3("FS Usage: %d/%d bytes\n\n", fs_info.usedBytes, fs_info.totalBytes);
  }

  connectToWiFi();
  
  DEBUG_PRINT("WS28xx strip setup ");
  strip.init(); DEBUG_PRINT(".");
  DEBUG_PRINTLN(" done!");

  DEBUG_PRINTLN("---------- WiFi Mesh Setup ---------");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname(HOSTNAME);
    
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, STATION_WIFI_CHANNEL );  //develop
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, wifi_channel);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  mesh.stationManual(wifi_ssid, wifi_pwd);
  mesh.setHostname(HOSTNAME);
  
  //myAPIP = IPAddress(mesh.getAPIP());  // develop
  myAPIP = IPAddress(mesh.getAPIP().addr);  //master
  DEBUG_PRINTLN("My AP IP is " + myAPIP.toString());

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  DEBUG_PRINTLN("----- WiFi Mesh Setup complete -----");

  //Async webserver
  modes_write_to_spiffs();
  modes_write_to_spiffs_mclighting();

  DEBUG_PRINT("Starting mDNS ");
  dns.start(DNS_PORT, "*", myAPIP);   // Start DNS server and redirect every request from port 53 to MeshIP
  bool mdns_result = MDNS.begin(HOSTNAME); DEBUG_PRINT(".");
  if (mdns_result) MDNS.addService("http", "tcp", 80); DEBUG_PRINT(".");
  DEBUG_PRINTLN(" done!");
  
  DEBUG_PRINT("Async HTTP server starting ");
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  /// everything called is present in WiFiHelper.h
  server.on("/esp_status", HTTP_GET, handleESPStatus); DEBUG_PRINT(".");
  server.on("/", HTTP_GET, handleRoot); DEBUG_PRINT(".");
  //server.on("/scan", HTTP_GET, handleScanNet);   //Custom WiFi Scanning Webpage attempt by @debsahu for AsyncWebServer
  //server.on("/scan", HTTP_POST, processScanNet); //Custom WiFi Scanning Webpage attempt by @debsahu for AsyncWebServer
  server.on("/upload", HTTP_GET, handleUpload); DEBUG_PRINT(".");
  server.on("/upload", HTTP_POST, processUploadReply, processUpload); DEBUG_PRINT(".");
  server.on("/update", HTTP_GET, handleUpdate); DEBUG_PRINT(".");
  server.on("/update", HTTP_POST, processUpdateReply, processUpdate); DEBUG_PRINT(".");
  server.on("/main.js", HTTP_GET, handlemainjs); DEBUG_PRINT(".");
  server.on("/modes", HTTP_GET, handleModes); DEBUG_PRINT(".");
  server.on("/status", HTTP_GET, handleStatus); DEBUG_PRINT(".");
  server.on("/restart", HTTP_GET, handleRestart); DEBUG_PRINT(".");
  server.on("/reset_wlan", HTTP_GET, handleResetWLAN); DEBUG_PRINT(".");
  server.on("/start_config_ap", HTTP_GET, handleStartAP); DEBUG_PRINT(".");
  server.on("/version", HTTP_GET, handleVersion); DEBUG_PRINT(".");
  //Light States
  server.on("/get_brightness", HTTP_GET, handleGetBrightness); DEBUG_PRINT(".");
  server.on("/get_speed", HTTP_GET, handleGetSpeed); DEBUG_PRINT(".");
  server.on("/get_color", HTTP_GET, handleGetColor); DEBUG_PRINT(".");
  server.on("/get_switch", HTTP_GET, handleGetSwitch); DEBUG_PRINT(".");
  server.on("/off", HTTP_GET, handleOff); DEBUG_PRINT(".");
  server.on("/get_modes", HTTP_GET, handleGetModes); DEBUG_PRINT(".");
  server.on("/set_mode", HTTP_GET, handleSetMode); DEBUG_PRINT(".");
  server.onNotFound(handleNotFound); DEBUG_PRINT(".");
  //server.onFileUpload(handleUpload);            //Not safe
  
  server.begin();
  DEBUG_PRINTLN(" done!");

  #ifdef ENABLE_STATE_SAVE_SPIFFS
    DEBUG_PRINTLN((readStateFS()) ? " Success!" : " Failure!");
  #endif

  #ifdef ENABLE_AMQTT
    async_mqtt_setup();
    userScheduler.addTask(taskConnecttMqtt);
    taskConnecttMqtt.disable();
  #endif
  #ifdef ENABLE_STATE_SAVE_SPIFFS
    userScheduler.addTask(taskSpiffsSaveState);
    taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
  #ifdef ENABLE_HOMEASSISTANT
    userScheduler.addTask(taskSendHAState);
    taskSendHAState.enable();
  #endif

  //AutoMode Scheduler
  userScheduler.addTask(autoModeTask);
  autoModeTask.disable();
}

void loop() {
  #ifdef ENABLE_BUTTON
    button();
  #endif
  userScheduler.execute();
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    DEBUG_PRINTLN("Station IP is " + myIP.toString());
    #ifdef ENABLE_AMQTT
      if(myIP != IPAddress(0,0,0,0)) {
      //if (WiFi.isConnected()) {
        //connectToMqtt();
        dns.stop();
        dns = DNSServer();              // Don't know if this will mess with mDNS
        dns.start(DNS_PORT, "*", myIP); // Start DNS server and redirect every request from port 53 to our newly obtained IP
        taskConnecttMqtt.restart();
        taskConnecttMqtt.enableIfNot();
      } else {
        if(taskConnecttMqtt.isEnabled()) taskConnecttMqtt.disable();
      }
    #endif
  }
  #ifdef ENABLE_AMQTT
    if((WiFi.status() != WL_CONNECTED or getlocalIP() == IPAddress(0,0,0,0)) and taskConnecttMqtt.isEnabled()){
      taskConnecttMqtt.disable(); // No need to connect to MQTT if there is no connection to WiFi Router
      dns.stop();
      dns = DNSServer();                // Don't know if this will mess with mDNS
      dns.start(DNS_PORT, "*", myAPIP); // Start DNS server and redirect every request from port 53 to MeshIP
    }
  #endif
  
  // Simple statemachine that handles the different modes
  if (mode == SET_MODE) {
    strip.setMode(ws2812fx_mode);
    DEBUG_PRINT("mode is "); DEBUG_PRINTLN(strip.getModeName(strip.getMode()));
    prevmode = SET_MODE;
    mode = SETCOLOR;
  }
  if (mode == OFF) {
    DEBUG_PRINTLN("mode is OFF");
    if(strip.isRunning()) strip.stop(); //should clear memory
    mode = HOLD;
  }
  if (mode == SETCOLOR) {
    strip.setColor(main_color.red, main_color.green, main_color.blue);
    DEBUG_PRINTF4("color R: %d G: %d B: %d\n", main_color.red, main_color.green, main_color.blue);
    mode = (prevmode == SET_MODE) ? SETSPEED : HOLD;
  }
  if (mode == SETSPEED) {
    strip.setSpeed(ws2812fx_speed);
    DEBUG_PRINT("speed is "); DEBUG_PRINTLN(strip.getSpeed());
    mode = (prevmode == SET_MODE) ? BRIGHTNESS : HOLD;
  }
  if (mode == BRIGHTNESS) {
    strip.setBrightness(brightness);
    DEBUG_PRINT("brightness is "); DEBUG_PRINTLN(strip.getBrightness());
    if (prevmode == SET_MODE) prevmode == HOLD;
    mode = HOLD;
  }
  //if (mode == HOLD or mode == CUSTOM or auto_cycle) {
  if (mode == HOLD or mode == CUSTOM) {
    if(!strip.isRunning()) strip.start();
    strip.service();
  }
}
