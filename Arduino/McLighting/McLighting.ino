#include "definitions.h"
#include "version.h"

// ***************************************************************************
// Load libraries for: WebServer / WiFiManager / WebSockets
// ***************************************************************************
#include <ESP8266WiFi.h>           //https://github.com/esp8266/Arduino

// needed for library WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>           //https://github.com/tzapu/WiFiManager

#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#if ENABLE_STATE_SAVE == 0 
    #include <EEPROM.h>
#endif
#include <WebSockets.h>            //https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsServer.h>
  
#if defined(ENABLE_BUTTON_GY33)
// ***************************************************************************
// Load libraries for GY33 and initialize color sensor
// ***************************************************************************
  #include <GY33_MCU.h>            //https://github.com/FabLab-Luenen/GY33_MCU/archive/master.zip ; //https://github.com/pasko-zh/brzo_i2c
  GY33_MCU tcs;
#endif

#include <ArduinoJson.h>         //https://github.com/bblanchon/ArduinoJson

// MQTT
#if defined(ENABLE_MQTT)
  #if ENABLE_MQTT == 0
// ***************************************************************************
// Load libraries for PubSubClient
// ***************************************************************************
    #include <PubSubClient.h>
    WiFiClient espClient;
    PubSubClient* mqtt_client = NULL;
  #endif
  
  #if ENABLE_MQTT == 1
// ***************************************************************************
// Load libraries for Amqtt
// ***************************************************************************
    #include <AsyncMqttClient.h>     //https://github.com/marvinroger/async-mqtt-client
                                     //https://github.com/me-no-dev/ESPAsyncTCP
    AsyncMqttClient* mqtt_client = NULL;
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;
  #endif
#endif

#if defined(ARDUINOJSON_VERSION)
  #if !(ARDUINOJSON_VERSION_MAJOR == 6 and ARDUINOJSON_VERSION_MINOR == 9)
    #error "Install ArduinoJson v6.9.x"
  #endif
#endif

#if defined(ENABLE_E131)
// ***************************************************************************
// Load libraries for E131 support
// ***************************************************************************
  #include <ESPAsyncUDP.h>         //https://github.com/me-no-dev/ESPAsyncUDP
  #include <ESPAsyncE131.h>        //https://github.com/forkineye/ESPAsyncE131
  ESPAsyncE131* e131 = NULL; //(END_UNIVERSE - START_UNIVERSE + 1);
#endif

#if defined(ENABLE_REMOTE)
// ***************************************************************************
// Load libraries for IR remote support
// ***************************************************************************
  #include <IRremoteESP8266.h>       //https://github.com/markszabo/IRremoteESP8266
  #include <IRrecv.h>
  #include <IRutils.h>
#endif

#if defined(USE_HTML_MIN_GZ)
#include "htm_index_gz.h" 
#include "htm_edit_gz.h" 
#endif


// ***************************************************************************
// Instanciate HTTP(80) / WebSockets(81) Server
// ***************************************************************************
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// ***************************************************************************
// Include: OTA
// ***************************************************************************
#if defined(ENABLE_OTA)
  #if ENABLE_OTA == 1
    #include <ESP8266HTTPUpdateServer.h>
    ESP8266HTTPUpdateServer httpUpdater;
  #endif
  #if ENABLE_OTA == 0
    #include <WiFiUdp.h>
    #include <ArduinoOTA.h>
  #endif
#endif

// ***************************************************************************
// Load and instanciate WS2812FX library
// ***************************************************************************
#include <WS2812FX.h>              // https://github.com/kitesurfer1404/WS2812FX
WS2812FX* strip = NULL;

