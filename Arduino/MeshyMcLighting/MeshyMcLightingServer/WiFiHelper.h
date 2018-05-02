void handleHeap(AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(ESP.getFreeHeap()));
}

void handleRoot(AsyncWebServerRequest *request){
  request->send_P(200, "text/html", index_html);
}

/*
//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
//Custom WiFi Scanning Webpage attempt by @debsahu for AsyncWebServer
void handleScanNet(AsyncWebServerRequest *request){
  String reply;
  reply += "<html lang='en'>";
  reply += "<head>";
  reply += "<meta http-equiv='Content-Type' content='text/html; charset=utf-8' />";
  reply += "<meta name='viewport' content='width=device-width' />";
  reply +="<style>";
  reply += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
  reply += "</style>";
  reply += "</head><body>";

  reply += "<script>";
  reply += "function select_wifi_channel(name){";
  reply +=   "var wifi_channels = document.getElementById('wifi_channel_list');";
  reply +=   "for (var i=0;i<wifi_channels.length;i++){";
  reply +=     "listboxname = wifi_channels[i].innerHTML;";
  reply +=     "if(listboxname == name){";
  reply +=       "wifi_channels.selectedIndex = i;";
  reply +=       "break;";
  reply +=     "}";
  reply +=   "}";
  reply += "}";
  reply += "</script>";
  reply += "<br>";  
  reply += "<form action='/scan' method='POST' id='scan'>";
  reply += "<center><select name='wifi_ssid_list' form_id='scan' onClick='select_wifi_channel(this.value);' onChange='wifi_ssid.value=this.value;wifi_channel.value=wifi_channel_list.options[this.selectedIndex].value;'>";
  
  int n = WiFi.scanComplete();
  String channel_number = "<select name='wifi_channel_list' form_id='scan'>";
  //String channel_number = "<select name='wifi_channel_list' form_id='scan' hidden='true'>";
  if(n == -2){
    WiFi.scanNetworks(true);
  } else if(n){
    for (uint8_t i = 0; i < n; ++i){
      reply += "<option value='";
      channel_number += "<option value='";
      reply += WiFi.SSID(i) + "'>";
      reply += WiFi.SSID(i);
      reply += "</option>";
      channel_number += String(WiFi.channel(i)) + "'>";
      channel_number += WiFi.SSID(i);
      channel_number += "</option>";
    }
    WiFi.scanDelete();
    if(WiFi.scanComplete() == -2){
      WiFi.scanNetworks(true);
    }
  }
  reply += "</select>";
  reply += channel_number;
  reply += "</select></center><br>";
  reply += "<center><input type='text' name='wifi_ssid' value='" + wifi_ssid + "placeholder='WiFi SSID' size='20' style='text-align:center;' autofocus>";
  reply += "<input type='text' name='wifi_channel' value='" + String(wifi_channel) + "'placeholder='Channel' size='2' style='text-align:center;'></center><br>";
  reply += "<center><input type='password' name='wifi_pwd' value='" + wifi_pwd + "' placeholder='WiFi Password' size='27' style='text-align:center;'></center><br>";
  #ifdef ENABLE_AMQTT
  reply += "<center><input type='text' name='mqtt_host' placeholder='MQTT Server' value='" + String(mqtt_host) + "' size='27' style='text-align:center;'></center><br>";
  reply += "<center><input type='text' name='mqtt_port' placeholder='MQTT Port' value='" + String(mqtt_port) + "' size='27' style='text-align:center;'></center><br>";
  reply += "<center><input type='text' name='mqtt_user' placeholder='MQTT User Name' value='" + String(mqtt_user) + "' size='27' style='text-align:center;'></center><br>";
  reply += "<center><input type='password' name='mqtt_pass' placeholder='MQTT Password' value='" + String(mqtt_pass) + "' size='27' style='text-align:center;'></center><br>";
  #endif
  reply += "<center><input type='submit' value='Save Values'></center>";
  reply += "</form><br>";
  reply += "<center><a href='/scan'>Scan Again?</a></center>";
  reply += "</body>";
  reply += "</html>";
  request->send(200, "text/html", reply);
  reply = String();
  channel_number = String();
}

void processScanNet(AsyncWebServerRequest *request){
  if (request->hasArg("wifi_ssid")){
    wifi_ssid = request->arg("wifi_ssid");
    DEBUG_PRINT("WiFi SSID: ");
    DEBUG_PRINTLN(wifi_ssid);
  }
  if (request->hasArg("wifi_channel")){
    String wifi_channel_str = request->arg("wifi_channel");
    wifi_channel = atoi(wifi_channel_str.c_str());
    DEBUG_PRINT("WiFi Channel: ");
    DEBUG_PRINTLN(wifi_channel);
  }
  if (request->hasArg("wifi_pwd")){
    wifi_pwd = request->arg("wifi_pwd");
    DEBUG_PRINT("WiFi Pass: ");
    DEBUG_PRINTLN(wifi_pwd);
  }
  #ifdef ENABLE_AMQTT
  if (request->hasArg("mqtt_host")){
    String mqtt_host_str = request->arg("mqtt_host");
    if(mqtt_host_str.length() > 0){
      strcpy(mqtt_host, mqtt_host_str.c_str());
    }else{
      //mqtt_host[0] = '\0';
      memset(mqtt_host, 0, sizeof(mqtt_host));
    }
    DEBUG_PRINT("MQTT Host: ");
    DEBUG_PRINTLN(mqtt_host_str);
  }
  if (request->hasArg("mqtt_port")){
    String mqtt_port_str = request->arg("mqtt_port");
    if(mqtt_port_str.length() > 0){
      strcpy(mqtt_port, mqtt_port_str.c_str());
    }else{
      //mqtt_port[0] = '\0';
      memset(mqtt_port, 0, sizeof(mqtt_port));
    }
    DEBUG_PRINT("MQTT Port: ");
    DEBUG_PRINTLN(mqtt_port_str);
  }
  if (request->hasArg("mqtt_user")){
    String mqtt_user_str = request->arg("mqtt_user");
    if(mqtt_user_str.length() > 0){
      strcpy(mqtt_user, mqtt_user_str.c_str());
    }else{
      //mqtt_user[0] = '\0';
      memset(mqtt_user, 0, sizeof(mqtt_user));
    }
    DEBUG_PRINT("MQTT User: ");
    DEBUG_PRINTLN(mqtt_user_str);
  }
  if (request->hasArg("mqtt_pass")){
    String mqtt_pass_str = request->arg("mqtt_pass");
    if(mqtt_pass_str.length() > 0){
      strcpy(mqtt_pass, mqtt_pass_str.c_str());
    }else{
      //mqtt_pass[0] = '\0';
      memset(mqtt_pass, 0, sizeof(mqtt_pass));
    }
    DEBUG_PRINT("MQTT Pass: ");
    DEBUG_PRINTLN(mqtt_pass_str);
  }
  #endif

  DEBUG_PRINTLN((writeConfigFS(true)) ? "Write Config Success!" : "Write Config Failed!" );
  request->send(200, "text/plain", "Rebooting");
  //ESP.reset();
}
*/

