# McLighting - The ESP8266 based multi-client lighting gadget

> Mc Lighting (the multi-client lighting gadget) is a very cheap internet-controllable lighting solution based on the famous ESP8266 microcontroller and WS2811/2812 led strips. It features a self-hosted responsive web-interface, a REST-API and a websocket connector.

> Because of it's open architecture and APIs it's easy to build new clients for different platforms (iOS, Android, Windows Universal Apps, Siri/Cortana integration, ...). 


___
Update 11.06.2016:  
Today I presented the project at [Pi and More 9](https://piandmore.de/) and got some good feedback, even though my presentation was not perfect and time was too short to present everything I prepared. So I uploaded the [slides (german)](documentation/slides/Ein%20SmartLight%20im%20Selbstbau%20für%20unter%2015%20€_Pi%20and%20More%209.pdf) to this repository for your reference.
___


[![Demo video WebClient](https://j.gifs.com/kRPrzN.gif)](https://youtu.be/rc6QVHKAXBs)

[![Demo video Apple Homekit integration](https://j.gifs.com/gJP2o6.gif)](https://youtu.be/4JnGXZaPnrw)


## The Hardware

The project ist based on the famous ESP8266 microcontroller and WD2811/WS2812 LED strips. There are many variations of the ESP chip out there, but I chose the NodeMCU dev board, because it's powered by micro USB and has a voltage converter included to power the ESP which uses 3.3V.
A standalone ESP8266 or a Adafruit Huzzah should work too.

The RGB LED strips are also available in many different flavours as strip or as standalone LEDs and can easily be chained.

See wiki [Hardware](../../wiki/Hardware)


## Software installation
See wiki [Software installation](../../wiki/Software-installation)


### Used Libraries
This project uses libraries and code by different authors:
- WiFiManager by @tzapu (tested with version 0.11.0)
  https://github.com/tzapu/WiFiManager
- WebSockets by @Links2004 (tested with version 2.0.2)
  https://github.com/Links2004/arduinoWebSockets
- Adafruit NeoPixel by @adafruit (tested with 1.0.5)
  https://github.com/adafruit/Adafruit_NeoPixel
  
The sketch also uses the following built-in library:
- Ticker by @igrr

Parts of the code were taken or inspired by the following sources:
- HSB3RGB conversion
  https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
- TV simulator logic inspired by @BulldogLowell
  https://github.com/BulldogLowell/PhoneyTV
- SPIFS Webserver by Hristo Gochkov
  https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser

Thank you to all the authors for distributing their software that way.
I hope I didn't miss any sources and mentioned every author. In case I forgot someone please let me know and I will fix it.


## Todos
- [x] Fix issue with websockets connection problems
- [ ] Add support for 433MHz wireless socket using the [RC switch](https://github.com/sui77/rc-switch) library.
- [ ] Switch to the [NeoPixelBus library](https://github.com/Makuna/NeoPixelBus/wiki)
- [ ] Use the led strip for status information in connection phase
- [ ] Enhance the documentation
- [ ] Stability improvements
- [ ] Additional clients


## Licence
[GNU LGPLv3](http://www.gnu.org/licenses/lgpl-3.0.txt)


## Disclaimer
You use this project at your own risk. This is not a solution that should be used in productive environments, but this code and guide could give you a quick start for your own experiments. Please keep also in mind that there are currently some security features missing.


*More information will be added as soon as I clean up the code and complete documentation.*