#if defined(USE_WS2812FX_DMA)
  #include <NeoPixelBus.h>

  #if USE_WS2812FX_DMA == 0 // Uses GPIO3/RXD0/RX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811) 
      NeoEsp8266Dma800KbpsMethod* dma = NULL ; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoEsp8266Dma400KbpsMethod* dma = NULL;  //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 1 // Uses UART1: GPIO1/TXD0/TX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811) 
      NeoEsp8266Uart0800KbpsMethod* dma = NULL; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoEsp8266Uart0400KbpsMethod* dma = NULL; //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 2 // Uses UART2: GPIO2/TXD1/D4, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811) 
      NeoEsp8266Uart1800KbpsMethod* dma = NULL; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoEsp8266Uart1400KbpsMethod* dma = NULL; //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  
  void initDMA(uint16_t stripSize = NUMLEDS){
    if (dma) delete dma;
    uint8_t ledcolors = 3;
    if (strstr(WS2812FXStripSettings.RGBOrder, "W") != NULL) {
      ledcolors = 4;
    }
  #if USE_WS2812FX_DMA == 0 // Uses GPIO3/RXD0/RX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)  
      dma = new NeoEsp8266Dma800KbpsMethod(stripSize, ledcolors);  //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoEsp8266Dma400KbpsMethod(stripSize, ledcolors);  //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 1 // Uses UART1: GPIO1/TXD0/TX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811) 
      dma = new NeoEsp8266Uart0800KbpsMethod(stripSize, ledcolors); //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoEsp8266Uart0400KbpsMethod(stripSize, ledcolors); //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 2 // Uses UART2: GPIO2/TXD1/D4, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811) 
      dma = new NeoEsp8266Uart1800KbpsMethod(stripSize, ledcolors); //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoEsp8266Uart1400KbpsMethod(stripSize, ledcolors); //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
    dma->Initialize();
  }

  void DMA_Show(void) {
    if(dma->IsReadyToUpdate()) {
      memcpy(dma->getPixels(), strip->getPixels(), dma->getPixelsSize());
      dma->Update();
    }
  }
#endif

// ***************************************************************************
// Load library "ticker" for blinking status led, mqtt send and save state
// ***************************************************************************
#include <Ticker.h>
Ticker ticker;

#if defined(ENABLE_MQTT)
  #if ENABLE_MQTT == 1
    Ticker mqttReconnectTimer;
    Ticker wifiReconnectTimer;
  #endif
  #if defined(ENABLE_HOMEASSISTANT)
    Ticker ha_send_data;
  #endif
#endif

