/*
 * 07 May 2018 v 2.1.0
 * - Start of versioning v2.1, added version support
 * - '/esp_status' returns lot more info
 * 
 * 11 May 2018 v 2.1.1
 * - Use ArduinoJSON to send JSON replies
 * - Add strip.trigger()
 * 
 * 17 Jun 2018 v 2.1.2
 * - Upgrade to ArduinoJSON 6.xx from ArduinoJSON 5.xx
 * - Added example for static IP
 * - Added more internal variables to /esp_status
 * 
 * 5 Jul 2018 v 2.1.3
 * - Fixes for ArduinoJson 6.1.0-beta
 * 
 * 10 Jul 2018 v 2.1.4
 * - Fixes measureJson() as pointed in #206
 * 
 * 2 Oct 2018 v 2.1.5
 * - Try fixing #224 HA brightness causes reboot 
 * 
 * 5 Nov 2018 v 2.1.6
 * - Retire NeoAnimationFX
 * - Use DMA or UART method along with WS2812FX instead
 * - fix #248 
 * 
 * 3 Dec 2018 v 2.1.7
 * - Contributions by @	MrTheBarbarian from #270
 * - rethink ESP.getChipId implementaion
 * - check ArduinoJSON version
 * - Try restting prevmode as suggested in #276
 * 
 * 11 Dec 2018 v 2.1.8
 * - Fix Auto-Discovery for HA version >= 0.84 #286
 * - Fix #283
 * 
 * 12 Dec 2018 v 2.2.0
 * - Add E1.31 mode initial commit
 * - E1.31 mode when activated now stops current animation
 * 
 * 13 Dec 2018 v 2.1.9
 * - HA is not getting the correct animation name being run, boils down to changes to ArduinoJson library
 * - Bump ArduinoJson library requirment for v6.7.0-beta (better memory management)
 * - sendState() needs extra memory for jsonBuffer
 * - sensState() effect can be sent directly instead of copying from PROGMEM
 * 
 * 16 Dec 2018 v 2.1.10
 * - more ArduinoJson library memory managment fixes
 * 
 * 18 Dec 2018 v 2.1.11
 * - More Auto-Discovery fix for HA version >= 0.84 #286
 * - Suggestions from https://github.com/home-assistant/home-assistant/issues/19420
 * 
 * 23 Dec 2018 v 2.2.0 
 * - Add E1.31 mode to getModes(), no need to change McLightingUI
 * 
 * 6 Jan 2018 v 2.2.0
 * - fix webserver not responding when E1.31 is mode is acivated: do a webserver.loop() for every 1.31 packet
 * - HA E1.31 mode added
  * 
 * 15 Feb 2018 v 2.2.0 rgbw 3colors
 * - Code cleanup
 * - Implemented support for back- and xtra-color
 * - Implemented IR remote control
 * - Remove some string data types (to be continued)
 */
