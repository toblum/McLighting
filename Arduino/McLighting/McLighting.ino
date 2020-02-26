#include "definitions.h"
// ***************************************************************************
// Load library "ticker" for blinking status led, mqtt send and save state
// ***************************************************************************
#include <Ticker.h>
#include "version.h"

// ***************************************************************************
// Load libraries for: WebServer / WiFiManager / WebSockets
// ***************************************************************************
#include <ESP8266WiFi.h>           //https://github.com/esp8266/Arduino

// needed for library WiFiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>           //https://github.com/tzapu/WiFiManager
#include <FS.h>

#include <ESP8266mDNS.h>
#include <WebSockets.h>            //https://github.com/Links2004/arduinoWebSockets
#include <WebSocketsServer.h>

#if defined(ENABLE_BUTTON_GY33)
// ***************************************************************************
// Load libraries for GY33 and initialize color sensor
// ***************************************************************************
  #include <GY33_MCU.h>            //https://github.com/FabLab-Luenen/GY33_MCU/archive/master.zip ; //https://github.com/pasko-zh/brzo_i2c
  GY33_MCU tcs;
#endif

// MQTT
#if defined(ENABLE_MQTT)
  #if ENABLE_MQTT == 0
// ***************************************************************************
// Load libraries for PubSubClient
// ***************************************************************************
    #include <WiFiClient.h>
    #include <PubSubClient.h>
    WiFiClient espClient;
    PubSubClient * mqtt_client;
  #endif

  #if ENABLE_MQTT == 1
// ***************************************************************************
// Load libraries for Amqtt
// ***************************************************************************
    #include <AsyncMqttClient.h>     //https://github.com/marvinroger/async-mqtt-client
                                     //https://github.com/me-no-dev/ESPAsyncTCP
    AsyncMqttClient * mqtt_client;
    WiFiEventHandler wifiConnectHandler;
    WiFiEventHandler wifiDisconnectHandler;
    Ticker mqttReconnectTimer;
    Ticker wifiReconnectTimer;
  #endif
  #if defined(ENABLE_HOMEASSISTANT)
    Ticker ha_send_data;
  #endif
#endif

#if defined(CUSTOM_WS2812FX_ANIMATIONS)
// ***************************************************************************
// Load libraries for E131 support
// ***************************************************************************
  #include <ESPAsyncUDP.h>         //https://github.com/me-no-dev/ESPAsyncUDP
  #include <ESPAsyncE131.h>        //https://github.com/forkineye/ESPAsyncE131
  ESPAsyncE131 * e131 = NULL;
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
#include "html_material_icons.h"
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
#include "WS2812FX.h"              // https://github.com/kitesurfer1404/WS2812FX
WS2812FX * strip = NULL;

#if defined(USE_WS2812FX_DMA)
  #include <NeoPixelBus.h>

  #if USE_WS2812FX_DMA == 0 // Uses GPIO3/RXD0/RX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Dma800KbpsMethod> * dma = NULL ; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Dma400KbpsMethod> * dma = NULL;  //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 1 // Uses UART1: GPIO1/TXD0/TX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart0800KbpsMethod> * dma = NULL; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart0400KbpsMethod> * dma = NULL; //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 2 // Uses UART2: GPIO2/TXD1/D4, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart1800KbpsMethod> * dma = NULL; //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart1400KbpsMethod> * dma = NULL; //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif

  void initDMA(uint16_t stripSize = NUMLEDS){
    if (dma != NULL) { delete(dma); }
  #if USE_WS2812FX_DMA == 0 // Uses GPIO3/RXD0/RX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Dma800KbpsMethod>(stripSize);  //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Dma400KbpsMethod>(stripSize);  //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 1 // Uses UART1: GPIO1/TXD0/TX, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart0800KbpsMethod>(stripSize); //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart0400KbpsMethod>(stripSize); //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
  #if USE_WS2812FX_DMA == 2 // Uses UART2: GPIO2/TXD1/D4, more info: https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods
    #if !defined(LED_TYPE_WS2811)
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart1800KbpsMethod>(stripSize); //800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
    #else
      dma = new NeoPixelBus<NeoRgbwFeature, NeoEsp8266Uart1400KbpsMethod>(stripSize); //400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
    #endif
  #endif
    dma->Begin();
  }

  void DMA_Show(void) {
    if(dma->CanShow()) {
      memcpy(dma->Pixels(), strip->getPixels(), dma->PixelsSize());
      dma->Dirty();
      dma->Show();
    }
  }