void tick() {
  //toggle state
  int state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

#if defined(ENABLE_REMOTE)
  IRrecv irrecv(ENABLE_REMOTE);
  decode_results results;
  
#endif

Ticker settings_save_state;

// ***************************************************************************
// Saved state handling in WifiManager
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
  String return_value = data.substring(strIndex[0], strIndex[1]);
  return_value.replace(" ", "");
  return found>index ? return_value : "";
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

#if defined(ENABLE_TV)
// ***************************************************************************
// Include: TV mode
// ***************************************************************************
  #include "mode_tv.h"
#endif

#if defined(CUSTOM_WS2812FX_ANIMATIONS)
// ***************************************************************************
// Include: Custom animations
// ***************************************************************************
  #include "mode_custom_ws2812fx_animations.h" // Add animations in this file
#endif


// function to Initialize the strip
void initStrip(uint16_t stripSize = WS2812FXStripSettings.stripSize, char RGBOrder[5] = WS2812FXStripSettings.RGBOrder, uint8_t pin = WS2812FXStripSettings.pin, uint8_t fxoptions = WS2812FXStripSettings.fxoptions ){
  if (strip != NULL) {
    DBG_OUTPUT_PORT.println("Deleting strip!");
    delete(strip);
    WS2812FXStripSettings.stripSize = stripSize;
    strcpy(WS2812FXStripSettings.RGBOrder, RGBOrder);
    WS2812FXStripSettings.pin = pin;
    WS2812FXStripSettings.fxoptions = fxoptions;
  }
  DBG_OUTPUT_PORT.println("Initializing strip!");
#if !defined(LED_TYPE_WS2811)
  strip = new WS2812FX(stripSize, pin, checkRGBOrder(RGBOrder) + NEO_KHZ800);
#else
  strip = new WS2812FX(stripSize, pin, checkRGBOrder(RGBOrder) + NEO_KHZ400);
#endif
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
  
#if defined(CUSTOM_WS2812FX_ANIMATIONS)
  if (heat != NULL) {
      delete(heat);
  }
  heat = new byte [WS2812FXStripSettings.stripSize];
#endif
#if defined(ENABLE_TV)
  if (ledStates != NULL) {
      delete(ledStates);
  }
  ledStates = new uint8_t [WS2812FXStripSettings.stripSize];
#endif
/*
  if (ledstates != NULL) {
    delete(ledstates);
  }
  ledStates = new LEDState ledstates[WS2812FXStripSettings.stripSize];
*/
  strip->init();
  #if defined(USE_WS2812FX_DMA)
    initDMA(stripSize);
    strip->setCustomShow(DMA_Show);
  #endif
  strip->setBrightness(brightness);
//parameters: index, start, stop, mode, color, speed, options
  strip->setSegment(0,  0,  WS2812FXStripSettings.stripSize - 1, ws2812fx_mode, hex_colors, convertSpeed(ws2812fx_speed), WS2812FXStripSettings.fxoptions);
#if defined(CUSTOM_WS2812FX_ANIMATIONS)
  strip->setCustomMode(0, F("Fire 2012"), myCustomEffect0);
//strip->setCustomMode(1, F("CustEffect"), myCustomEffect1); 
#endif
}

#if defined(ENABLE_MQTT)
void initMqtt() {
  DBG_OUTPUT_PORT.println("Initializing Mqtt_Client!");
  // ***************************************************************************
  // Configure MQTT
  // ***************************************************************************
  #if ENABLE_MQTT == 0
    mqtt_client = new PubSubClient(espClient);
  #endif
  #if ENABLE_MQTT == 1
    mqtt_client = new AsyncMqttClient();
  #endif

  #if defined(ENABLE_MQTT_HOSTNAME_CHIPID)
    snprintf(mqtt_clientid, sizeof(mqtt_clientid), "%s-%08X", HOSTNAME, ESP.getChipId());
  #else
    snprintf(mqtt_clientid, sizeof(mqtt_clientid), "%s", HOSTNAME);
  #endif
  snprintf(mqtt_will_topic, sizeof(mqtt_will_topic), "%s/config", HOSTNAME);
  snprintf(mqtt_intopic,  sizeof(mqtt_intopic),  "%s/in",  HOSTNAME);
  snprintf(mqtt_outtopic, sizeof(mqtt_outtopic), "%s/out", HOSTNAME);
  #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
    snprintf(mqtt_ha_config, sizeof(mqtt_ha_config), "homeassistant/light/%s/config", HOSTNAME);
    snprintf(mqtt_ha_state_in,  sizeof(mqtt_ha_state_in),   "home/%s_ha/state/in",  HOSTNAME);
    snprintf(mqtt_ha_state_out, sizeof(mqtt_ha_state_out),  "home/%s_ha/state/out", HOSTNAME);
  #endif
  if ((strlen(mqtt_host) != 0) && (mqtt_port != 0)) {
    #if ENABLE_MQTT == 0
      DBG_OUTPUT_PORT.printf("MQTT active: %s:%d\r\n", mqtt_host, mqtt_port);
      mqtt_client->setServer(mqtt_host, mqtt_port);
      mqtt_client->setCallback(mqtt_callback);
    #endif
    #if ENABLE_MQTT == 1   
      DBG_OUTPUT_PORT.printf("AMQTT active: %s:%d\r\n", mqtt_host, mqtt_port);
      mqtt_client->onConnect(onMqttConnect);
      mqtt_client->onDisconnect(onMqttDisconnect);
      mqtt_client->onMessage(onMqttMessage);
      if ((strlen(mqtt_user) != 0) || (strlen(mqtt_pass) != 0)) mqtt_client->setCredentials(mqtt_user, mqtt_pass);
      mqtt_client->setClientId(mqtt_clientid);
      mqtt_client->setWill(mqtt_will_topic, 2, true, mqtt_will_payload, 0);
      mqtt_client->setServer(mqtt_host, mqtt_port);
      connectToMqtt();
    #endif
  }
}
#endif


// ***************************************************************************
// MAIN Setup
// ***************************************************************************
void setup() {
//  system_update_cpu_freq(160);

  DBG_OUTPUT_PORT.begin(115200);
  delay(500);
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.println("Starting...Main Setup");
#if ENABLE_STATE_SAVE == 0   
  EEPROM.begin(512);
#endif
  // set builtin led pin as output
  pinMode(LED_BUILTIN, OUTPUT);
  // button pin setup
#if defined(ENABLE_BUTTON)
  DBG_OUTPUT_PORT.printf("Enabled Button Mode on PIN: %d\r\n", ENABLE_BUTTON);
  pinMode(ENABLE_BUTTON,INPUT_PULLUP);
#endif

#if defined(ENABLE_BUTTON_GY33)
  DBG_OUTPUT_PORT.printf("Enabled GY-33 Button Mode on PIN: %d\r\n", ENABLE_BUTTON_GY33);
  pinMode(ENABLE_BUTTON_GY33, INPUT_PULLUP);
  if (tcs.begin()) {
    DBG_OUTPUT_PORT.println("Found GY-33 sensor");
  } else {
    DBG_OUTPUT_PORT.println("No GY33 sensor found ... check your I2C connections");
  }
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
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    DBG_OUTPUT_PORT.printf("FS Usage: %d/%d bytes\r\n", fs_info.usedBytes, fs_info.totalBytes);
  }

  // ***************************************************************************
  // Setup: WiFiManager
  // ***************************************************************************
  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length

#if defined(ENABLE_STATE_SAVE)
  //Strip Config
  #if ENABLE_STATE_SAVE == 1
    (readConfigFS()) ? DBG_OUTPUT_PORT.println("WiFiManager config FS read success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Read failure!");
    delay(500);
    (readStateFS()) ? DBG_OUTPUT_PORT.println("Strip state config FS read Success!") : DBG_OUTPUT_PORT.println("Strip state config FS read failure!");
  #endif
  #if ENABLE_STATE_SAVE == 0
    (setConfByConfString(readEEPROM(0, 222)))? DBG_OUTPUT_PORT.println("WiFiManager config EEPROM read success!"): DBG_OUTPUT_PORT.println("WiFiManager config EEPROM Read failure!");
    (setModeByStateString(readEEPROM(256, 66)))? DBG_OUTPUT_PORT.println("Strip state config EEPROM read Success!") : DBG_OUTPUT_PORT.println("Strip state config EEPROM read failure!");
  #endif
  char tmp_strip_size[6], tmp_fxoptions[5], tmp_rgbOrder[5]; //needed tempararily for WiFiManager Settings
  WiFiManagerParameter custom_hostname("hostname", "Hostname", HOSTNAME, 64, " maxlength=64");
  #if defined(ENABLE_MQTT)
    char tmp_mqtt_port[6]; //needed tempararily for WiFiManager Settings
    WiFiManagerParameter custom_mqtt_host("host", "MQTT hostname", mqtt_host, 64, " maxlength=64");
    snprintf(tmp_mqtt_port, sizeof(tmp_mqtt_port), "%d", mqtt_port);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", tmp_mqtt_port, 5, " maxlength=5 type=\"number\"");
    WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 32, " maxlength=32");
    WiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32, " maxlength=32 type=\"password\"");
  #endif
  snprintf(tmp_strip_size, sizeof(tmp_strip_size), "%d", WS2812FXStripSettings.stripSize);
  WiFiManagerParameter custom_strip_size("strip_size", "Number of LEDs", tmp_strip_size, 4, " maxlength=4 type=\"number\"");
  #if !defined(USE_WS2812FX_DMA)
    char tmp_led_pin[3];
    snprintf(tmp_led_pin, sizeof(tmp_led_pin), "%d", WS2812FXStripSettings.pin);
    WiFiManagerParameter custom_led_pin("led_pin", "LED GPIO", tmp_led_pin, 2, " maxlength=2 type=\"number\"");
  #endif
  snprintf(tmp_rgbOrder, sizeof(tmp_rgbOrder), "%s", WS2812FXStripSettings.RGBOrder);
  WiFiManagerParameter custom_rgbOrder("rgbOrder", "RGBOrder", tmp_rgbOrder, 4, " maxlength=4");
  snprintf(tmp_fxoptions, sizeof(tmp_fxoptions), "%d", WS2812FXStripSettings.fxoptions);
  WiFiManagerParameter custom_fxoptions("fxoptions", "fxOptions", tmp_fxoptions, 3, " maxlength=3");
