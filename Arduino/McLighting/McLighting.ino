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


// ***************************************************************************
// Instanciate HTTP(80) / WebSockets(81) Server
// ***************************************************************************
ESP8266WebServer server ( 80 );
WebSocketsServer webSocket = WebSocketsServer(81);


// ***************************************************************************
// Load libraries / Instanciate Neopixel
// ***************************************************************************
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
	#include <avr/power.h>
#endif

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMLEDS, PIN, NEO_GRB + NEO_KHZ800);

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
	// Setup: Neopixel
	// ***************************************************************************
	strip.begin();
	strip.setBrightness(brightness);
	strip.show(); // Initialize all pixels to 'off'
	
	
	// ***************************************************************************
	// Setup: MDNS responder
	// ***************************************************************************  
	MDNS.begin(HOSTNAME);
	DBG_OUTPUT_PORT.print("Open http://");
	DBG_OUTPUT_PORT.print(HOSTNAME);
	DBG_OUTPUT_PORT.println(".local/edit to see the file browser");
	
	
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
	server.on("/edit", HTTP_GET, [](){
		if(!handleFileRead("/edit.htm")) server.send(404, "text/plain", "FileNotFound");
	});
	//create file
	server.on("/edit", HTTP_PUT, handleFileCreate);
	//delete file
	server.on("/edit", HTTP_DELETE, handleFileDelete);
	//first callback is called after the request has ended with all parsed arguments
	//second callback handles file uploads at that location
	server.on("/edit", HTTP_POST, [](){ server.send(200, "text/plain", ""); }, handleFileUpload);
	//get heap status, analog input value and all GPIO statuses in one json call
	server.on("/status", HTTP_GET, [](){
		String json = "{";
		json += "\"heap\":"+String(ESP.getFreeHeap());
		json += ", \"analog\":"+String(analogRead(A0));
		json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
		json += "}";
		server.send(200, "text/json", json);
		json = String();
	});
	
	
	//called when the url is not defined here
	//use it to load content from SPIFFS
	server.onNotFound([](){
		if(!handleFileRead(server.uri()))
			handleNotFound();
	});
	
	
	server.on("/upload", handleMinimalUpload);
	
	// ***************************************************************************
	// Setup: SPIFFS Webserver handler
	// ***************************************************************************
	server.on("/brightness", []() {
		brightness = server.arg("p").toInt();
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
	
	server.begin();
}

void loop() {
	server.handleClient();
	webSocket.loop();
	
	// Simple statemachine that handles the different modes
	if (mode == OFF) {
		//colorWipe(strip.Color(0, 0, 0), 50);
    uint16_t i;
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
		mode = HOLD;
	}
	if (mode == ALL) {
		uint16_t i;
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, main_color.red, main_color.green, main_color.blue);
		}
		strip.show();
		//mode = HOLD;
	}
	if (mode == WIPE) {
		colorWipe(strip.Color(main_color.red, main_color.green, main_color.blue), delay_ms);
	}
	if (mode == RAINBOW) {
		rainbow(delay_ms);
	}
	if (mode == RAINBOWCYCLE) {
		rainbowCycle(delay_ms);
	}
	if (mode == THEATERCHASE) {
		theaterChase(strip.Color(main_color.red, main_color.green, main_color.blue), delay_ms);
	}
	if (mode == THEATERCHASERAINBOW) {
		theaterChaseRainbow(delay_ms);
	}
	if (mode == HOLD) {
		if (exit_func) {
			exit_func = false;
		}
	}
	if (mode == TV) {
		tv();
	}
}
