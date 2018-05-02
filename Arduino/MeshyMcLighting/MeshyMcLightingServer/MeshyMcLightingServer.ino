#include <FS.h>
//#include <ESP8266mDNS.h>
#include <painlessMesh.h>           //https://gitlab.com/painlessMesh/painlessMesh/tree/develop
#include <ArduinoJson.h>            //https://github.com/bblanchon/ArduinoJson
#include "definitions.h"            //https://github.com/arkhipenko/TaskScheduler

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

// Prototypes
void autoTick(void);
void sendMessage(void);
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
#ifdef ENABLE_AMQTT
  void connectToMqtt(void);
#endif
#ifdef ENABLE_HOMEASSISTANT
  void fnSendHAState(void);
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
  void fnSpiffsSaveState(void);
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

painlessMesh  mesh;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

IPAddress getlocalIP();
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
  Task taskConnecttMqtt( TASK_SECOND * 1, TASK_FOREVER, &connectToMqtt ); 
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
  
  DEBUG_PRINT("WS2812FX setup ... ");
  strip.init();
  DEBUG_PRINTLN("done!");

  DEBUG_PRINTLN("---------- WiFi Mesh Setup ---------");
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname(HOSTNAME);
    
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
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
  //modes.reserve(5000);
  modes_write_to_spiffs();
  
  DEBUG_PRINT("Async HTTP server starting ... ");
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  /// everything called is present in WiFiHelper.h
  server.on("/heap", HTTP_GET, handleHeap);
  server.on("/", HTTP_GET, handleRoot);
  //server.on("/scan", HTTP_GET, handleScanNet);   //Custom WiFi Scanning Webpage attempt by @debsahu for AsyncWebServer
  //server.on("/scan", HTTP_POST, processScanNet); //Custom WiFi Scanning Webpage attempt by @debsahu for AsyncWebServer
  server.on("/upload", HTTP_GET, handleUpload);
  server.on("/upload", HTTP_POST, processUploadReply, processUpload);
  server.on("/update", HTTP_GET, handleUpdate);
  server.on("/update", HTTP_POST, processUpdateReply, processUpdate);
  server.on("/main.js", HTTP_GET, handlemainjs);
  server.on("/modes", HTTP_GET, handleModes);
  server.on("/setmode", HTTP_GET, handleSetMode);
  server.onNotFound(handleNotFound);
  //server.onFileUpload(handleUpload);            //Not safe
  
  server.begin();
  DEBUG_PRINTLN("done!");

//  DEBUG_PRINT("Starting MDNS ... ");
//  bool mdns_result = MDNS.begin(HOSTNAME);
//  if (mdns_result) MDNS.addService("http", "tcp", 80);
//  DEBUG_PRINTLN("done!");

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
        taskConnecttMqtt.enableDelayed(TASK_SECOND * 5);
      }
    #endif
  }
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
