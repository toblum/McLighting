#define USE_WS2812FX_DMA 0      // 0 = Used PIN is ignored & set to RX/GPIO3; 1 = Used PIN is ignored & set to TX/GPIO1; 2 = Uses PIN is ignored & set to D4/GPIO2;  Uses WS2812FX, see: https://github.com/kitesurfer1404/WS2812FX
                                // or comment it out
#if defined(USE_WS2812FX_DMA)
  #define MAXLEDS 384           // due to memory limit of esp8266 at the moment only 384 leds are supported in DMA Mode. More can crash if mqtt is used.
#else
  #define MAXLEDS 4096
#endif 
// Neopixel
#define LED_PIN 3          // PIN (15 / D8) where neopixel / WS2811 strip is attached; is configurable, if USE_WS2812FX_DMA is not defined. Just for the start
#define NUMLEDS 50         // Number of leds in the; is configurable just for the start 
#define RGBORDER "GRBW"    // RGBOrder; is configurable just for the start
#define FX_OPTIONS 48      // ws2812fx Options 48 = SIZE_SMALL + FADE_MEDIUM  is configurable just for the start; for WS2812FX setSegment OPTIONS, see: https://github.com/kitesurfer1404/WS2812FX/blob/master/extras/WS2812FX%20Users%20Guide.md
//#define LED_TYPE_WS2811    // Uncomment, if LED type uses 400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
#define LED_BUILTIN 2      // ESP-12F has the built in LED on GPIO2, see https://github.com/esp8266/Arduino/issues/2192
#define POWER_SUPPLY 12    // PIN (12 / D6) If defined, enable output to control external power supply 
#if defined(POWER_SUPPLY)
  #define POWER_ON   HIGH           // Define the output state to turn on the power supply, either HIGH or LOW.  Opposite will be uses for power off.
#endif 

char HOSTNAME[65] = "McLightingRGBW"; // Friedly hostname  is configurable just for the start. Hostname should not contain spaces as this can break Home Assistant discovery if used.

#define ENABLE_OTA 1                  // If defined, enable Arduino OTA code. If set to 0 enable Arduino OTA code, if set to 1 enable ESP8266HTTPUpdateServer OTA code.
#define ENABLE_MQTT 1                 // If defined use MQTT OR AMQTT, if set to 0 enable MQTT client code, see: https://github.com/toblum/McLighting/wiki/MQTT-API, if set to 1, enable Async MQTT code, see: https://github.com/marvinroger/async-mqtt-client
//#define ENABLE_MQTT_HOSTNAME_CHIPID   // Uncomment/comment to add ESPChipID to end of MQTT hostname
#define ENABLE_HOMEASSISTANT          // If defined, enable Homeassistant integration, ENABLE_MQTT must be active
#define MQTT_HOME_ASSISTANT_SUPPORT   // If defined, use AMQTT and select Tools -> IwIP Variant -> Higher Bandwidth
#define ENABLE_BUTTON 14              // If defined, enable button handling code, see: https://github.com/toblum/McLighting/wiki/Button-control, the value defines the input pin (14 / D5) for switching the LED strip on / off, connect this PIN to ground to trigger button.
//#define ENABLE_BUTTON_GY33 12         // If defined, enable button handling code for GY-33 color sensor to scan color. The value defines the input pin (12 / D6) for read color data with RGB sensor, connect this PIN to ground to trigger button.
#if defined(ENABLE_BUTTON_GY33)
  #define GAMMA 2.5                   // Gamma correction for GY-33 sensor
#endif
//#define ENABLE_REMOTE 13              // If defined, enable Remote Control via TSOP31238. The value defines the input pin (13 / D7) for TSOP31238 Out 

#define ENABLE_STATE_SAVE 1           // If defined, load saved state on reboot and save state. If set to 0 from EEPROM, if set to 1 from SPIFFS

#define ENABLE_LEGACY_ANIMATIONS      // Enable Legacy Animations
#define CUSTOM_WS2812FX_ANIMATIONS    // uncomment and put animations in "custom_ws2812fx_animations.h" 
#define ENABLE_E131                   // E1.31 implementation You have to uncomment #define USE_WS2812FX_DMA and set it to 0
#define ENABLE_TV                     // Enable TV Animation 
#define USE_HTML_MIN_GZ               // uncomment for using index.htm & edit.htm from PROGMEM instead of SPIFFs

#if defined(ENABLE_E131)
  #define MULTICAST false
  #define START_UNIVERSE 1            // First DMX Universe to listen for
      uint8_t END_UNIVERSE = START_UNIVERSE; // Total number of Universes to listen for, starting at UNIVERSE

#endif

