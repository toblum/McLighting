/*

Example of adding the example: https://github.com/kitesurfer1404/WS2812FX/blob/master/examples/ws2812fx_custom_FastLED/ws2812fx_custom_FastLED.ino
as a custom effect

More info on how to create custom aniamtions for WS2812FX: https://github.com/kitesurfer1404/WS2812FX/blob/master/extras/WS2812FX%20Users%20Guide.md#custom-effects

*/

#include <FastLED.h>  //https://github.com/FastLED/FastLED

byte* heat;

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
  for( int i = 0; i < WS2812FXStripSettings.stripSize; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / WS2812FXStripSettings.stripSize) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= WS2812FXStripSettings.stripSize - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < WS2812FXStripSettings.stripSize; j++) {
    CRGB color = HeatColor( heat[j]);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (WS2812FXStripSettings.stripSize - 1) - j;
    } else {
      pixelnumber = j;
    }
    strip->setPixelColor(pixelnumber, color.red, color.green, color.blue);
  }
}

uint16_t myCustomEffect0() {
  Fire2012();
  return (strip->getSpeed() / WS2812FXStripSettings.stripSize);
}
