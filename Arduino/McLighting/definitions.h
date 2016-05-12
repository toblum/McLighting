// Neopixel
#define PIN 5         // PIN where neopixel / WS2811 strip is attached
#define NUMLEDS 12    // Number of leds in the strip


#define HOSTNAME "ESP8266_01"   // Friedly hostname


// ***************************************************************************
// Global variables / definitions
// ***************************************************************************
#define DBG_OUTPUT_PORT Serial  // Set debug output port

// List of all color modes
enum MODE { HOLD, OFF, ALL, WIPE, RAINBOW, RAINBOWCYCLE, THEATERCHASE, THEATERCHASERAINBOW, TV };

MODE mode = RAINBOWCYCLE;   // Standard mode that is active when software starts

int delay_ms = 50;          // Global variable for storing the delay between color changes --> smaller == faster
int brightness = 128;       // Global variable for storing the brightness (255 == 100%)

bool exit_func = false;     // Global helper variable to get out of the color modes when mode changes

struct ledstate             // Data structure to store a state of a single led
{
   uint8_t red;
   uint8_t green;
   uint8_t blue;
};

typedef struct ledstate LEDState;   // Define the datatype LEDState
LEDState ledstates[NUMLEDS];        // Get an array of led states to store the state of the whole strip
LEDState main_color;                // Store the "main color" of the strip used in single color modes 
