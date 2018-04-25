#include <FS.h>
#include <ESP8266mDNS.h>
#include <WS2812FX.h>
#include <painlessMesh.h>        //https://gitlab.com/painlessMesh/painlessMesh/tree/develop
#include <ArduinoJson.h>         //https://github.com/bblanchon/ArduinoJson
#include "definitions.h"

// Prototypes
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
#ifdef ENABLE_STATE_SAVE_SPIFFS
  void fnSpiffsSaveState(void);
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

painlessMesh  mesh;

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
#ifdef ENABLE_STATE_SAVE_SPIFFS
  Task taskSpiffsSaveState(TASK_SECOND * 1, TASK_ONCE, &fnSpiffsSaveState);
#endif

#include "request_handlers.h"

void mesh_setup() {
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.hostname("MeshyMcLighting");
  //mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  mesh.setHostname(HOSTNAME);
  mesh.init( MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, STATION_WIFI_CHANNEL );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  
  myAPIP = IPAddress(mesh.getAPIP());
  Serial.println("My AP IP is " + myAPIP.toString());
}

void modes_setup() {
  modes = "<li><a href='#' class='m' id='0'>OFF</a></li>";
  uint8_t num_modes = sizeof(myModes) > 0 ? sizeof(myModes) + 1 : strip.getModeCount() + 1;
  for(uint8_t i=1; i < num_modes; i++) {
    uint8_t m = sizeof(myModes) > 0 ? myModes[i-1] : i;
    modes += "<li><a href='#' class='m' id='";
    modes += (m);
    modes += "'>";
    modes += strip.getModeName(m-1);
    modes += "</a></li>";
  }
}

void receivedCallback(uint32_t from, String & msg) {
  //Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  if(!processJson(msg)) return;
  else Serial.printf("Processed incoming message from %u successfully!", from);
}

void newConnectionCallback(uint32_t nodeId) {
  String msg = statusLEDs();
  mesh.sendSingle(nodeId, msg);
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

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
  
  Serial.print("WS2812FX setup ... ");
  strip.init();
  Serial.println("done!");
  
  Serial.println("---------- WiFi Mesh Setup ---------");
  mesh_setup();
  Serial.println("----- WiFi Mesh Setup complete -----");

  #ifdef ENABLE_STATE_SAVE_SPIFFS
    Serial.println((readStateFS()) ? " Success!" : " Failure!");
    userScheduler.addTask(taskSpiffsSaveState);
    taskSpiffsSaveState.enableDelayed(TASK_SECOND * 3);
  #endif
}

void loop() {
  unsigned long now = millis();

  userScheduler.execute();
  mesh.update();
//  if(myIP != getlocalIP()){
//    myIP = getlocalIP();
//    Serial.println("Station IP is " + myIP.toString());
//  }
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
    auto_last_change = now;
  }
}
