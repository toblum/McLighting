#include <FS.h>
//#include <ESP8266mDNS.h>
#include <painlessMesh.h>           //https://gitlab.com/painlessMesh/painlessMesh/tree/develop
#include <ArduinoJson.h>            //https://github.com/bblanchon/ArduinoJson
#include "definitions.h"

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

#include "request_handlers.h"

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Starting...");

  if(SPIFFS.begin()){
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %dB\n", fileName.c_str(), fileSize);
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.printf("FS Usage: %d/%d bytes\n\n", fs_info.usedBytes, fs_info.totalBytes);
  }
  
  modes.reserve(5000);
  modes_setup();

  #ifdef ENABLE_AMQTT
    async_mqtt_setup();
  #endif
  
  Serial.print("WS2812FX setup ... ");
  strip.init();
  Serial.println("done!");
  
  Serial.println("---------- WiFi Mesh Setup ---------");
  mesh_setup();
  Serial.println("----- WiFi Mesh Setup complete -----");

  //Async webserver
  Serial.print("Async HTTP server starting ... ");
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
      Serial.printf("UploadStart: %s\n", filename.c_str());
      if (!filename.startsWith("/")) filename = "/" + filename;
      if (SPIFFS.exists(filename)) SPIFFS.remove(filename);
      fsUploadFile = SPIFFS.open(filename, "w");
    }
    for(size_t i=0; i<len; i++)
      fsUploadFile.write(data[i]);
    if(final){
      fsUploadFile.close();
      Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
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
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) Update.printError(Serial);
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len)
        Update.printError(Serial);
    }
    if(final){
      if(Update.end(true)) 
        Serial.printf("Update Success: %uB\n", index+len);
      else 
        Update.printError(Serial);
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
        brightness = strip.getBrightness() * 0.8;
      } else if(brightnesss == " ") {
        brightness = min(max(strip.getBrightness(), 5) * 1.2, 255);
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
      ws2812fx_speed = (request->arg("s") == "-") ? strip.getSpeed() * 0.8 : max(strip.getSpeed(), 5) * 1.2 ;
      if(!taskSendMessage.isEnabled()) taskSendMessage.enableDelayed(TASK_SECOND * 5);
      #ifdef ENABLE_STATE_SAVE_SPIFFS
        if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
      #endif
      stateOn = true;
      mode = SETSPEED;
    }

    if (request->hasArg("a")) {
      if(request->arg("a") == "-") {
        auto_cycle = false;
        mode = HOLD;
      } else {
        auto_cycle = true;
        auto_last_change = 0;
        mode = HOLD;
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
        stateOn = true;
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

    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");

    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());       
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    request->send(404);
  });
  
//  server.onFileUpload([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
//    File fsUploadFile;
//    if(!index){
//      Serial.printf("UploadStart: %s\n", filename.c_str());
//      fsUploadFile = SPIFFS.open("/" + filename, "w");
//    }
//    for(size_t i=0; i<len; i++){
//      fsUploadFile.write(data[i]);
//    }
//    if(final){
//      fsUploadFile.close();
//      Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index+len);
//    }
//    request->send(200, "text/plain", "Upload " + filename);
//  });
  
  server.begin();
  Serial.println("done!");

//  Serial.print("Starting MDNS ... ");
//  bool mdns_result = MDNS.begin(HOSTNAME);
//  if (mdns_result) MDNS.addService("http", "tcp", 80);
//  Serial.println("done!");

  #ifdef ENABLE_STATE_SAVE_SPIFFS
    Serial.println((readStateFS()) ? " Success!" : " Failure!");
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
}

void loop() {
  unsigned long now = millis();

  userScheduler.execute();
  mesh.update();
  if(myIP != getlocalIP()){
    myIP = getlocalIP();
    Serial.println("Station IP is " + myIP.toString());
    #ifdef ENABLE_AMQTT
      if(myIP != IPAddress(0,0,0,0)) {
      //if (WiFi.isConnected()) {
        //connectToMqtt();
        taskConnecttMqtt.enable();
      }
    #endif
  }
  // Simple statemachine that handles the different modes
  if (mode == SET_MODE) {
    strip.setMode(ws2812fx_mode);
    Serial.print("mode is "); Serial.println(strip.getModeName(strip.getMode()));
    prevmode = SET_MODE;
    mode = SETCOLOR;
  }
  if (mode == OFF) {
    Serial.println("mode is OFF");
    if(strip.isRunning()) strip.stop(); //should clear memory
    mode = HOLD;
  }
  if (mode == SETCOLOR) {
    strip.setColor(main_color.red, main_color.green, main_color.blue);
    Serial.printf("color R: %d G: %d B: %d\n", main_color.red, main_color.green, main_color.blue);
    mode = (prevmode == SET_MODE) ? SETSPEED : HOLD;
  }
  if (mode == SETSPEED) {
    strip.setSpeed(ws2812fx_speed);
    Serial.print("speed is "); Serial.println(strip.getSpeed());
    mode = (prevmode == SET_MODE) ? BRIGHTNESS : HOLD;
  }
  if (mode == BRIGHTNESS) {
    strip.setBrightness(brightness);
    Serial.print("brightness is "); Serial.println(strip.getBrightness());
    if (prevmode == SET_MODE) prevmode == HOLD;
    mode = HOLD;
  }
  if (mode == HOLD or mode == CUSTOM or auto_cycle) {
    if(!strip.isRunning()) strip.start();
    strip.service();
  }

  if(auto_cycle && (now - auto_last_change > 10000)) { // cycle effect mode every 10 seconds
    uint8_t next_mode = (strip.getMode() + 1) % strip.getModeCount();
    if(sizeof(myModes) > 0) { // if custom list of modes exists
      for(uint8_t i=0; i < sizeof(myModes); i++) {
        if(myModes[i] == strip.getMode()) {
          next_mode = ((i + 1) < sizeof(myModes)) ? myModes[i + 1] : myModes[0];
          break;
        }
      }
    }
    strip.setMode(next_mode);
    Serial.print("mode is "); Serial.println(strip.getModeName(strip.getMode()));
    taskSendMessage.enableIfNot();
    taskSendMessage.forceNextIteration();
    #ifdef ENABLE_STATE_SAVE_SPIFFS
      if(!taskSpiffsSaveState.isEnabled()) taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
    #endif
    auto_last_change = now;
  }
}
