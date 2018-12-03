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
 */
