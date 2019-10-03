// Prototypes
uint16_t convertSpeed(uint8_t mcl_speed);
uint32_t trans(uint32_t newcolor, uint32_t oldcolor, uint8_t level);
// End Prototypes
/*

Example of adding the example: https://github.com/kitesurfer1404/WS2812FX/blob/master/examples/ws2812fx_custom_FastLED/ws2812fx_custom_FastLED.ino
as a custom effect

More info on how to create custom aniamtions for WS2812FX: https://github.com/kitesurfer1404/WS2812FX/blob/master/extras/WS2812FX%20Users%20Guide.md#custom-effects

*/

// ***************************************************************************
// Functions and variables for automatic cycling
// ***************************************************************************

void handleAutoPlay() {
  if (autoDelay <= millis()) {
    hex_colors[0] = autoParams[autoCount][0];
    hex_colors[1] = autoParams[autoCount][1];
    hex_colors[2] = autoParams[autoCount][2];    
    strip->setColors(segment, hex_colors);
    strip->setSpeed(segment, convertSpeed((uint8_t)autoParams[autoCount][3]));
    strip->setMode(segment, (uint8_t)autoParams[autoCount][4]);
    strip->trigger();
    autoDelay = millis() + (uint32_t)autoParams[autoCount][5];
    DBG_OUTPUT_PORT.print("autoTick ");
    DBG_OUTPUT_PORT.printf("autoTick[%d]: {0x%08x, 0x%08x, 0x%08x, %d, %d, %d}\r\n", autoCount, hex_colors[0], hex_colors[1], hex_colors[2], autoParams[autoCount][3], autoParams[autoCount][4], autoParams[autoCount][5]);

    autoCount++;
    if (autoCount >= (sizeof(autoParams) / sizeof(autoParams[0]))) autoCount = 0;
  }
}

void handleAuto() {
  // Dummy function
}

void handleCustomWS() {
  // Dummy function
}

#if defined(CUSTOM_WS2812FX_ANIMATIONS)
// ***************************************************************************
// TV mode
// ***************************************************************************

uint8_t  dipInterval = 10;
uint16_t darkTime = 250;
unsigned long currentDipTime;
unsigned long dipStartTime;
unsigned long currentMillis;
uint8_t  ledState = LOW;
long     previousMillis = 0; 
uint16_t led = 5;
uint16_t interv = 2000;
uint8_t  twitch = 50;
uint8_t  dipCount = 0;
boolean  timeToDip = false;


void hsb2rgbAN1(uint16_t index, uint8_t sat, uint8_t bright, uint16_t led) {
  // Source: https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
  uint8_t temp[5], n = (index >> 8) % 3;
  temp[0] = temp[3] = (uint8_t)((                                         (sat ^ 255)  * bright) / 255);
  temp[1] = temp[4] = (uint8_t)((((( (index & 255)        * sat) / 255) + (sat ^ 255)) * bright) / 255);
  temp[2] =           (uint8_t)(((((((index & 255) ^ 255) * sat) / 255) + (sat ^ 255)) * bright) / 255);
  strip->setPixelColor(led, temp[n + 2], temp[n + 1], temp[n], 0);
}


void updateLed (uint16_t led, uint8_t brightness) {
  ledstates[led] = brightness;
  for (uint16_t i=seg_start; i<=seg_stop; i++) {
    uint16_t index = (i%3 == 0) ? 400 : random(0,767);
    hsb2rgbAN1(index, 200, ledstates[i], i);
  }
  //strip->show();
}


// See: http://forum.mysensors.org/topic/85/phoneytv-for-vera-is-here/13
void handleTV() {
  if (timeToDip == false) {
    currentMillis = millis();
    if(currentMillis-previousMillis > interv) {
      previousMillis = currentMillis;
      //interv = random(750,4001);//Adjusts the interval for more/less frequent random light changes
      interv = random(1000-(ws2812fx_speed*2),5001-(ws2812fx_speed*8));
      twitch = random(40,100);// Twitch provides motion effect but can be a bit much if too high
      dipCount = dipCount++;
    }
    if(currentMillis-previousMillis<twitch) {
      led=random(0, WS2812FXStripSettings.stripSize - 1);
      ledState = ledState == LOW ? HIGH: LOW; // if the LED is off turn it on and vice-versa: 
      updateLed(led, (ledState) ? 255 : 0);   
      if (dipCount > dipInterval) { 
        DBG_OUTPUT_PORT.println("dip");
        timeToDip = true;
        dipCount = 0;
        dipStartTime = millis();
        darkTime = random(50,150);
        dipInterval = random(5,250);// cycles of flicker
      }
    } 
  } else {
    DBG_OUTPUT_PORT.println("Dip Time");
    currentDipTime = millis();
    if (currentDipTime - dipStartTime < darkTime) {
      for (uint16_t i=3; i<WS2812FXStripSettings.stripSize; i++) {
        updateLed(i, 0);
      }
    } else {
      timeToDip = false;
    }
  }
}