void handleUpload(AsyncWebServerRequest *request){
  request->send_P(200, "text/html", uploadspiffs_html);
}

void processUploadReply(AsyncWebServerRequest *request){
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "upload success");
  response->addHeader("Connection", "close");
  request->send(response);
}

void processUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
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
}

void handleUpdate(AsyncWebServerRequest *request){
  request->send(200, "text/html", update_html);
}

void processUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
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
}

void processUpdateReply(AsyncWebServerRequest *request){
  bool shouldReboot = !Update.hasError();
  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
  response->addHeader("Connection", "close");
  request->send(response);
  if(shouldReboot) ESP.reset();
}

void handlemainjs(AsyncWebServerRequest *request){
  request->send_P(200, "application/javascript", main_js);
}

void handleModes(AsyncWebServerRequest *request){
  request->send(SPIFFS, "/modes", "text/plain");
}

void handleSetMode(AsyncWebServerRequest *request){
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
}

void handleNotFound(AsyncWebServerRequest *request){
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
  else if (filename.endsWith("ico.gz"))
    ContentType = "image/x-icon";

  if (SPIFFS.exists(filename + ".gz") || SPIFFS.exists(filename)) {
    if(SPIFFS.exists(filename + ".gz")) filename += ".gz";
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filename, ContentType);
    if(filename.endsWith("ico.gz")) response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    return;
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
}