#endif


  //Local intialization. Once its business is done, there is no need to keep it around
  wifi_station_set_hostname(const_cast<char*>(HOSTNAME));
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
   
  wifiManager.addParameter(&custom_hostname);
  #if defined(ENABLE_MQTT)
    //add all your parameters here
    wifiManager.addParameter(&custom_mqtt_host);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
  #endif
  wifiManager.addParameter(&custom_strip_size);
  #if !defined(USE_WS2812FX_DMA)
     wifiManager.addParameter(&custom_led_pin);
  #endif
  wifiManager.addParameter(&custom_rgbOrder);
  wifiManager.addParameter(&custom_fxoptions);
    
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  
  // Uncomment if you want to restart ESP8266 if it cannot connect to WiFi.
  // Value in brackets is in seconds that WiFiManger waits until restart
#if defined(WIFIMGR_PORTAL_TIMEOUT)
  wifiManager.setConfigPortalTimeout(WIFIMGR_PORTAL_TIMEOUT);
#endif

  // Uncomment if you want to set static IP 
  // Order is: IP, Gateway and Subnet 
#if defined(WIFIMGR_SET_MANUAL_IP)
  wifiManager.setSTAStaticIPConfig(IPAddress(_ip[0], _ip[1], _ip[2], _ip[3]), IPAddress(_gw[0], _gw[1], _gw[2], _gw[3]), IPAddress(_sn[0], _sn[1], _sn[2], _sn[3]));
