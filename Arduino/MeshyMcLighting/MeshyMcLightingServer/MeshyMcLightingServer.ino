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
  
  modes.reserve(5000);
  modes_setup();

  #ifdef ENABLE_AMQTT
    async_mqtt_setup();
  #endif
  
  DEBUG_PRINT("WS2812FX setup ... ");
  strip.init();
  DEBUG_PRINTLN("done!");
  
  DEBUG_PRINTLN("---------- WiFi Mesh Setup ---------");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname(HOSTNAME);
    
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  //mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, STATION_WIFI_CHANNEL );  //develop
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, STA_AP, WIFI_AUTH_WPA2_PSK, STATION_WIFI_CHANNEL);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  
  //myAPIP = IPAddress(mesh.getAPIP());  // develop
  myAPIP = IPAddress(mesh.getAPIP().addr);  //master
  DEBUG_PRINTLN("My AP IP is " + myAPIP.toString());

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  DEBUG_PRINTLN("----- WiFi Mesh Setup complete -----");

  //Async webserver
  DEBUG_PRINT("Async HTTP server starting ... ");
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", uploadspiffs_html);
  });

  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "upload success");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      DEBUG_PRINTF("UploadStart: %s\n", filename.c_str());
      if (!filename.startsWith("/")) filename = "/" + filename;
      if (SPIFFS.exists(filename)) SPIFFS.remove(filename);
      fsUploadFile = SPIFFS.open(filename, "w");
    }
    for(size_t i=0; i<len; i++)
      fsUploadFile.write(data[i]);
    if(final){
      fsUploadFile.close();
      DEBUG_PRINTF3("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
    }
  });

  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", update_html);
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
    if(shouldReboot) ESP.restart();
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if (!filename.endsWith(".bin")) {
      return;
    }
    if(!index){
      DEBUG_PRINTF("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
      #ifdef SERIALDEBUG
        Update.printError(Serial);
      #endif
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len) {
      #ifdef SERIALDEBUG
        Update.printError(Serial);
      #endif
      }
    }
    if(final){
      if(Update.end(true)) 
        DEBUG_PRINTF("Update Success: %uB\n", index+len);
      else {
      #ifdef SERIALDEBUG
        Update.printError(Serial);
      #endif
      }
    }
  });
  
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/javascript", main_js);
  });

  server.on("/modes", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", modes);
  });

  server.on("/setmode", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasArg("rgb")){
      String color = request->arg("rgb");
      uint32_t tmp = (uint32_t) strtol(color.c_str(), NULL, 16);
      if(tmp >= 0x000000 && tmp <= 0xFFFFFF) {
        uint8_t r = ((tmp >> 16) & 0xFF);
        uint8_t g = ((tmp >>  8) & 0xFF);
        uint8_t b = (tmp         & 0xFF);
        main_color = {r, g, b};
        if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
        #ifdef ENABLE_STATE_SAVE_SPIFFS
          if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
        #endif
        stateOn = true;
        mode = SETCOLOR;
      }
    }

    if (request->hasArg("c")) {
      String brightnesss = request->arg("c");
      if(brightnesss == "-") {
        brightness = constrain(strip.getBrightness() * 0.8, 0, 255);
      } else if(brightnesss == " ") {
        brightness = constrain(strip.getBrightness() * 1.2, 6, 255);
      } else { // set brightness directly
        uint8_t tmp = (uint8_t) strtol(brightnesss.c_str(), NULL, 10);
        brightness = tmp;
      }
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
      stateOn = true;
      mode = BRIGHTNESS;
    }

    if (request->hasArg("s")) {
      String http_speed = request->arg("s");
      if(http_speed == "-") {
        ws2812fx_speed = constrain(strip.getSpeed() * 0.8, SPEED_MIN, SPEED_MAX);
      } else if (http_speed == " ") {
        ws2812fx_speed = constrain(strip.getSpeed() * 1.2, SPEED_MIN, SPEED_MAX);
      } else {
        uint8_t tmp = (uint8_t) strtol(http_speed.c_str(), NULL, 10);
        ws2812fx_speed = convertSpeed(tmp);
      }
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
      stateOn = true;
      mode = SETSPEED;
    }

    if (request->hasArg("a")) {
      if(request->arg("a") == "-") {
        //auto_cycle = false;
        //mode = HOLD;
        handleAutoStop();
      } else {
        //auto_cycle = true;
        //auto_last_change = 0;
        //mode = HOLD;
        handleAutoStart();
      }
    }

    if (request->hasArg("m")) {
      String modes = request->arg("m");
      uint8_t tmp = (uint8_t) strtol(modes.c_str(), NULL, 10);
      if(tmp > 0) {
        ws2812fx_mode = (tmp - 1) % strip.getModeCount();
        stateOn = true;
        mode = SET_MODE;
      } else {
        ws2812fx_mode = FX_MODE_STATIC;
        stateOn = false;
        mode = OFF;
      }
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
    }
    
    request->send(200, "text/plain", "OK");
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    String filename = request->url();
    String ContentType = "text/plain";
    if (filename.endsWith(".htm"))
      ContentType = "text/html";
    else if (filename.endsWith(".html"))
      ContentType = "text/html";
    else if (filename.endsWith(".css"))
      ContentType = "text/css";
    else if (filename.endsWith(".js"))
      ContentType = "application/javascript";
    else if (filename.endsWith(".png"))
      ContentType = "image/png";
    else if (filename.endsWith(".gif"))
      ContentType = "image/gif";
    else if (filename.endsWith(".jpg"))
      ContentType = "image/jpeg";
    else if (filename.endsWith(".ico"))
      ContentType = "image/x-icon";
    else if (filename.endsWith(".xml"))
      ContentType = "text/xml";
    else if (filename.endsWith(".pdf"))
      ContentType = "application/x-pdf";
    else if (filename.endsWith(".zip"))
      ContentType = "application/x-zip";
    else if (filename.endsWith(".gz"))
      ContentType = "application/x-gzip";

    if (SPIFFS.exists(filename + ".gz") || SPIFFS.exists(filename)) {
      if (SPIFFS.exists(filename + ".gz")) filename += ".gz";
      request->send(SPIFFS, filename, ContentType);
      return true;
    }

    DEBUG_PRINT("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      DEBUG_PRINT("GET");
    else if(request->method() == HTTP_POST)
      DEBUG_PRINT("POST");
    else if(request->method() == HTTP_DELETE)
      DEBUG_PRINT("DELETE");
    else if(request->method() == HTTP_PUT)
      DEBUG_PRINT("PUT");
    else if(request->method() == HTTP_PATCH)
      DEBUG_PRINT("PATCH");
    else if(request->method() == HTTP_HEAD)
      DEBUG_PRINT("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      DEBUG_PRINT("OPTIONS");
    else
      DEBUG_PRINT("UNKNOWN");

    DEBUG_PRINTF3(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      DEBUG_PRINTF("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      DEBUG_PRINTF("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      DEBUG_PRINTF3("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        DEBUG_PRINTF4("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());       
      } else if(p->isPost()){
        DEBUG_PRINTF3("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        DEBUG_PRINTF3("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  
//  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
//    File fsUploadFile;
//    if(!index){
//      DEBUG_PRINTF("UploadStart: %s\n", filename.c_str());
//      fsUploadFile = SPIFFS.open("/" + filename, "w");
//    }
//    for(size_t i=0; i<len; i++){
//      fsUploadFile.write(data[i]);
//    }
//    if(final){
//      fsUploadFile.close();
//      DEBUG_PRINTF3("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
//    }
//    request->send(200, "text/plain", "Upload " + filename);
//  });
  
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
