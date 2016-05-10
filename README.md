# McLighting - The ESP8266 based multi-client lighting gadget

> Mc Lighting (the multi-client lighting gadget) is a very cheap internet-controllable lighting solution based on the famous ESP8266 microcontroller and WS2811/2812 led strips. It features a self-hosted responsive web-interface, a REST-API and a websocket connector.

> Because of it's open architecture and APIs it's easy to build new clients for different platforms (iOS, Android, Windows Universal Apps, Siri/Cortana integration, ...). 

## The Hardware
- A NodeMCU development board, based on the ESP8266 ESP-12E, (that you can get for under 5 USD from eBay). A standalone ESP8266 or a Adafruit Huzzah should work too.
- A WS2811 or WS2812 led strip that you can get in many sizes and forms. I'm using a ring of 12 leds. When you use more than about 15-20 leds you may have to use a dedicated 5V power source.
- Power via USB

## Todos
- [x] Fix issue with websockets connection problems
- [ ] Switch to teh NeoPixelBus library (https://github.com/Makuna/NeoPixelBus/wiki)
- [ ] Use the led strip for status information in connection phase
- [ ] Enhance the documentation
- [ ] Stability improvements
- [ ] Additional clients

## Licence
GNU LGPLv3 (http://www.gnu.org/licenses/lgpl-3.0.txt)




*More information will be added as soon as I clean up the code.*