//Upload any file to SPIFFs supplied via POST, not safe!
//void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
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
//  }

#include <ESPAsyncWiFiManager.h>            //https://github.com/alanswx/ESPAsyncWiFiManager

DNSServer dns;

bool shouldSaveConfig = false;

bool readConfigFS();
bool writeConfigFS(bool saveConfig);

void configModeCallback(AsyncWiFiManager *myWiFiManager) {
  // ***************************************************************************
  // Callback for AsyncWiFiManager library when config mode is entered
  // ***************************************************************************
  DEBUG_PRINTLN("Entered config mode");
  DEBUG_PRINTLN(WiFi.softAPIP());
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID()); //if you used auto generated SSID, print it

  for (uint16_t i = 0; i < strip.numPixels(); i++)
    strip.setPixelColor(i, 0, 0, 255);
  strip.show();
}

void saveConfigCallback(void) {
  //callback notifying us of the need to save config
  DEBUG_PRINTLN("Should save config");
  shouldSaveConfig = true;
}

void connectToWiFi(void) {
  // ***************************************************************************
  // Setup: AsyncWiFiManager
  // ***************************************************************************
  DEBUG_PRINTLN((readConfigFS()) ? "WiFiManager config FS Read success!": "WiFiManager config FS Read failure!");
  
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(wifi_ssid.c_str(), wifi_pwd.c_str());
  
  AsyncWiFiManager wifiManager(&server, &dns); //Local intialization. Once its business is done, there is no need to keep it around
  //wifiManager.resetSettings(); //reset settings - for testing
  wifiManager.setAPCallback(configModeCallback);  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setConfigPortalTimeout(180); //sets timeout until configuration portal gets turned off, useful to make it all retry or go to sleep in seconds
  wifiManager.setSaveConfigCallback(saveConfigCallback); //set config save notify callback
  
  #ifdef ENABLE_AMQTT
    AsyncWiFiManagerParameter custom_mqtt_host("host", "MQTT hostname", mqtt_host, 64);
    AsyncWiFiManagerParameter custom_mqtt_port("port", "MQTT port", mqtt_port, 6);
    AsyncWiFiManagerParameter custom_mqtt_user("user", "MQTT user", mqtt_user, 32);
    AsyncWiFiManagerParameter custom_mqtt_pass("pass", "MQTT pass", mqtt_pass, 32);
    wifiManager.addParameter(&custom_mqtt_host);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
  #endif
  
  if (!wifiManager.autoConnect(HOSTNAME)) {
    DEBUG_PRINTLN("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  wifi_ssid = WiFi.SSID();
  wifi_pwd = WiFi.psk();
  if(WiFi.channel() != wifi_channel) shouldSaveConfig = true;
  wifi_channel = WiFi.channel();

  //Uncomment to debug
  //DEBUG_PRINTLN(wifi_ssid);
  //DEBUG_PRINTLN(wifi_pwd);
  //DEBUG_PRINTLN(wifi_channel);
  
  DEBUG_PRINT("COMPILE ALL CLIENTS WITH THIS CHANNEL #####> ");
  DEBUG_PRINT(wifi_channel);
  DEBUG_PRINTLN(" <#####");
  
  #ifdef ENABLE_AMQTT
    //read updated parameters
    strcpy(mqtt_host, custom_mqtt_host.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_user.getValue());
    strcpy(mqtt_pass, custom_mqtt_pass.getValue());
  #endif

  //save the custom parameters to FS
  DEBUG_PRINTLN((writeConfigFS(shouldSaveConfig)) ? "\nAsyncWiFiManager config FS Save success!": "\nAsyncWiFiManager config FS Save failure!");

  ETS_UART_INTR_DISABLE();
  wifi_station_disconnect(); // We disconnect from current WiFi so that Mesh networking will work
  ETS_UART_INTR_ENABLE();
  
  dns=DNSServer();
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP_STA);
}