#endif

Ticker ticker;



void tick() {
  //toggle state
  uint16_t state = digitalRead(LED_BUILTIN);  // get the current state of GPIO1 pin
  digitalWrite(LED_BUILTIN, !state);     // set pin to the opposite state
}

#if defined(ENABLE_REMOTE)
  IRrecv irrecv(ENABLE_REMOTE);
  decode_results results;
#endif

// ***************************************************************************
// Saved state handling in WifiManager
// ***************************************************************************
// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String getValue(String data, char separator, uint8_t index)
{
  uint8_t found = 0;
  uint8_t strIndex[] = {0, -1};
  uint8_t maxIndex = data.length()-1;

  for(uint8_t i=0; i<=maxIndex && found<=index; i++){
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
  updateConfig = true;
}

// ***************************************************************************
// Include: Webserver
// ***************************************************************************
#include "spiffs_webserver.h"

// ***************************************************************************
// Include: Custom animations
// ***************************************************************************
#include "mode_custom_ws2812fx_animations.h" // Add animations in this file

// ***************************************************************************
// Include: helper functions
// ***************************************************************************
#include "helper_functions.h"

// ***************************************************************************
// Include: other functions
// ***************************************************************************
#include "json_functions.h"
#include "filesystem_functions.h"
#include "request_handlers.h"

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
  mqtt_clientid[sizeof(mqtt_clientid) - 1] = 0x00;
  snprintf(mqtt_will_topic, sizeof(mqtt_will_topic), "%s/config", mqtt_clientid);
  mqtt_will_topic[sizeof(mqtt_will_topic) - 1] = 0x00;
  snprintf(mqtt_intopic,  sizeof(mqtt_intopic),  "%s/in",  mqtt_clientid);
  mqtt_intopic[sizeof(mqtt_intopic) - 1] = 0x00;
  snprintf(mqtt_outtopic, sizeof(mqtt_outtopic), "%s/out", mqtt_clientid);
  mqtt_outtopic[sizeof(mqtt_outtopic) - 1] = 0x00;
  #if defined(MQTT_HOME_ASSISTANT_SUPPORT)
    snprintf(mqtt_ha_config, sizeof(mqtt_ha_config), "homeassistant/light/%s/config", mqtt_clientid);
    mqtt_ha_config[sizeof(mqtt_ha_config) - 1] = 0x00;
    snprintf(mqtt_ha_state_in,  sizeof(mqtt_ha_state_in),   "home/%s_ha/state/in",  mqtt_clientid);
    mqtt_ha_state_in[sizeof(mqtt_ha_state_in) - 1] = 0x00;
    snprintf(mqtt_ha_state_out, sizeof(mqtt_ha_state_out),  "home/%s_ha/state/out", mqtt_clientid);
    mqtt_ha_state_out[sizeof(mqtt_ha_state_out) - 1] = 0x00;
  #endif
  if ((strlen(mqtt_host) != 0) && (mqtt_port != 0)) {
    #if ENABLE_MQTT == 0
      DBG_OUTPUT_PORT.printf("MQTT active: %s:%d\r\n", mqtt_host, mqtt_port);
      mqtt_client->setServer(mqtt_host, mqtt_port);
      mqtt_client->setCallback(onMqttMessage);
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

#if defined(POWER_SUPPLY)
  pinMode(POWER_SUPPLY, OUTPUT); // output to control external power supply
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
  (readConfigFS()) ? DBG_OUTPUT_PORT.println("WiFiManager config FS read success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Read failure!");
  delay(250);
  (readStateFS()) ? DBG_OUTPUT_PORT.println("Strip state config FS read Success!") : DBG_OUTPUT_PORT.println("Strip state config FS read failure!");
  char _stripSize[6], _fx_options[5], _rgbOrder[5]; //needed tempararily for WiFiManager Settings
  WiFiManagerParameter custom_hostname("hostname", "Hostname", HOSTNAME, 64, " maxlength=64");
  #if defined(ENABLE_MQTT)
    char _mqtt_port[6]; //needed tempararily for WiFiManager Settings
    WiFiManagerParameter custom_mqtt_host("host", "MQTT hostname", mqtt_host, 64, " maxlength=64");
    sprintf(_mqtt_port, "%d", mqtt_port);
    WiFiManagerParameter custom_mqtt_port("port", "MQTT port", _mqtt_port, 5, " maxlength=5 type=\"number\"");
    WiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 32, " maxlength=32");
    WiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32, " maxlength=32 type=\"password\"");
  #endif
  sprintf(_stripSize, "%d", Config.stripSize);
  WiFiManagerParameter custom_strip_size("strip_size", "Number of LEDs", _stripSize, 4, " maxlength=4 type=\"number\"");
  #if !defined(USE_WS2812FX_DMA)
    char tmp_led_pin[3];
    sprintf(tmp_led_pin, "%d", Config.pin);
    WiFiManagerParameter custom_led_pin("led_pin", "LED GPIO", tmp_led_pin, 2, " maxlength=2 type=\"number\"");
  #endif
  sprintf(_rgbOrder, "%s", Config.RGBOrder);
  WiFiManagerParameter custom_rgbOrder("rgbOrder", "RGBOrder", _rgbOrder, 4, " maxlength=4");
  sprintf(_fx_options, "%d", segState.options);
  WiFiManagerParameter custom_fxoptions("fxoptions", "fxOptions", _fx_options, 3, " maxlength=3");
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
    //ESP.reset();  //Will be removed when upgrading to standalone offline McLightingUI version
    //delay(1000);  //Will be removed when upgrading to standalone offline McLightingUI version
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
    strcpy(_stripSize, custom_strip_size.getValue());
    Config.stripSize = constrain(atoi(custom_strip_size.getValue()), 1, MAXLEDS);
    #if !defined(USE_WS2812FX_DMA)
      checkPin(atoi(custom_led_pin.getValue()));
    #endif
    strcpy(_rgbOrder, custom_rgbOrder.getValue());
    checkRGBOrder(_rgbOrder);
    segState.options = atoi(custom_fxoptions.getValue());
    if (updateConfig) {
      (writeConfigFS(updateConfig)) ? DBG_OUTPUT_PORT.println("WiFiManager config FS Save success!"): DBG_OUTPUT_PORT.println("WiFiManager config FS Save failure!");
    }
  #endif

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
    ArduinoOTA.onProgress([](uint16_t progress, uint16_t total) {
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
#if !defined(USE_HTML_MIN_GZ)
  DBG_OUTPUT_PORT.print("New users: Open http://");
  DBG_OUTPUT_PORT.print(WiFi.localIP());
  DBG_OUTPUT_PORT.println("/upload to upload the webpages first.");
#endif
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

  #if defined(ENABLE_BUTTON_GY33)
    tcs.setConfig(MCU_LED_06, MCU_WHITE_ON);
//    delay(2000);
//    tcs.setConfig(MCU_LED_OFF, MCU_WHITE_OFF);
  #endif
  #if defined(ENABLE_REMOTE)
    irrecv.enableIRIn();  // Start the receiver
  #endif
  fx_speed = segState.speed[State.segment];
  brightness_trans = State.brightness;
  initStrip();
  strip->setBrightness(0);
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
        WiFi.hostname(HOSTNAME);
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

  if ((State.mode == OFF) && ((strip->getBrightness() == 0) || !Config.transEffect)) {
    if(strip->isRunning()) {
      strip->strip_off();        // Workaround: to be shure,
      delay(10);                 // that strip is really off. Sometimes strip->stop isn't enought
      strip->stop();             // should clear memory
      for (uint8_t i = 0; i < Config.segments; i++) {
        autoCount[i] = 0;
        autoDelay[i] = 0;
      }
    } else {
      if (prevmode != State.mode) {    // Start temporarily to clear strip
        strip->start();
        strip->strip_off();      // Workaround: to be shure,
        delay(10);               // that strip is really off. Sometimes strip->stop isn't enought
        strip->stop();           // should clear memory
        for (uint8_t i = 0; i < Config.segments; i++) {
          autoCount[i] = 0;
          autoDelay[i] = 0;
        }
      }
    }
  }

  if (State.mode == OFF) {
    if (prevmode != State.mode) {
      #if defined(ENABLE_MQTT)
         snprintf(mqtt_buf, sizeof(mqtt_buf), "OK =off", "");
      #endif
      #if defined(POWER_SUPPLY)
         digitalWrite(POWER_SUPPLY, !POWER_ON); // power off -> external power supply
      #endif
      if (Config.transEffect) {
        brightness_trans   = 0;
      }
    }
  }
#if defined(POWER_SUPPLY)
  if (State.mode != OFF) {
    if (prevmode != State.mode) {digitalWrite(POWER_SUPPLY, POWER_ON); } // power on -> external power supply
  }
#endif

  if (State.mode == SET) {
    State.mode = HOLD;
    // Segment
    if (prevsegment != State.segment) {
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK Ss%i", State.segment);
      #endif
      //prevsegment = State.segment;
    }
    // Mode
    if (segState.mode[State.segment] != fx_mode) {
      segState.mode[State.segment] = fx_mode;
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK /%i", segState.mode[State.segment]);
      #endif
      strip->strip_off();
      autoCount[State.segment] = 0;
      autoDelay[State.segment] = 0;
      //strip->setSpeed(State.segment, segState.speed[State.segment]);
      //strip->setColors(State.segment, segState.colors[State.segment]);
      strip->setMode(State.segment, segState.mode[State.segment]);
      //strip->trigger;
    }
    //Color
    /*if (memcmp(segmentState.colors[prevsegment)], strip->getColors(prevsegment), sizeof(segmentState.colors[prevsegment)])) != 0) {
     convertColors();
    }*/
    // Brightness
    if (strip->getBrightness() != State.brightness) {
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK %%%i", State.brightness);
      #endif
      brightness_trans = State.brightness;
    }
    // Speed
    if (fx_speed != segState.speed[State.segment]) {
      #if defined(ENABLE_MQTT)
        snprintf(mqtt_buf, sizeof(mqtt_buf), "OK ?%i", segState.speed[prevsegment]);
      #endif
    }
    prevmode = SET;
  }

  if ((State.mode == HOLD) || ((State.mode == OFF) && (strip->getBrightness() > 0) && Config.transEffect)) {
    if(!strip->isRunning()) strip->start();
    strip->service();
    bool playE131 = false;
    for (uint8_t i = 0; i < Config.segments; i++) {
      if (segState.mode[i] == FX_MODE_CUSTOM_0) { handleAutoPlay(i); }
      if (segState.mode[i] == FX_MODE_CUSTOM_3) { playE131 = true; }
    }
    if (playE131 == true) { handleE131Play(); }
  }

  if (prevmode != State.mode) {
    if (segState.mode[prevsegment] != FX_MODE_CUSTOM_0) {
      convertColors();
      if (memcmp(hexcolors_trans, strip->getColors(prevsegment), sizeof(hexcolors_trans)) != 0) {
        DBG_OUTPUT_PORT.println("Color changed!");
        trans_cnt_max = convertColorsFade(prevsegment);
        trans_cnt = 1;
        memcpy(segState.colors[prevsegment], hexcolors_trans, sizeof(hexcolors_trans));
      }
      strip->setSpeed(prevsegment, convertSpeed(fx_speed));
    }
    //strip->setBrightness(brightness_actual);
    #if defined(ENABLE_MQTT)
      #if ENABLE_MQTT == 0
        mqtt_client->publish(mqtt_outtopic, mqtt_buf);
      #endif
      #if ENABLE_MQTT == 1
        mqtt_client->publish(mqtt_outtopic, qospub, false, mqtt_buf);
      #endif
      #if defined(ENABLE_HOMEASSISTANT)
        if(ha_send_data.active()) ha_send_data.detach();
        ha_send_data.once(5, tickerSendState);
      #endif
    #endif
  }

  prevmode = State.mode;

  #if defined(ENABLE_STATE_SAVE)
    if (updateState){
      (writeStateFS(updateState)) ? DBG_OUTPUT_PORT.println("State FS Save Success!") : DBG_OUTPUT_PORT.println("State FS Save failure!");
    }
    if (updateSegState) {
      (writeSegmentStateFS(updateSegState, prevsegment)) ? DBG_OUTPUT_PORT.println("Segment State FS Save Success!") : DBG_OUTPUT_PORT.println("Segment State FS Save failure!");
    }
    if (updateConfig) {
      (writeConfigFS(updateConfig)) ? DBG_OUTPUT_PORT.println("Config FS Save success!"): DBG_OUTPUT_PORT.println("Config FS Save failure!");
    }
  #endif

  // Async color transition
  if ((segState.mode[prevsegment] != FX_MODE_CUSTOM_0) && (memcmp(hexcolors_trans, strip->getColors(prevsegment), sizeof(hexcolors_trans)) != 0)) {
    if (Config.transEffect) {
      if ((trans_cnt > 0) && (trans_cnt < trans_cnt_max)) {
        if (colorFadeDelay <= millis()) {
          uint32_t _hexcolors_new[3] = {};
          _hexcolors_new[0] = trans(hexcolors_trans[0], strip->getColors(prevsegment)[0], trans_cnt, trans_cnt_max);
          _hexcolors_new[1] = trans(hexcolors_trans[1], strip->getColors(prevsegment)[1], trans_cnt, trans_cnt_max);
          _hexcolors_new[2] = trans(hexcolors_trans[2], strip->getColors(prevsegment)[2], trans_cnt, trans_cnt_max);
          strip->setColors(prevsegment, _hexcolors_new);
          trans_cnt++;
          colorFadeDelay = millis() + TRANS_COLOR_DELAY;
          if (State.mode == HOLD) strip->trigger();
        }
      } else if (trans_cnt >= trans_cnt_max) {
        strip->setColors(prevsegment, hexcolors_trans);
        if (State.mode == HOLD) strip->trigger();
        DBG_OUTPUT_PORT.println("Color transition finished!");
        trans_cnt = 0;
      }
    } else {
      strip->setColors(prevsegment, hexcolors_trans);
      if (State.mode == HOLD) strip->trigger();
    }
  }
  // Async speed transition
  if ((segState.mode[prevsegment] != FX_MODE_CUSTOM_0) && (segState.mode[prevsegment] != FX_MODE_CUSTOM_3) && (fx_speed != segState.speed[prevsegment])) {
    if (Config.transEffect) {
    //if (true == false) {  // disabled for the moment
      if (speedFadeDelay <= millis()) {
        //DBG_OUTPUT_PORT.print("Speed trans actual: ");
        if (fx_speed < segState.speed[prevsegment]) {
          fx_speed++;
        }
        if (fx_speed > segState.speed[prevsegment]) {
          fx_speed--;
        }
        //DBG_OUTPUT_PORT.println(fx_speed);
        speedFadeDelay = millis() + TRANS_DELAY;
        strip->setSpeed(prevsegment, convertSpeed(fx_speed));
        if (State.mode == HOLD) strip->trigger();
      }
    } else {
       fx_speed = segState.speed[prevsegment];
       //DBG_OUTPUT_PORT.print("Speed actual: ");
       strip->setSpeed(prevsegment, convertSpeed(fx_speed));
       //DBG_OUTPUT_PORT.println(fx_speed);
       if (State.mode == HOLD) strip->trigger();
    }
  }

  // Async brightness transition
  if (strip->getBrightness() != brightness_trans) {
    if (Config.transEffect) {
      if(brightnessFadeDelay <= millis()) {
        if (strip->getBrightness() < brightness_trans) {
          strip->increaseBrightness(1);
        }
        if (strip->getBrightness() > brightness_trans) {
          strip->decreaseBrightness(1);
        }
        brightnessFadeDelay = millis() + TRANS_DELAY;
        if (State.mode == HOLD) strip->trigger();
      }
    } else {
       brightness_trans = State.brightness;
       strip->setBrightness(brightness_trans);
    }
  }

  // Segment change only if color and speed transitions are finished, because they are segment specific
  if (prevsegment != State.segment) {
    DBG_OUTPUT_PORT.println("Segment not equal");
    //if ((segState.mode[State.segment] == FX_MODE_CUSTOM_0) || (segState.mode[State.segment] == FX_MODE_CUSTOM_2) || (segState.mode[prevsegment] == FX_MODE_CUSTOM_0)) {
    if ((segState.mode[State.segment] == FX_MODE_CUSTOM_0) || (segState.mode[prevsegment] == FX_MODE_CUSTOM_0)) {
      fx_speed  = segState.speed[State.segment];
      DBG_OUTPUT_PORT.printf("Switched segment from: %i to %i", prevsegment, State.segment);
      prevsegment = State.segment;
    } else if ((memcmp(hexcolors_trans, strip->getColors(prevsegment), sizeof(hexcolors_trans)) == 0) && (fx_speed == segState.speed[prevsegment])) {
      memcpy(hexcolors_trans, segState.colors[State.segment], sizeof(hexcolors_trans));
      fx_speed  = segState.speed[State.segment];
      DBG_OUTPUT_PORT.printf("Switched segment from: %i to %i\r\n", prevsegment, State.segment);
      prevsegment = State.segment;
    }
  }


  #if defined(ENABLE_REMOTE)
    handleRemote();
  #endif
}