#if defined(ENABLE_REMOTE)
  uint8_t  selected_color = 1;
  uint64_t last_remote_cmd;
  enum                     RMT_BTN {ON_OFF,    MODE_UP, MODE_DOWN,   RED_UP, RED_DOWN, GREEN_UP, GREEN_DOWN,  BLUE_UP, BLUE_DOWN, WHITE_UP, WHITE_DOWN, BRIGHTNESS_UP, BRIGHTNESS_DOWN, SPEED_UP, SPEED_DOWN,    COL_M,    COL_B,    COL_X, AUTOMODE,    CUST_1,   CUST_2,    CUST_3,   CUST_4,   CUST_5,          REPEATCMD, BTN_CNT};
  // Change your IR Commands here. You can see them in console, after you pressed a button on the remote
  uint64_t rmt_commands[BTN_CNT] = {0xF7C03F, 0xF7708F,  0xF7F00F, 0xF720DF, 0xF710EF, 0xF7A05F,   0xF7906F, 0xF7609F,  0xF750AF, 0xF7E01F,   0xF7D02F,      0xF730CF,        0xF7B04F, 0xF748B7,   0xF7C837, 0xF700FF, 0xF7807F, 0xF740BF, 0xF708F7,  0xF78877, 0xF728D7,  0xF7A857, 0xF76897, 0xF7E817, 0xFFFFFFFFFFFFFFFF};
#endif
//#define WIFIMGR_PORTAL_TIMEOUT 180
//#define WIFIMGR_SET_MANUAL_IP

#if defined(WIFIMGR_SET_MANUAL_IP)
  uint8_t _ip[4] = {192,168,0,128};
  uint8_t _gw[4] = {192,168,0,1};
  uint8_t _sn[4] = {255,255,255,0};
#endif

#if defined(MQTT_HOME_ASSISTANT_SUPPORT)
  #define MQTT_HOME_ASSISTANT_0_87_SUPPORT // Comment if using HA version < 0.87 
#endif

#if defined(USE_WS2812FX_DMA) && (USE_WS2812FX_DMA < 0 || USE_WS2812FX_DMA > 2)
#error "Definition of USE_WS2812FX_DMA is wrong!"
#endif

#if defined(ENABLE_MQTT) and ENABLE_MQTT < 0 and ENABLE_MQTT > 1
#error "Definition of ENABLE_MQTT is wrong!"
#endif

#if defined(ENABLE_MQTT) and ENABLE_MQTT < 0 and ENABLE_MQTT > 1
#error "Definition of ENABLE_MQTT is wrong!"
#endif

#if defined(ENABLE_HOMEASSISTANT) and !defined(ENABLE_MQTT)
#error "To use HA, you have to either enable PubCubClient or AsyncMQTT"
#endif
#if !defined(ENABLE_HOMEASSISTANT) and defined(MQTT_HOME_ASSISTANT_SUPPORT)
#error "To use HA support, you have to either enable Homeassistant component"
#endif

// parameters for automatically cycling favorite patterns
uint32_t autoParams[][6] = {   // main_color, back_color, xtra_color, speed, mode, duration (seconds)
  {0x00ff0000, 0x0000ff00, 0x00000000, 200,  1,  5000}, // blink red/geen for 5 seconds
  {0x0000ff00, 0x000000ff, 0x00000000, 200,  3, 10000}, // wipe green/blue for 10 seconds
  {0x000000ff, 0x00ff0000, 0x00000000,  60, 14, 10000}, // dual scan blue on red for 10 seconds
  {0x000000ff, 0x00ff0000, 0x00000000,  40, 45, 15000}, // fireworks blue/red for 15 seconds
  {0x00ff0000, 0x0000ff00, 0x000000ff,  40, 54, 15000}  // tricolor chase red/green/blue for 15 seconds
};

#if defined(ENABLE_MQTT)
  char mqtt_buf[80];
  char mqtt_will_topic[sizeof(HOSTNAME) + 7]; // Topic 'will' will be:HOSTNAME "/status";
  const char mqtt_will_payload[] = "OFFLINE";
  char mqtt_intopic[sizeof(HOSTNAME) + 3];      // Topic 'in' will be: <HOSTNAME>/in
  char mqtt_outtopic[sizeof(HOSTNAME) + 4];     // Topic 'out' will be: <HOSTNAME>/out
  bool mqtt_lwt_boot_flag = true;
  #if ENABLE_MQTT == 0
    #define MQTT_MAX_PACKET_SIZE 512
    #define MQTT_MAX_RECONNECT_TRIES 4
    uint8_t mqtt_reconnect_retries = 0;
    uint8_t qossub = 0; // PubSubClient can sub qos 0 or 1
  #endif

  #if ENABLE_MQTT == 1
    uint8_t qossub = 0; // AMQTT can sub qos 0 or 1 or 2
    uint8_t qospub = 0; // AMQTT can pub qos 0 or 1 or 2
  #endif

  #if defined(ENABLE_HOMEASSISTANT)
    char mqtt_ha_config[20 + sizeof(HOSTNAME) +  7];   // Topic config will be: "homeassistant/light/<HOSTNAME>/config"
    char mqtt_ha_state_in[5 + sizeof(HOSTNAME) + 12];  // Topic in will be: "home/<HOSTNAME>_ha/state/in"
    char mqtt_ha_state_out[5 + sizeof(HOSTNAME) + 13]; // Topic in will be: "home/<HOSTNAME>_ha/state/out"
    const char* on_cmd = "ON";
    const char* off_cmd = "OFF";
    bool new_ha_mqtt_msg = false;
    uint16_t color_temp = 327; // min is 154 and max is 500
  #endif

  char     mqtt_clientid[sizeof(HOSTNAME) + 9];
  char     mqtt_host[65] = "";    //is configurable just for the start
  uint16_t mqtt_port     = 1883;  //is configurable just for the start
  char     mqtt_user[33] = "";    //is configurable just for the start
  char     mqtt_pass[33] = "";    //is configurable just for the start
