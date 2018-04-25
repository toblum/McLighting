///////// Settings for Mesh Network ///////////
#define   MESH_PREFIX     "McLightingMesh"
#define   MESH_PASSWORD   "mclighting"
#define   MESH_PORT       5555
#define   STATION_WIFI_CHANNEL 6

///////// LED Settings ////////////////
#define USE_WS2812FX                  // Uses WS2812FX
//#define USE_NEOANIMATIONFX            // Uses NeoAnimationFX, PIN is ignored & set to RX/GPIO3

#define LED_PIN 14                    // 0 = GPIO0, 2=GPIO2
#define NUMLEDS 24                    // Number of leds in the strip

///////// McLighting Settings //////////
const char HOSTNAME_BASE[] = "MeshyMcLighting";
const char* HOSTNAME = String(String(HOSTNAME_BASE) + "-" + String(ESP.getChipId())).c_str();

#define ENABLE_STATE_SAVE_SPIFFS

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

#if defined(USE_NEOANIMATIONFX) and defined(USE_WS2812FX)
#error "Cant have both NeoAnimationFX and WS2812FX enabled. Choose either one."
#endif
#if !defined(USE_NEOANIMATIONFX) and !defined(USE_WS2812FX)
#error "Need to either use NeoAnimationFX and WS2812FX mode."
#endif
#if (defined(ENABLE_HOMEASSISTANT) and !defined(ENABLE_AMQTT))
#error "To use HA, you have enable AsyncMQTT"
#endif
