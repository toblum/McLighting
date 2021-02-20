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
 * 6 Jan 2019 v 2.2.0
 * - fix webserver not responding when E1.31 is mode is acivated: do a webserver.loop() for every 1.31 packet
 * - HA E1.31 mode added
 * 
 * 24 Jan 2019 v 2.2.1
 * - checkForRequests() is vital for e131 mode, remove from #ifdef ENABLE_LEGACY_ANIMATIONS
 * - Minor fixes related to NeoPixelBus UART methods
 * - Modify platformio.ini for future bump to esp8266-arduino v2.5.0 (shamelessly stolen settings from espurna project)
 * - Gzipped index2.htm & edit.htm.gz(untouched), convereted to hex format using xxd -i abcd.gz > html_gz.h
 * - Pointers added for WS2812FX & NeoPixelBus
 * - new "REST API": /pixelconf?ct=xxx to change length of LED strip
 * - new "REST API": /pixelconf?rgbo=xxx to change RGB order
 * - new "REST API": /pixelconf?pin=GPIO_NO to change PIN# (Allowed GPIO values: 16/5/4/0/2/14/12/13/15/3/1)
 * - added HA 0.87 version support https://github.com/toblum/McLighting/issues/327
 * - Added alternative way to send large messages using PubSubClient
 * - Bump PIO core to 2.0.4
 * - Send HA state on MQTT connect, address https://github.com/toblum/McLighting/issues/349
 * - Add LWT for MQTT and AMQTT, address https://github.com/toblum/McLighting/issues/340
 * - Added file for custom WS2812FX animations in custom slots
 * - Rename varaibles to be char instead of String
 * - Added LED pixel count and PIN settings to WiFiManager
 * - Gamma correction to LEDs
 * 
 * 7 Mar 2019 v 2.2.2
 * - Add compiler flag for WS2811 strips #define LED_TYPE_WS2811
 * - Hotfix #351
 * 
 * 18 Mar 2019 v 2.2.3 (mostly bugfix)
 * - PubSubClient related bug fixed
 * - UART 1 and 0 were mixed up
 * - LWT revisit
 * - Custom mode needs index
 * 
 * 16 Apr 2019
 * - fix all issues with 2.2.3 (REST API)
 * - #368 WDT fix
 * - #371 E1.31 fix
 * 
 * 05 Jul 2019 v2.2.5 (Hotfix)
 * - #415 wrong WS2812FXStripSettings.stripSize
 * - #424 PROGMEM edit is broken
 */