#endif

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect(HOSTNAME)) {
    DBG_OUTPUT_PORT.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();  //Will be removed when upgrading to standalone offline McLightingUI version
    delay(1000);  //Will be removed when upgrading to standalone offline McLightingUI version
  }
  
  //save the custom parameters to FS/EEPROM
  #if defined(ENABLE_STATE_SAVE)
    strcpy(HOSTNAME, custom_hostname.getValue());
    #if defined(ENABLE_MQTT)
      //read updated parameters
      strcpy(mqtt_host, custom_mqtt_host.getValue());
      mqtt_port = atoi(custom_mqtt_port.getValue());
      strcpy(mqtt_user, custom_mqtt_user.getValue());
      strcpy(mqtt_pass, custom_mqtt_pass.getValue());
    #endif
    strcpy(tmp_strip_size, custom_strip_size.getValue());
    WS2812FXStripSettings.stripSize = atoi(custom_strip_size.getValue());
    #if !defined(USE_WS2812FX_DMA)
      checkPin(atoi(custom_led_pin.getValue()));
    #endif
    strcpy(tmp_rgbOrder, custom_rgbOrder.getValue());
    checkRGBOrder(tmp_rgbOrder);
    WS2812FXStripSettings.fxoptions = atoi(custom_fxoptions.getValue());
    #if ENABLE_STATE_SAVE == 1
      (writeConfigFS(shouldSaveConfig)) ? DBG_OUTPUT_PORT.println("WiFiManager config FS Save success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Save failure!");
      (writeStateFS(shouldSaveConfig)) ? DBG_OUTPUT_PORT.println("State config FS Save success!"): DBG_OUTPUT_PORT.println("State config FS Save failure!");
    #endif
    #if ENABLE_STATE_SAVE == 0
      if (shouldSaveConfig) {
        char last_conf[223];
        DBG_OUTPUT_PORT.println("Saving WiFiManager config");
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

  initStrip();
 
  //if you get here you have connected to the WiFi
  DBG_OUTPUT_PORT.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(LED_BUILTIN, LOW);
  //switch LED off
  //digitalWrite(LED_BUILTIN, HIGH);

#if defined(ENABLE_OTA)
  #if ENABLE_OTA == 0
  // ***************************************************************************
  // Configure Arduino OTA
  // ***************************************************************************
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
  #if ENABLE_OTA == 1
    httpUpdater.setup(&server, "/update");
  #endif
#endif

#if defined(ENABLE_MQTT)
  initMqtt();
#endif

#if ENABLE_MQTT == 1
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
#endif
  
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

#include "rest_api.h"

  server.begin();

  // Start MDNS service
  if (mdns_result) {
    MDNS.addService("http", "tcp", 80);
  }

  #if defined(ENABLE_E131)
  // Choose one to begin listening for E1.31 data
  e131 = new ESPAsyncE131(END_UNIVERSE - START_UNIVERSE + 1);
  // if (e131.begin(E131_UNICAST))                              // Listen via Unicast
  if (e131->begin(E131_MULTICAST, START_UNIVERSE, END_UNIVERSE)) {// Listen via Multicast
      DBG_OUTPUT_PORT.println(F("Listening for data..."));
  } else {
      DBG_OUTPUT_PORT.println(F("*** e131.begin failed ***"));
  }
  #endif
 DBG_OUTPUT_PORT.println("You are here!: 1117"); 
  prevmode = mode;
  
  #if defined(ENABLE_BUTTON_GY33)
    tcs.setConfig(MCU_LED_06, MCU_WHITE_ON);
//    delay(2000);
//    tcs.setConfig(MCU_LED_OFF, MCU_WHITE_OFF);
  #endif
  #if defined(ENABLE_REMOTE)
    irrecv.enableIRIn();  // Start the receiver
    snprintf(last_state, sizeof(last_state), "STA|%2d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d", mode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue, main_color.white, back_color.red, back_color.green, back_color.blue, back_color.white, xtra_color.red, xtra_color.green, xtra_color.blue,xtra_color.white);
  #endif
  DBG_OUTPUT_PORT.println("finished Main Setup!");
}

