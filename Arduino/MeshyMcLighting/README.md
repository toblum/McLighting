# What is this?

Wouldn't be cool for many McLighting to talk to each other and synchronize? Here is my naive attempt at this, which requires McLighting to be served as stand-alone webclient.

- Uses painlessMesh to create mesh network and broadcasts state to every node
- Connects to known wifi network predefined in code (no WiFiManager yet)
- Does not need WiFi connection to internet, standalone mode + mesh
- Web interface is borrowed from "WS2812FX esp8266" example, completely served on ESP8266
- Can do minimal file upload to SPIFFs
- Completely Async!
- Uses Task Scheduler, no more tickers
- No delays in entire code
- Has RESTful API (same API as McLighting, not all status are returned now)
- Async Websockets (ws://**HOSTNAME**/ws on port 80, same API as McLighting)
- Async MQTT/Home Assistant Intergration (Only **SERVER** connects to outside world)
- Auto Mode (same as McLighting)
- Button Mode (same as McLighting)

### Limitations/TODO

- Use Arduino ESP8266 **GIT** version (Issues with v2.4.1: not memory optimized)
- For **SERVER**, compile with IwIP v1.4 Higher Bandwidth for now, painlessMesh doesnt work properly on IwIP v2 (Low Memory/Bandwidth)
- WS2812FX has **delays** meant for ESP32. Track [issue here](https://github.com/kitesurfer1404/WS2812FX/issues/89) NeoAnimationFX has no delays.

### Issues where help is needed

- All issues mentioned in limitations
- No WiFiManager yet for **SERVER**, have to think of ways of implementing this - **High Priority**

#### Libraries to install

In Arduino, Goto Sketch -> Include Library -> Add .ZIP Library... and point to the zip file downloaded.

* [Painless Mesh](https://gitlab.com/painlessMesh/painlessMesh/-/archive/master/painlessMesh-master.zip)
* [Arduino JSON](https://github.com/bblanchon/ArduinoJson/archive/master.zip)
* [Task Scheduler](https://github.com/arkhipenko/TaskScheduler/archive/master.zip)
* [Async TCP](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip)
* [Async Web Server](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip)
* [Async MQTT Client](https://github.com/marvinroger/async-mqtt-client/archive/master.zip)

#### LED libraries
In definitions.h: `#define USE_WS2812FX`
* [WS2812FX](https://github.com/kitesurfer1404/WS2812FX/archive/master.zip)
* [Adafruit NeoPixels](https://github.com/adafruit/Adafruit_NeoPixel/archive/master.zip)

or

In definitions.h: `#define USE_NEOANIMATIONFX`
* [NeoAnimationFX](https://github.com/debsahu/NeoAnimationFX/archive/master.zip)
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus/archive/master.zip)
