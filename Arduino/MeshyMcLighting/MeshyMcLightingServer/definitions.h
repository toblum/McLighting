///////// Settings for Mesh Network ///////////
#define   MESH_PREFIX     "McLightingMesh"
#define   MESH_PASSWORD   "mclighting"
#define   MESH_PORT       5555
//////////// Must match with client ///////////

///////// Settings for WiFi Network for Internet/MQTT etc //////////////////////
#define   STATION_SSID     "WiFi_SSID"
#define   STATION_PASSWORD "WiFi Password"
#define   STATION_WIFI_CHANNEL 2 
///////////////////////////    ^Enter WiFi channel set on your ROUTER /////////

///////// LED Settings ////////////////
#define USE_WS2812FX                  // Uses WS2812FX
//#define USE_NEOANIMATIONFX            // Uses NeoAnimationFX, PIN is ignored & set to RX/GPIO3

#define LED_PIN 14                    // 0 = GPIO0, 2=GPIO2
#define NUMLEDS 24                    // Number of leds in the strip

///////// McLighting Settings //////////
#define HOSTNAME "MeshyMcLighting"

#define ENABLE_AMQTT                  // If defined, enable AMQTT client code, see: https://github.com/toblum/McLighting/wiki/MQTT-API
//#define ENABLE_HOMEASSISTANT          // If defined, enable Homeassistant integration, ENABLE_AMQTT must be active
//#define MQTT_HOME_ASSISTANT_SUPPORT   // If defined, use AMQTT and select Tools -> IwIP Variant -> Higher Bandwidth
#define ENABLE_STATE_SAVE_SPIFFS

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
  
  char mqtt_host[64] = "debsahu.ddns.net";
  char mqtt_port[6] = "1884";
  char mqtt_user[32] = "";
  char mqtt_pass[32] = "";
#endif

#define DEFAULT_COLOR 0xFF5900
#define DEFAULT_BRIGHTNESS 196
#define DEFAULT_SPEED 1000
#define DEFAULT_MODE 0

unsigned long auto_last_change = 0;
String modes = "";
uint8_t myModes[] = {}; // *** optionally create a custom list of effect/mode numbers
boolean auto_cycle = false;

enum MODE { SET_MODE, HOLD, OFF, SETCOLOR, SETSPEED, BRIGHTNESS, CUSTOM };
MODE mode = SET_MODE; 
MODE prevmode = mode;

int ws2812fx_speed = DEFAULT_SPEED;   // Global variable for storing the delay between color changes --> smaller == faster
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