void handleE131(){
  if (!e131->isEmpty())
  {
    e131_packet_t packet;
    e131->pull(&packet); // Pull packet from ring buffer

    uint16_t universe = htons(packet.universe);
    uint8_t *data = packet.property_values + 1;

    if (universe < START_UNIVERSE || universe > END_UNIVERSE) return; //async will take care about filling the buffer

    // Serial.printf("Universe %u / %u Channels | Packet#: %u / Errors: %u / CH1: %u\n",
    //               htons(packet.universe),                 // The Universe for this packet
    //               htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
    //               e131.stats.num_packets,                 // Packet counter
    //               e131.stats.packet_errors,               // Packet error counter
    //               packet.property_values[1]);             // Dimmer data for Channel 1
/*  #if defined(RGBW)
    uint16_t multipacketOffset = (universe - START_UNIVERSE) * 128; //if more than 128 LEDs * 4 colors = 512 channels, client will send in next higher universe
    if (NUMLEDS <= multipacketOffset) return;
    uint16_t len = (128 + multipacketOffset > WS2812FXStripSettings.stripSize) ? (WS2812FXStripSettings.stripSize - multipacketOffset) : 128;
  #else*/
    uint16_t multipacketOffset = (universe - START_UNIVERSE) * 170; //if more than 170 LEDs * 3 colors = 510 channels, client will send in next higher universe
    if (WS2812FXStripSettings.stripSize <= multipacketOffset) return;
    uint16_t len = (170 + multipacketOffset > WS2812FXStripSettings.stripSize) ? (WS2812FXStripSettings.stripSize - multipacketOffset) : 170;
/*  #endif */
    for (uint16_t i = 0; i < len; i++){
      uint16_t j = i * 3;
/*  #if defined(RGBW)
      strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], data[j + 3]);
  #else */
      strip->setPixelColor(i + multipacketOffset, data[j], data[j + 1], data[j + 2], 0);
/*  #endif */
    }
  }
}

#include <FastLED.h>  //https://github.com/FastLED/FastLED
/*
 * paste in the Fire2012 code with a small edit at the end which uses the
 * setPixelColor() function to copy the color data to the ws2812fx instance. 
*/

 // Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  70

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

bool gReverseDirection = false;

void Fire2012() {
// Array of temperature readings at each simulation cell
  
  // Step 1.  Cool down every cell a little
  for( uint16_t i = seg_start; i <= seg_stop; i++) {
    ledstates[i] = qsub8( ledstates[i],  random8(0, ((COOLING * 10) / WS2812FXStripSettings.stripSize) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( uint16_t k= seg_stop-seg_start - 1; k >= 2; k--) {
    ledstates[k] = (ledstates[k - 1] + ledstates[k - 2] + ledstates[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < SPARKING ) {
    uint8_t y = random8(7);
    ledstates[y] = qadd8(ledstates[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( uint16_t j = seg_start; j <= seg_stop; j++) {
    CRGB color = HeatColor( ledstates[j]);
    uint16_t pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = seg_stop - j;
    } else {
      pixelnumber = j;
    }
    strip->setPixelColor(pixelnumber, color.red, color.green, color.blue, 0);
  }
}

void Gradient() {
  for( uint16_t j = seg_start; j <= seg_stop; j++) {
    uint16_t pixelnumber;
    uint32_t color;
    if( gReverseDirection ) {
      pixelnumber = seg_stop - j;
    } else {
      pixelnumber = j;
    }
    color = trans(strip->getColors(segment)[1], strip->getColors(segment)[0], (j*255)/(seg_stop - seg_start));
    strip->setPixelColor(pixelnumber, ((color >> 16) & 0xFF), ((color >> 8) & 0xFF), ((color >> 0) & 0xFF), ((color >> 24) & 0xFF));
  }
}
#endif

uint16_t myCustomEffect0() {
  handleAuto();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}

uint16_t myCustomEffect1() {
  handleCustomWS();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}
#if defined(CUSTOM_WS2812FX_ANIMATIONS)
uint16_t myCustomEffect2() {
  handleTV();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}

uint16_t myCustomEffect3() {
  handleE131();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}

uint16_t myCustomEffect4() {
  Fire2012();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}

uint16_t myCustomEffect5() {
  Gradient();
  return (strip->getSpeed(segment) / WS2812FXStripSettings.stripSize);
}
#endif
