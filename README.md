# McLighting v2 - The ESP8266 based multi-client lighting gadget

[![Gitter](https://badges.gitter.im/mclighting/Lobby.svg)](https://gitter.im/mclighting/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

Mc Lighting (the multi-client lighting gadget) is a very cheap internet-controllable lighting solution based on the famous ESP8266 microcontroller and WS2811/2812 led strips. It features a self-hosted responsive web-interface, a REST-API and a websocket connector.

Because of it's open architecture and APIs it's easy to build new clients for different platforms (iOS, Android, Windows Universal Apps, Siri/Cortana integration, ...). 

[![Demo video WebClient](https://j.gifs.com/kRPrzN.gif)](https://youtu.be/rc6QVHKAXBs)

[![Demo video Apple Homekit integration](https://j.gifs.com/gJP2o6.gif)](https://youtu.be/4JnGXZaPnrw)

___


## The Hardware

The project ist based on the famous ESP8266 microcontroller and WD2811/WS2812 LED strips. There are many variations of the ESP chip out there, but I chose the NodeMCU dev board, because it's powered by micro USB and has a voltage converter included to power the ESP which uses 3.3V.
A standalone ESP8266 or a Adafruit Huzzah should work too.

The RGB LED strips are also available in many different flavours as strip or as standalone LEDs and can easily be chained.

See wiki [Hardware](../../wiki/Hardware)


## Software installation
See wiki [Software installation](../../wiki/Software-installation)


### Used Libraries

This project uses libraries and code by different authors:

- [WiFiManager](https://github.com/tzapu/WiFiManager) by tzapu (tested with version 0.12.0)

- [WS2812FX](https://github.com/kitesurfer1404/WS2812FX) by kitesurfer1404 (tested with version downloaded 2017-02-05)

- [WebSockets](https://github.com/Links2004/arduinoWebSockets) by Links2004 (tested with version 2.0.6)

- [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) by adafruit (tested with 1.1.2)

- Optional: [PubSubClient](https://github.com/knolleary/pubsubclient/) by knolleary (tested with 2.6.0)
  _Only when you have activated MQTT in definitions.h._
  
The sketch also uses the following built-in library:
- Ticker by [@igrr](https://github.com/igrr)

Parts of the code were taken or inspired by the following sources:

- [HSB3RGB conversion](https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/)

- [TV simulator](https://github.com/BulldogLowell/PhoneyTV) logic inspired by BulldogLowell
  
- [SPIFFS Webserver](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser) by Hristo Gochkov

Thank you to all the authors for distributing their software that way.
I hope I didn't miss any sources and mentioned every author. In case I forgot someone please let me know and I will fix it.


## Todos
- [x] MQTT support
- [ ] Support multiple strips and control them separately or together #118
- [x] Save favourite effects? [Issue](https://github.com/toblum/McLighting/issues/35)
- [ ] Make number of pixels, MQTT and PIN configurable via front end [Issue](https://github.com/toblum/McLighting/issues/93) and [Issue](https://github.com/toblum/McLighting/issues/101)
- [x] OTA update [Issue](https://github.com/toblum/McLighting/issues/93)
- [ ] Bundle webpages instead of SPIFFS [Issue](https://github.com/toblum/McLighting/issues/93)
- [ ] Remove old / wrong EEPROM settings completely (https://github.com/toblum/McLighting/issues/92)
- [x] Fix issue with websockets connection problems
- [x] Switch to the [NeoPixelBus library](https://github.com/Makuna/NeoPixelBus/wiki)
- [x] Use the led strip for status information in connection phase
- [x] Enhance the documentation
- [x] Stability improvements
- [x] RGBW mode [Issue](https://github.com/toblum/McLighting/issues/24)
- [x] Add called command to response [Issue](https://github.com/toblum/McLighting/issues/19)
- [ ] Customer profile to define segments of (in)active areas on the strip [Issue](https://github.com/toblum/McLighting/issues/37)
- [x] Button control [Issue](https://github.com/toblum/McLighting/issues/36)
- [x] Retain last state [Issue](https://github.com/toblum/McLighting/issues/47)
- [ ] Additional clients
- [ ] If no wifi, at least enable button mode.
- [ ] Also enable McLighting in Wifi AP mode.
- [x] Make a set of NodeRed nodes.
- [ ] IR remote support [issue](https://github.com/toblum/McLightingUI/issues/3)
- [ ] Multiple buttons/GPIO Inputs. [Issue](https://github.com/toblum/McLighting/issues/119)
- [ ] Music visualizer / Bring back ArtNet [Issue](https://github.com/toblum/McLighting/issues/111)
- [ ] Display version and parameters (Number of LEDs, definition settings, ..) in the web UI [Issue](https://github.com/toblum/McLighting/issues/150)


## Licence
[MIT](https://choosealicense.com/licenses/mit/)


## Disclaimer
You use this project at your own risk. This is not a solution that should be used in productive environments, but this code and guide could give you a quick start for your own experiments. Please keep also in mind that there are currently some security features missing.


*More information will be added as soon as I clean up the code and complete documentation.*
