Update 13.05.2018:
I've worked on a new web UI for the last weeks. It's now available as an early preview. There is a [video](https://youtu.be/lryDPMA2qpY) that shows the new features. [Try it out](https://github.com/toblum/McLightingUI) if you want and leave some feedback.

Update 07.04.2018:  
And even more changes to McLighting! Most of them were contributed by user @debsahu. Thank you!
- Update arduino-esp8266 to latest, at least version 2.4.1
- AMQTT is now the default MQTT library, it's a bit more lightweight and stable. You can still use PubSubClient if you want to.
- You can use @debsahu great NeoAnimationFX library as a alternative to WS2812FX. It's based on the NeoPixelBus instead of Adafruits NeoPixel library. It can handle longer strips more efficient. If you want, give it a try. WS2812FX is still the default.
- Some more changes regarding Homeassistant integration.  
Please see the [Wiki](https://github.com/toblum/McLighting/wiki/Software-installation) for details on the required libraries.  
If you have problems with the new version, let us know. You can get the last version [here](https://github.com/toblum/McLighting/tree/Before_AMQTT_NeoAnimationFX).  

I'm also working on a alternative web interface for McLighting in the meanwhile, but it may take some more time.  
For the german users: McLighting was used in [Kliemannsland](https://youtu.be/3TUjszkS3bY?t=1211) (a funny web show) when they built a really big Neopixel installation.

Update 18.03.2018:  
The code for integration with homeassistant was merged into master. It's currently active by default. You can safely disable it in definitions.h when use do not want to use it, or want to use McLighting on a small ESP_01.
There are some informations in the [Wiki](https://github.com/toblum/McLighting/wiki/Homeassistant-integration).

Update 17.02.2018:  
User @debsahu contributed code for integration with homeassistant. It's currently in a separate branch (https://github.com/toblum/McLighting/tree/feature/ha_integration). If you're using Homeassistant, please try it out and give feedback.
User @FabLab-Luenn created a version of McLighting (https://github.com/FabLab-Luenen/McLighting) for 6812 and other RGBW strips. Give it a try, if you own such strips.
A thank you goes to all contributors.

Update 12. / 15.02.2018:  
Added Home Assistant Support using MQTT Light. A better implementation would be using MQTT Light JSON.
Replaced Home Assistant Support using MQTT Light to MQTT JSON Light.

Update 31.01.2018:  
User @codmpm did a very professional McLighting installation and even designed his own PCBs. He has a great writeup for his project at: https://allgeek.de/2018/01/29/esp8266-neopixel-controller/ (in german).

Update 27.01.2018:  
Many people asked if it's possible to connect more than one strip (currently not) or at least "sync" multiple McLighting nodes. Although it may be possible to connect more then one WS2812 strip to the same data pin (works in many cases, you just have to try), syncing many McLighting instances would be a benefit. This could easily be achieved done by software like [NodeRed](https://nodered.org/). I added a example flow to demonstrate that [here](https://github.com/toblum/McLighting/blob/master/clients/node_red/websocket_proxy.json). Have a look at the short video [here](https://youtu.be/g3CHtG9c520).

Update 21.01.2018:  
User @szepnorbee contributed code for button control. Thank you! It's merged into the master branch now. There is a short manual for configuration [here](https://github.com/toblum/McLighting/wiki/Button-control).

Update 06.01.2018:  
After som etesting I merged the "feature/save_state" banch into master, so everybody should now be able to use this new functionality. Basically McLighting now saves the current mode to EEPROM and restores the setting on reboot. So you wont need to select your favorite mode again. If you don't want to use this, you can disable it in definitions.h.  
~~Some people noticed that there are currently problems compiling McLighting whe using ESP8266 core in version 2.4.0. This is due to a [problem](https://github.com/kitesurfer1404/WS2812FX/issues/58) with WS2812FX when using this version. For the moment you can stick to the 2.4.0 RC2 (also easily available via the boards manager).~~ (fixed now )  
Funny! McLighting was featured in the german radio show ["Netzbasteln"](https://www.deutschlandfunknova.de/beitrag/netzbasteln-wolkenlampe-mit-cloud-anschluss) on Deutschlandfunk Nova with a nice audio tutorial.

Update 16.12.2017:  
There was a breaking change in the WS2812FX library: Speeds have a new format (65535-0 instead of 0-255). I released a new version that converts the speeds settings. Please use the latest [WS2812FX library](https://github.com/kitesurfer1404/WS2812FX) (14.12.2017 or later) if use have an existing version installed.
I got many messages from people who use McLighting for own projects. User Brian Lough built a lighting system for his wedding and made a nice instruction video for his build: https://goo.gl/NbfKi8

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