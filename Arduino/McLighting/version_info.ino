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
 * 15 Feb 2019 v 2.2.0 rgbw 3colors
 * - Code cleanup
 * - Implemented support for back- and xtra-color
 * - Implemented IR remote control
 * - Remove some string data types (to be continued)
 *
 * 08 Mar 2018 v 2.2.1 rgbw 3colors
 * - checkForRequests() is not needed
 * - Minor fixes related to NeoPixelBus UART methods
 * - Modify platformio.ini for future bump to esp8266-arduino v2.5.0 (shamelessly stolen settings from espurna project)
 * - Gzipped index.htm & edit.htm, convereted to hex format using xxd -i abcd.gz > html_gz.h
 * - Pointers added for WS2812FX & NeoPixelBus
 * - new "REST API": /config?ws_cnt=xxx to change length of LED strip
 * - new "REST API": /config?ws_rgbo=xxx to change RGB order
 * - new "REST API": /config?ws_pin=GPIO_NO to change PIN# (Allowed GPIO values: 16/5/4/0/2/14/12/13/15/3/1) if not used DMA or UART. Otherwise it is ignored
 * - added HA 0.87 version support https://github.com/toblum/McLighting/issues/327
 * - Added alternative way to send large messages using PubSubClient
 * - Bump PIO core to 2.0.4
 * - Send HA state on MQTT connect, address https://github.com/toblum/McLighting/issues/349
 * - Add LWT for MQTT and AMQTT, address https://github.com/toblum/McLighting/issues/340
 * - Added file for custom WS2812FX animations in custom slots
 * - Rename variables to be char instead of String
 * - Added LED pixel count and PIN settings to WiFiManager
 * - Gamma correction to LEDs via ws_fxopts
 * 
 * 10 Mar 2019 v 2.2.2 rgbw 3colors
 * - integraded neoconfig.json into config.json
 * - Add compiler flag for WS2811 strips #define LED_TYPE_WS2811
 * - new "REST API": /config?hostname=xxx to change hostname
 * - new "REST API": /config?mqtt_host=xxx to change mqtt hostname
 * - new "REST API": /config?mqtt_port=xxx to change mqtt port
 * - new "REST API": /config?mqtt_user=xxx to change mqtt username
 * - new "REST API": /config?mqtt_pass=xxx to change mqtt password
 * - new "REST API": /config?ws_fxopt=xxx to change ws2812fx options
 * - Pointers added for PubSubClient & AMQTTCLient
 * - RGBOrder is now stored human readable not as integer
 * - Bugfix on Fire 2012 animation as one variable was destroyed
 * 
 * 15 Mar 2019 v 2.2.2 rgbw 3colors
 * websocket commands
 * #   Set Maincolor
 * ##  Set Back color
 * ### Set xtra Color
 * ?   Set speed
 * %   Set brightness
 * *   Set all
 * !   Set single LED
 * +   Set multiple LEDs
 * R   Set Range
 * =   Set named Mode (legacy)
 * $   Get Status
 * new from here
 * C   Get Config  
 * Ch  Set hostname
 * Cmh Set mqtt hostname
 * Cmp Set mqtt port
 * Cmu Set mqtt username
 * Cmw Set mqtt password
 * Csc Set Strip LED count
 * Csr Set Strip RGB Order
 * Csp Set Strip pin
 * Cso Set Strip FX Options
 * to here
 * ~   Get Modes
 * /   Set modes
 * 
 * 17 Mar 2019 
 * adressed issue: #2
 * adressed issue: #3
 * 
 * 18 Mar 2019 
 * adressed issue: #6 (possibly affects R[r_start][r_end][hexrgb] [...]; +[numled][hexrgb]+[numled][hexrgb]+[numled][hexrgb] !<numled><hexrgb>)
 *
 * 19 Mar 2019
 * included custom mode in UI
 * adressed issue #4
 *
 * Version Bump to 2.2.3 rgbw 3colors
 * PubSubClient Bug fixes
 * Reverted Pointers for MQTT for the moment
 * 
 * 21 Mar 2019
 * Bugfixes
 * added pointer for MQTT again
 * Removed some String datatype and replaced with char array
 * better responsiveness in ui for sliders
 *
 * 26 Mar 2019
 * Bugfixes
 * 
 * 19 May 2019
 * Bugfixes regarding MQTT Hostname
 *
 * 08 September 2019
 * Version Bump to 2.2.5 rgbw 3colors
 * adressed issue: #27 
 * adressed issue: #28 (see new REST-API documentation)
 *
 * 10 September 2019
 * Version Bump to 2.2.6 rgbw 3colors 
 * adressed issue: #28 (see new REST-API documentation) again
 * adressed issue: #26
 * adressed issue: #31
 * adressed issue: #32 
 * 
 * 09 Oktober 2019
 * Version Bump to 2.2.7 rgbw 3colors 
 * added output to control external power supply
 * 
 * 06 December 2019
 * Version Bump to 2.2.8 rgbw 3colors 
 * Bugfixes
 * adressed issue #59
 */
