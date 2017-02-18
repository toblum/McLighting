#include "definitions.h"

// ***************************************************************************
// Load libraries for: WebServer / WiFiManager / WebSockets
// ***************************************************************************
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

// needed for library WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#include <WebSockets.h>           //https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsServer.h>

// OTA
#ifdef ENABLE_OTA
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
#endif

// ***************************************************************************
// Instanciate HTTP(80) / WebSockets(81) Server
// ***************************************************************************
ESP8266WebServer server ( 80 );
WebSocketsServer webSocket = WebSocketsServer(81);


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


// ***************************************************************************
// Load library "ticker" for blinking status led
// ***************************************************************************
#include <Ticker.h>
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
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
#include "colormodes.h"



// ***************************************************************************
// MAIN
// ***************************************************************************
void setup() {
  DBG_OUTPUT_PORT.begin(115200);

  // set builtin led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  // ***************************************************************************
  // Setup: Neopixel
  // ***************************************************************************
  strip.init();
  strip.setBrightness(brightness);
  strip.setSpeed(ws2812fx_speed);
  //strip.setMode(FX_MODE_RAINBOW_CYCLE);
  strip.start();

  // ***************************************************************************
  // Setup: WiFiManager
  // ***************************************************************************
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

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
  // Setup: MDNS responder
  // ***************************************************************************
  MDNS.begin(HOSTNAME);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(WiFi.localIP());
  DBG_OUTPUT_PORT.println("/ to open McLighting.");

  DBG_OUTPUT_PORT.print("Use http://");
  DBG_OUTPUT_PORT.print(HOSTNAME);
  DBG_OUTPUT_PORT.println(".local/ when you have Bobjour installed.");

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
    DBG_OUTPUT_PORT.printf("\n");
  }

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
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
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
    DBG_OUTPUT_PORT.printf("/restart:\n");
    server.send(200, "text/plain", "restarting..." );
    ESP.restart();
  });

  server.on("/reset_wlan", []() {
    DBG_OUTPUT_PORT.printf("/reset_wlan:\n");
    server.send(200, "text/plain", "Resetting WLAN and restarting..." );
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    ESP.restart();
  });


  // ***************************************************************************
  // Setup: SPIFFS Webserver handler
  // ***************************************************************************
  server.on("/set_brightness", []() {
    if (server.arg("c").toInt() > 0) {
      brightness = (int) server.arg("c").toInt() * 2.55;
    } else {
      brightness = server.arg("p").toInt();
    }
    if (brightness > 255) {
      brightness = 255;
    }
    if (brightness < 0) {
      brightness = 0;
    }
    strip.setBrightness(brightness);

    if (mode == HOLD) {
      mode = ALL;
    }

    getStatusJSON();
  });

  server.on("/get_brightness", []() {
    String str_brightness = String((int) (brightness / 2.55));
    server.send(200, "text/plain", str_brightness );
    DBG_OUTPUT_PORT.print("/get_brightness: ");
    DBG_OUTPUT_PORT.println(str_brightness);
  });

  server.on("/get_switch", []() {
    server.send(200, "text/plain", (mode == OFF) ? "0" : "1" );
    DBG_OUTPUT_PORT.printf("/get_switch: %s\n", (mode == OFF) ? "0" : "1");
  });

  server.on("/get_color", []() {
    String rgbcolor = String(main_color.red, HEX) + String(main_color.green, HEX) + String(main_color.blue, HEX);
    server.send(200, "text/plain", rgbcolor );
    DBG_OUTPUT_PORT.print("/get_color: ");
    DBG_OUTPUT_PORT.println(rgbcolor);
  });

  server.on("/status", []() {
    getStatusJSON();
  });

  server.on("/off", []() {
    exit_func = true;
    mode = OFF;
    getArgs();
    getStatusJSON();
  });

  server.on("/all", []() {
    exit_func = true;
    mode = ALL;
    getArgs();
    getStatusJSON();
  });

  server.on("/wipe", []() {
    exit_func = true;
    mode = WIPE;
    getArgs();
    getStatusJSON();
  });

  server.on("/rainbow", []() {
    exit_func = true;
    mode = RAINBOW;
    getArgs();
    getStatusJSON();
  });

  server.on("/rainbowCycle", []() {
    exit_func = true;
    mode = RAINBOWCYCLE;
    getArgs();
    getStatusJSON();
  });

  server.on("/theaterchase", []() {
    exit_func = true;
    mode = THEATERCHASE;
    getArgs();
    getStatusJSON();
  });

  server.on("/theaterchaseRainbow", []() {
    exit_func = true;
    mode = THEATERCHASERAINBOW;
    getArgs();
    getStatusJSON();
  });

  server.on("/tv", []() {
    exit_func = true;
    mode = TV;
    getArgs();
    getStatusJSON();
  });

  server.on("/get_modes", []() {
    getModesJSON();
  });

  server.on("/set_mode", []() {
    getArgs();
    mode = SET_MODE;
    getStatusJSON();
  });

  server.begin();
}


void loop() {
  server.handleClient();
  webSocket.loop();
  #ifdef ENABLE_OTA
    ArduinoOTA.handle();
  #endif

  // Simple statemachine that handles the different modes
  if (mode == SET_MODE) {
    DBG_OUTPUT_PORT.printf("SET_MODE: %d %d\n", ws2812fx_mode, mode);
    strip.setMode(ws2812fx_mode);
    mode = HOLD;
  }
  if (mode == OFF) {
    strip.setColor(0,0,0);
    strip.setMode(FX_MODE_STATIC);
    // mode = HOLD;
  }
  if (mode == ALL) {
    strip.setColor(main_color.red, main_color.green, main_color.blue);
    strip.setMode(FX_MODE_STATIC);
    mode = HOLD;
  }
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
  if (mode == THEATERCHASERAINBOW) {
    strip.setMode(FX_MODE_THEATER_CHASE_RAINBOW);
    mode = HOLD;
  }
  if (mode == HOLD) {
    if (exit_func) {
      exit_func = false;
    }
  }
  if (mode == TV) {
    tv();
  }

  if (mode != TV) {
    strip.service();
  }
}
