# McLighting v2 - The ESP8266 based multi-client lighting gadget

[![Gitter](https://badges.gitter.im/mclighting/Lobby.svg)](https://gitter.im/mclighting/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)

> Mc Lighting (the multi-client lighting gadget) is a very cheap internet-controllable lighting solution based on the famous ESP8266 microcontroller and WS2811/2812 led strips. It features a self-hosted responsive web-interface, a REST-API and a websocket connector.

> Because of it's open architecture and APIs it's easy to build new clients for different platforms (iOS, Android, Windows Universal Apps, Siri/Cortana integration, ...). 

___
Update 30.09.2017:
Thanks to [@moose4lord](https://github.com/moose4lord) Mclighting works with the newest version of WS1812FX and has a possibility to define autocycle patterns [Wiki](https://github.com/toblum/McLighting/wiki/Autocycling). Thank for contributing to McLighting everyone!
I was also informed of a new project that is loosely based on McLighting: [Responsive_LED_Control](https://github.com/doctormord/Responsive_LED_Control) That looks very promising.

Update 07.08.2017:
As requested by many of you, McLighting now also features MQTT support. Thanks at @LeonVos for his attempts on this. I implemented the same API as used in WebSockets now for MQTT. Please have a look here for details: https://github.com/toblum/McLighting/wiki/MQTT-API I will try to add a new instruction video soon.

Many of you also took McLighting and adapted the software according your needs. This is great. I found some videos on YouTube that show these projects. I collected them here: https://goo.gl/yG7M4h
If you have done something similar with McLighting, please drop me a note. I'm always interested in what you've done with it.

Update 19.02.2017:
Added OTA support as promised by @markbajaj. Minor other improvements.

Update 05.02.2017:
After a long time I was able to work a bit on McLighting v2 and it's finally out now. The main difference, among minor improvements and library updates, is the usage of the great WS2812FX library for color animations. It brings a lot (almost 50!) of new animations.
The API changed a little bit, because the speed can now be set as a value from 0 to 255, not the delay anymore. So the web inferface had to change accordingly. The new animation mode have to be set also by their number, instead of a dedicated url. The list of all animation modes can also be received by the API. All existing API endpoints are kept for downward compatibility. So you should be able to use the new version without big changes. The original version is kept as branch "mclighting_legacy". Documentation will be updated soon.

Update 04.01.2017:
Now, there are two forks of McLighting (using the famous FastLED library). I did not notice it first, because I currently do not receive notification e-mails by Github (I have no idea why). Maybe you want to give them also a try, I will definitely do so as soon as I find time.  
https://github.com/russp81/LEDLAMP_FASTLEDs  
And this one was also forked: https://github.com/jake-b/Griswold-LED-Controller

Update 12.08.2016:
There is now a [gitter.im](https://gitter.im/mclighting/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link) chat room for this project.

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
- WiFiManager by @tzapu (tested with version 0.12.0)
  https://github.com/tzapu/WiFiManager
- WS2812FX by @kitesurfer1404 (tested with version downloaded 2017-02-05)
  https://github.com/kitesurfer1404/WS2812FX
- WebSockets by @Links2004 (tested with version 2.0.6)
  https://github.com/Links2004/arduinoWebSockets
- Adafruit NeoPixel by @adafruit (tested with 1.1.2)
  https://github.com/adafruit/Adafruit_NeoPixel
- Optional: PubSubClient by @knolleary (tested with 2.6.0)
  Only when you have activated MQTT in definitions.h.
  https://github.com/knolleary/pubsubclient/
  
The sketch also uses the following built-in library:
- Ticker by @igrr

Parts of the code were taken or inspired by the following sources:
- HSB3RGB conversion
  https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
- TV simulator logic inspired by @BulldogLowell
  https://github.com/BulldogLowell/PhoneyTV
- SPIFFS Webserver by Hristo Gochkov
  https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser

Thank you to all the authors for distributing their software that way.
I hope I didn't miss any sources and mentioned every author. In case I forgot someone please let me know and I will fix it.


## Todos
- [x] MQTT support
- [ ] Support multiple strips and control them separatley or together
- [ ] Save favourite effects? [Issue](https://github.com/toblum/McLighting/issues/35)
- [x] Fix issue with websockets connection problems
- [ ] Add support for 433MHz wireless socket using the [RC switch](https://github.com/sui77/rc-switch) library.
- [ ] Switch to the [NeoPixelBus library](https://github.com/Makuna/NeoPixelBus/wiki)
- [x] Use the led strip for status information in connection phase
- [x] Enhance the documentation
- [x] Stability improvements
- [x] RGBW mode [Issue](https://github.com/toblum/McLighting/issues/24)
- [x] Add called command to response [Issue](https://github.com/toblum/McLighting/issues/19)
- [ ] Customer profile to define segments of (in)active areas on the strip [Issue](https://github.com/toblum/McLighting/issues/37)
- [ ] Button control [Issue](https://github.com/toblum/McLighting/issues/36)
- [ ] Retain last state [Issue](https://github.com/toblum/McLighting/issues/47)
- [ ] Additional clients


## Licence
[GNU LGPLv3](http://www.gnu.org/licenses/lgpl-3.0.txt)


## Disclaimer
You use this project at your own risk. This is not a solution that should be used in productive environments, but this code and guide could give you a quick start for your own experiments. Please keep also in mind that there are currently some security features missing.


*More information will be added as soon as I clean up the code and complete documentation.*
