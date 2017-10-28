// Neopixel
#define PIN 5         // PIN where neopixel / WS2811 strip is attached
#define NUMLEDS 24    // Number of leds in the strip

const char HOSTNAME[] = "ESP8266_01";   // Friedly hostname

#define ENABLE_OTA    // If defined, enable Arduino OTA code.

#define ENABLE_MQTT   // If defined, enable MQTT client code.

// parameters for automatically cycling favorite patterns
uint32_t autoParams[][4] = {   // color, speed, mode, duration (seconds)
    {0xff0000, 200,  1,  5.0}, // blink red for 5 seconds
    {0x00ff00, 200,  3, 10.0}, // wipe green for 10 seconds
    {0x0000ff, 200, 11,  5.0}, // dual scan blue for 5 seconds
    {0x0000ff, 200, 42, 15.0}  // fireworks for 15 seconds
};

#ifdef ENABLE_MQTT
  #define MQTT_MAX_PACKET_SIZE 256
  #define MQTT_MAX_RECONNECT_TRIES 4

  int mqtt_reconnect_retries = 0;
  
  char mqtt_intopic[strlen(HOSTNAME) + 3];      // Topic in will be: <HOSTNAME>/in
  char mqtt_outtopic[strlen(HOSTNAME) + 4];     // Topic out will be: <HOSTNAME>/out
  
  const char mqtt_clientid[] = "ESP8266Client"; // MQTT ClientID 

  char mqtt_host[64] = "";
  char mqtt_port[6] = "";
  char mqtt_user[32] = "";
  char mqtt_pass[32] = "";
#endif


// ***************************************************************************
// Global variables / definitions
// ***************************************************************************
#define DBG_OUTPUT_PORT Serial  // Set debug output port

// List of all color modes
enum MODE { SET_MODE, HOLD, OFF, ALL, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, THEATERCHASERAINBOW, TV, CUSTOM };

MODE mode = RAINBOW;        // Standard mode that is active when software starts

int ws2812fx_speed = 10;    // Global variable for storing the delay between color changes --> smaller == faster
int brightness = 192;       // Global variable for storing the brightness (255 == 100%)

int ws2812fx_mode = 0;      // Helper variable to set WS2812FX modes

bool exit_func = false;     // Global helper variable to get out of the color modes when mode changes

bool shouldSaveConfig = false;  // For WiFiManger custom config

struct ledstate             // Data structure to store a state of a single led
{
   uint8_t red;
   uint8_t green;
   uint8_t blue;
};

typedef struct ledstate LEDState;    // Define the datatype LEDState
LEDState ledstates[NUMLEDS];         // Get an array of led states to store the state of the whole strip
LEDState main_color = { 255, 0, 0 }; // Store the "main color" of the strip used in single color modes 