// ***************************************************************************
// MAIN Loop
// ***************************************************************************
void loop() {
  #if defined(ENABLE_BUTTON)
    button();
  #endif

  #if defined(ENABLE_BUTTON_GY33)
    button_gy33();
  #endif 

  server.handleClient();
  webSocket.loop();

  #if defined(ENABLE_OTA)
    #if ENABLE_OTA == 0
      ArduinoOTA.handle();
    #endif
  #endif

  #if defined(ENABLE_MQTT)
    #if ENABLE_MQTT == 0
      if (WiFi.status() != WL_CONNECTED) {
        #if defined(ENABLE_HOMEASSISTANT)
           ha_send_data.detach();
        #endif
        DBG_OUTPUT_PORT.println("WiFi disconnected, reconnecting!");
        WiFi.disconnect();
        WiFi.setSleepMode(WIFI_NONE_SLEEP);
        WiFi.mode(WIFI_STA);
        WiFi.begin();
      } else {
        if ((strlen(mqtt_host) != 0) && (mqtt_port != 0) && (mqtt_reconnect_retries < MQTT_MAX_RECONNECT_TRIES)) {
          if (!mqtt_client->connected()) {
            #if defined(ENABLE_HOMEASSISTANT)
             ha_send_data.detach();
            #endif
            DBG_OUTPUT_PORT.println("MQTT disconnected, reconnecting!");
            mqtt_reconnect();
          } else {
            mqtt_client->loop();
          }
        }
      }
    #endif

    #if defined(ENABLE_HOMEASSISTANT)
      if (new_ha_mqtt_msg) sendState();
    #endif
  #endif
          
  // ***************************************************************************
  // Simple statemachine that handles the different modes
  // *************************************************************************** 
  if ((mode != OFF) && (mode != TV) && (mode != E131)) { // strip->start() is only needed for modes with WS2812FX functionality
    if(!strip->isRunning()) strip->start();
  } 

  if ((mode == OFF) || (mode == TV) || (mode == E131)) {
    if(strip->isRunning()) {
      strip->strip_off();         // Workaround: to be shure,
      delay(10);                 // that strip is really off. Sometimes strip->stop isn't enought
      strip->stop();              // should clear memory
    } else {
      if (prevmode != mode) {    // Start temporarily to clear strip
        strip->start();
        strip->strip_off();       // Workaround: to be shure,
        delay(10);               // that strip is really off. Sometimes strip->stop isn't enought
        strip->stop();            // should clear memory
      }
    }
  }
    
  if (( mode == AUTO) || (mode == HOLD)) { // strip->service() is only needed for modes with WS2812FX functionality
    strip->service();
  }
  
  if ((prevmode == AUTO) && (mode != AUTO)) { handleAutoStop(); } // stop auto mode
  
  if (mode == OFF) {
    #if defined(ENABLE_MQTT)
      if (prevmode != mode) { snprintf(mqtt_buf, sizeof(mqtt_buf), "OK =off", ""); }
    #endif
  }
  
  if (mode == AUTO) {
    if (prevmode != mode) {
      handleAutoStart();
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK =auto", "");
      #endif
    }
  }
    
  #if defined(ENABLE_TV)
    if (mode == TV) {
      handleTV();
      #if defined(ENABLE_MQTT)
        if (prevmode != mode) { snprintf(mqtt_buf, sizeof(mqtt_buf), "OK =tv", ""); }
      #endif
    }
  #endif
  
  #if defined(ENABLE_E131)
    if (mode == E131) {
      handleE131();
      #if defined(ENABLE_MQTT)
        if (prevmode != mode) { snprintf(mqtt_buf, sizeof(mqtt_buf), "OK =e131", ""); }
      #endif
    }
  #endif
 
  if (mode == INIT_STRIP) {
     mode = prevmode;
     //ws2812fx_mode = strip->getMode();
     strip->strip_off();
     delay(10);
     if(strip->isRunning()) strip->stop();
     initStrip();
     prevmode = INIT_STRIP;
  }
  
  if (mode == SET_ALL) {
    mode = HOLD;
    if ((prevmode == OFF) || (prevmode == AUTO) || (prevmode == TV) || (prevmode == E131)) { setModeByStateString(last_state); }
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK /%i", ws2812fx_mode);
    #endif
    strip->setMode(ws2812fx_mode);
    convertColors();
    strip->setColors(0, hex_colors);
    strip->setSpeed(convertSpeed(ws2812fx_speed));
    strip->setBrightness(brightness);
    prevmode = SET_ALL;
    strip->trigger();
  }  
  
  if (mode == SET_MODE) {
    mode = HOLD;
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK /%i", ws2812fx_mode);
    #endif
    strip->setMode(ws2812fx_mode);
    prevmode = SET_MODE;
    strip->trigger();
  }
    
  if (mode == SET_COLOR) {
    convertColors();
    strip->setColors(0, hex_colors);
    mode = prevmode;
    prevmode = SET_COLOR;
    if (mode == HOLD) strip->trigger();
  }
  if (mode == SET_SPEED) {
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK ?%i", ws2812fx_speed);
    #endif
    strip->setSpeed(convertSpeed(ws2812fx_speed));
    mode = prevmode;
    prevmode = SET_SPEED;
    if (mode == HOLD) strip->trigger();
  }
  if (mode == SET_BRIGHTNESS) {
    #if defined(ENABLE_MQTT)
      snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %%%i", brightness);
    #endif
    strip->setBrightness(brightness);
    mode = prevmode;
    prevmode = SET_BRIGHTNESS;
    if (mode == HOLD) strip->trigger();
  }
 
  if (prevmode != mode) {  
    if (prevmode != AUTO) {  // do not save if AUTO Mode was set
      #if defined(ENABLE_STATE_SAVE)
        if(!settings_save_state.active()) settings_save_state.once(3, tickerSaveState);
      #endif
      snprintf(last_state, sizeof(last_state), "STA|%2d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d|%3d", prevmode, ws2812fx_mode, ws2812fx_speed, brightness, main_color.red, main_color.green, main_color.blue, main_color.white, back_color.red, back_color.green, back_color.blue, back_color.white, xtra_color.red, xtra_color.green, xtra_color.blue, xtra_color.white);
    }
    #if defined(ENABLE_MQTT)
      #if ENABLE_MQTT == 0
        mqtt_client->publish(mqtt_outtopic, mqtt_buf);
      #endif
      #if ENABLE_MQTT == 1
        mqtt_client->publish(mqtt_outtopic, qospub, false, mqtt_buf);
      #endif
      #if defined(ENABLE_HOMEASSISTANT)
        if(!ha_send_data.active())  ha_send_data.once(3, tickerSendState);
      #endif
    #endif
  }
  #if defined(ENABLE_STATE_SAVE)
    if (updateState){
    #if ENABLE_STATE_SAVE == 1
      (writeStateFS(true)) ? DBG_OUTPUT_PORT.println(" Success!") : DBG_OUTPUT_PORT.println(" Failure!");
    #endif
    #if ENABLE_STATE_SAVE == 0
      writeEEPROM(384, 66, last_state);  // 384 --> last_state (reserved 66 bytes)
      EEPROM.commit();
      updateState = false;
      settings_save_state.detach();
    #endif
  #endif
  }
  
  prevmode = mode;
  
  #if defined(ENABLE_REMOTE)
    handleRemote();
  #endif
}
