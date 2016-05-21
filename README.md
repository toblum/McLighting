# McLighting - The ESP8266 based multi-client lighting gadget

> Mc Lighting (the multi-client lighting gadget) is a very cheap internet-controllable lighting solution based on the famous ESP8266 microcontroller and WS2811/2812 led strips. It features a self-hosted responsive web-interface, a REST-API and a websocket connector.

> Because of it's open architecture and APIs it's easy to build new clients for different platforms (iOS, Android, Windows Universal Apps, Siri/Cortana integration, ...). 


[![Demo video WebClient](https://j.gifs.com/kRPrzN.gif)](https://youtu.be/rc6QVHKAXBs)

## The Hardware
- A NodeMCU development board, based on the ESP8266 ESP-12E, (that you can get for under $5 from eBay). A standalone ESP8266 or a Adafruit Huzzah should work too.
- A WS2811 or WS2812 led strip that you can get in many sizes and forms. I'm using a ring of 12 leds. When you use more than about 15-20 leds you may have to use a dedicated 5V power source.
- Power via USB

## Wiring

Fritzing: 
![Wiring schema](https://raw.githubusercontent.com/toblum/McLighting/master/documentation/pics/McLighting-NodeMCU_Board.png)

Parts via:
- https://github.com/squix78/esp8266-fritzing-parts
- https://github.com/adafruit/Fritzing-Library/

## Software installation
You need to complete the following steps to build your development environment that enables you to flash the Mc Lighting software to the ESP8622.

### Arduino Software (tested with 1.6.8)
Download and install the arduino software (IDE) at https://www.arduino.cc/en/Main/Software

### ESP8266 board support for arduino IDE
In the Arduino IDE open the preferences dialog and enter the following URL as "Additional Boards Manger URL":\
http://arduino.esp8266.com/stable/package_esp8266com_index.json
![Preferences](https://raw.githubusercontent.com/toblum/McLighting/master/documentation/pics/arduino_preferences.png)

Go to "Tools" > "Board: <some board>" > "Boards Manager ...", search for "esp" and install the "esp8266 by ESP8266 Community" in version 2.2.0 (https://github.com/esp8266/Arduino):
![Preferences](https://raw.githubusercontent.com/toblum/McLighting/master/documentation/pics/arduino_boards_manager.png)

Now go to "Tools" > "Board: <some board>" and choose "NodeMCU 1.0 (ESP-12E Module)", set CPU frequency to 80 MHz, and Flash size to "4M (1M SPIFFS)"leave upload spped at 115200. Select the right COM port.

### Used Libraries
Go to "Sketch" > "Include Library" > "Manage Libraries ..." and install the following libraries by searching for them and installing:
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

### Compiling and upload
Now open the MC Lighting Arduino sketch in the IDE via "File" > "Open ...". Have a look at the "definitions.h" and change the values here to fit your setup:
```c
// Neopixel
#define PIN 5         // PIN where neopixel / WS2811 strip is attached
#define NUMLEDS 12    // Number of leds in the strip

#define HOSTNAME "ESP8266_01"   // Friedly hostname
```

Now you have done everything to get all the dependencies. You should now be able to build the software by choosing "Sketch" > "Verify / Compile" (or clicking the tick mark in the tool bar).

Please verify that you have connected the ESP board correctly to your computer via USB and that the correct COM port is chosen.

Now you should be able to upload the compiled sketch to the board via "Sketch" > "Upload" (or by clicking the right arrow in the tool bar).

## Todos
- [x] Fix issue with websockets connection problems
- [ ] Switch to the [NeoPixelBus library](https://github.com/Makuna/NeoPixelBus/wiki)
- [ ] Use the led strip for status information in connection phase
- [ ] Enhance the documentation
- [ ] Stability improvements
- [ ] Additional clients

## Licence
[GNU LGPLv3](http://www.gnu.org/licenses/lgpl-3.0.txt)




*More information will be added as soon as I clean up the code and complete documentation.*
