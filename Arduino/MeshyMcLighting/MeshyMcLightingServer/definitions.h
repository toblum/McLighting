///////// Settings for Mesh Network ///////////
#define   MESH_PREFIX     "McLightingMesh"
#define   MESH_PASSWORD   "mclighting"
#define   MESH_PORT       5555
//////////// MUST match with client ///////////

/////////// Settings for WiFi Network for Internet/MQTT etc //////////////////////
// Enter values below, but not necessary as AsyncWiFiManager takes care of this //
#define   STATION_SSID     "WiFi_SSID"
#define   STATION_PASSWORD "WiFi Password"
#define   STATION_WIFI_CHANNEL 6
///////////////////////////    ^Enter WiFi channel set on your ROUTER /////////

///////// LED Settings ////////////////
//#define USE_WS2812FX                  // Uses WS2812FX
#define USE_NEOANIMATIONFX            // Uses NeoAnimationFX, PIN is ignored & set to RX/GPIO3

#define LED_PIN 14                    // PIN (14 / D5) where neopixel / WS2811 strip is attached
#define NUMLEDS 24                    // Number of leds in the strip
#define BUTTON 0                      // Input pin (4 / D2) for switching the LED strip on / off, connect this PIN to ground to trigger button.

///////// McLighting Settings //////////
#define HOSTNAME "MeshyMcLighting"    // Name that shows up on WiFi router once connected to STATION_SSID

#define ENABLE_BUTTON                 // If defined, enable button handling code, see: https://github.com/toblum/McLighting/wiki/Button-control
#define ENABLE_AMQTT                  // If defined, enable AMQTT client code, see: https://github.com/toblum/McLighting/wiki/MQTT-API
//#define ENABLE_HOMEASSISTANT          // If defined, enable Homeassistant integration, ENABLE_AMQTT must be active
//#define MQTT_HOME_ASSISTANT_SUPPORT   // If defined, use AMQTT and select Tools -> IwIP Variant -> Higher Bandwidth
#define ENABLE_STATE_SAVE_SPIFFS      // Saves LED state to SPIFFs
#define SERIALDEBUG                   // Un/comment to enable debug messages in Serial

//////////////////////////////////////////////////////////////////////////////////////
#ifdef ENABLE_AMQTT
  String mqtt_intopic = String(HOSTNAME) + "/in";
  String mqtt_outtopic = String(HOSTNAME) + "/out";
  uint8_t qossub = 0; // AMQTT can sub qos 0 or 1 or 2
  uint8_t qospub = 0; // AMQTT can pub qos 0 or 1 or 2
  
  #ifdef ENABLE_HOMEASSISTANT
    String mqtt_ha = "home/" + String(HOSTNAME) + "_ha/";
    String mqtt_ha_state_in = mqtt_ha + "state/in";
    String mqtt_ha_state_out = mqtt_ha + "state/out";
  #endif
  
  //#define ENABLE_MQTT_HOSTNAME_CHIPID          // Uncomment/comment to add ESPChipID to end of MQTT hostname
  #ifdef ENABLE_MQTT_HOSTNAME_CHIPID
    const char* mqtt_clientid = String(String(HOSTNAME) + "-" + String(ESP.getChipId())).c_str(); // MQTT ClientID
  #else
    const char* mqtt_clientid = HOSTNAME;          // MQTT ClientID
  #endif
  
  char mqtt_host[64] = "";
  char mqtt_port[6] = "";
  char mqtt_user[32] = "";
  char mqtt_pass[32] = "";
#endif

#define DEFAULT_COLOR 0xFF5900
#define DEFAULT_BRIGHTNESS 196
#define DEFAULT_SPEED 1000
#define DEFAULT_MODE 0

// parameters for automatically cycling favorite patterns
uint32_t autoParams[][4] = { // color, speed, mode, duration (seconds)
  {0xff0000, 200,  1,  5.0}, // blink red for 5 seconds
  {0x00ff00, 200,  3, 10.0}, // wipe green for 10 seconds
  {0x0000ff, 200, 11,  5.0}, // dual scan blue for 5 seconds
  {0x0000ff, 200, 42, 15.0}  // fireworks for 15 seconds
};
int autoCount;

///////////////////////////////////////
enum MODE { SET_MODE, HOLD, OFF, SETCOLOR, SETSPEED, BRIGHTNESS, CUSTOM };
MODE mode = SET_MODE; 
MODE prevmode = mode;