#endif


// ***************************************************************************
// Global variables / definitions
// ***************************************************************************
#define DBG_OUTPUT_PORT Serial  // Set debug output port

// List of all color modes
#if defined(ENABLE_LEGACY_ANIMATIONS)
  enum MODE {OFF, AUTO, TV, E131, CUSTOM, HOLD, SET_ALL, SET_MODE, SET_COLOR, SET_SPEED, SET_BRIGHTNESS, INIT_STRIP, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, TWINKLERANDOM, THEATERCHASERAINBOW};
#else
  enum MODE {OFF, AUTO, TV, E131, CUSTOM, HOLD, SET_ALL, SET_MODE, SET_COLOR, SET_SPEED, SET_BRIGHTNESS, INIT_STRIP};
#endif
MODE mode = SET_ALL;        // Standard mode that is active when software starts
MODE prevmode = mode;

uint8_t ws2812fx_speed = 196;  // Global variable for storing the delay between color changes --> smaller == faster
uint8_t brightness = 196;      // Global variable for storing the brightness (255 == 100%)
uint8_t ws2812fx_mode = 0;     // Global variable for storing the WS2812FX modes

uint32_t hex_colors[3] = {};   // Color array for setting WS2812FX
struct ledstate                // Data structure to store a state of a single led
{
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t white;
};

typedef struct ledstate LEDState;        // Define the datatype LEDState
uint8_t* ledstates;                      // Set a pointer to get an array of led states to store the state of the whole strip
LEDState main_color = { 255, 0, 0, 0 };  // Store the "main color" of the strip used in single color modes
LEDState back_color = {   0, 0, 0, 0 };  // Store the "2nd color" of the strip used in single color modes
LEDState xtra_color = {   0, 0, 0, 0 };  // Store the "3rd color" of the strip used in single color modes

bool updateConfig = false;  // For WiFiManger custom config and config
char last_state[67];            // Keeps the state representation before auto or off mode 
bool updateState = false;

// Button handling

#if defined(ENABLE_BUTTON)
//#define BTN_MODE_SHORT  "STA|mo|fxm|  s|  b| r1| g1| b1| w1| r2| g2| b2| w2| r3| g3| b3| w3"   // Example
  #define BTN_MODE_SHORT  "STA| 5|  0|196|255|  0|  0|  0|255|  0|  0|  0|  0|  0|  0|  0|  0"   // Static white
  #define BTN_MODE_MEDIUM "STA| 5| 48|196|200|255|102|  0|  0|  0|  0|  0|  0|  0|  0|  0|  0"   // Fire flicker
  #define BTN_MODE_LONG   "STA| 5| 46|196|200|255|102|  0|  0|  0|  0|  0|  0|  0|  0|  0|  0"   // Fireworks random
  unsigned long keyPrevMillis = 0;
  const unsigned long keySampleIntervalMs = 25;
  byte longKeyPressCountMax = 80;       // 80 * 25 = 2000 ms
  byte mediumKeyPressCountMin = 20;     // 20 * 25 = 500 ms
  byte KeyPressCount = 0;
  byte prevKeyState = HIGH;             // button is active low
#endif

#if defined(ENABLE_BUTTON_GY33)
  unsigned long keyPrevMillis_gy33 = 0;
  const unsigned long keySampleIntervalMs_gy33 = 25;
  byte longKeyPressCountMax_gy33 = 80;       // 80 * 25 = 2000 ms
  byte mediumKeyPressCountMin_gy33 = 20;     // 20 * 25 = 500 ms
  byte KeyPressCount_gy33 = 0;
  byte prevKeyState_gy33 = HIGH;             // button is active low
#endif
  
struct {
  uint16_t stripSize = NUMLEDS;
  char RGBOrder[5]  = RGBORDER;
  #if defined(USE_WS2812FX_DMA)
    #if USE_WS2812FX_DMA == 0
      uint8_t pin = 3;
    #endif
    #if USE_WS2812FX_DMA == 1
      uint8_t pin = 2;
    #endif
    #if USE_WS2812FX_DMA == 2
      uint8_t pin = 1;
    #endif
  #else
    uint8_t pin = LED_PIN;
  #endif
  uint8_t fxoptions = FX_OPTIONS;
} WS2812FXStripSettings;