uint16_t ws2812fx_speed = DEFAULT_SPEED;   // Global variable for storing the delay between color changes --> smaller == faster
uint8_t brightness = DEFAULT_BRIGHTNESS;       // Global variable for storing the brightness (255 == 100%)
uint8_t ws2812fx_mode = 0;      // Helper variable to set WS2812FX modes
const char* on_cmd = "ON";
const char* off_cmd = "OFF";
bool stateOn = false;
bool animation_on = false;
uint16_t color_temp = 327; // min is 154 and max is 500
    
struct ledstate {           // Data structure to store a state of a single led
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

typedef struct ledstate LEDState;     // Define the datatype LEDState
LEDState ledstates[NUMLEDS];          // Get an array of led states to store the state of the whole strip
LEDState main_color = { ((DEFAULT_COLOR >> 16) & 0xFF), ((DEFAULT_COLOR >> 8) & 0xFF), (DEFAULT_COLOR & 0xFF) };  // Store the "main color" of the strip used in single color modes

String wifi_ssid = STATION_SSID;
String wifi_pwd = STATION_PASSWORD;
uint8_t wifi_channel = STATION_WIFI_CHANNEL;

extern const char index_html[];
extern const char main_js[];
extern const char uploadspiffs_html[];
extern const char update_html[];

#if defined(USE_NEOANIMATIONFX) and defined(USE_WS2812FX)
#error "Cant have both NeoAnimationFX and WS2812FX enabled. Choose either one."
#endif
#if !defined(USE_NEOANIMATIONFX) and !defined(USE_WS2812FX)
#error "Need to either use NeoAnimationFX and WS2812FX mode."
#endif
#if (defined(ENABLE_HOMEASSISTANT) and !defined(ENABLE_AMQTT))
#error "To use HA, you have enable AsyncMQTT"
#endif
#ifdef ENABLE_STATE_SAVE_SPIFFS
bool updateFS = false;
#endif

// Button handling
#ifdef ENABLE_BUTTON
  #define BTN_MODE_SHORT  "STA| 1|  0|245|196|255|255|255"   // Static white
  #define BTN_MODE_MEDIUM "STA| 1| 48|245|196|255|102|  0"   // Fire flicker
  #define BTN_MODE_LONG   "STA| 1| 46|253|196|255|102|  0"   // Fireworks random

  unsigned long keyPrevMillis = 0;
  const unsigned long keySampleIntervalMs = 25;
  byte longKeyPressCountMax = 80;       // 80 * 25 = 2000 ms
  byte mediumKeyPressCountMin = 20;     // 20 * 25 = 500 ms
  byte KeyPressCount = 0;
  byte prevKeyState = HIGH;             // button is active low
  boolean buttonState = false;
#endif

#ifdef SERIALDEBUG
  #define         DEBUG_PRINT(x)                 Serial.print(x)
  #define         DEBUG_PRINTLN(x)               Serial.println(x)
  #define         DEBUG_PRINTF(x,y)              Serial.printf(x,y)
  #define         DEBUG_PRINTF3(x,y,z)           Serial.printf(x,y,z)
  #define         DEBUG_PRINTF4(x,y,z,a)         Serial.printf(x,y,z,a)
  #define         DEBUG_PRINTF5(x,y,z,a,b)       Serial.printf(x,y,z,a,b)
  #define         DEBUG_PRINTF6(x,y,z,a,b,c)     Serial.printf(x,y,z,a,b,c)
  #define         DEBUG_PRINTF7(x,y,z,a,b,c,d)   Serial.printf(x,y,z,a,b,c,d)
  #define         DEBUG_PRINTF8(x,y,z,a,b,c,d,e) Serial.printf(x,y,z,a,b,c,d,e)
#else
  #define         DEBUG_PRINT(x)
  #define         DEBUG_PRINTLN(x)
  #define         DEBUG_PRINTF(x,y)
  #define         DEBUG_PRINTF3(x,y,z)
  #define         DEBUG_PRINTF4(x,y,z,a)
  #define         DEBUG_PRINTF5(x,y,z,a,b)
  #define         DEBUG_PRINTF6(x,y,z,a,b,c)
  #define         DEBUG_PRINTF7(x,y,z,a,b,c,d)
  #define         DEBUG_PRINTF8(x,y,z,a,b,c,d,e)
#endif
