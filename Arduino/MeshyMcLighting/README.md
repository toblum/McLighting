# What is this?

Wouldn't be cool for many McLighting to talk to each other and synchronize? Here is my naive attempt at this, which requires McLighting to be served as stand-alone webclient.

## Features

- Uses painlessMesh to create mesh network and broadcasts state to every node
- Does not need WiFi connection to internet, standalone mode + mesh
- Web interface is borrowed from "WS2812FX esp8266" example, completely served on ESP8266
- Can do minimal file upload to SPIFFs
- Completely Async!
- Uses Task Scheduler, no more tickers
- No delays in entire code
- Has RESTful API (same API as McLighting, use set_mode for setting mode, speed, brighness)
- Async Websockets (ws://**HOSTNAME**/ws on port 80, same API as McLighting)
- Async MQTT/Home Assistant Intergration (Only **SERVER** connects to outside world)
- Auto Mode (same as McLighting)
- Button Mode (same as McLighting)
- Async WiFiManager for **SERVER**

### Limitations/TODO

- Use Arduino ESP8266 **GIT** version (Issues with v2.4.1: not memory optimized)
- WS2812FX has **delays** meant for ESP32. Track [issue here](https://github.com/kitesurfer1404/WS2812FX/issues/89) NeoAnimationFX has no delays.

### Issues where help is needed

- All issues mentioned in limitations

## How to use this?

**SERVER** (Connects to outside world:  WiFi/MQTT/HA)

* Compile **SERVER** first and upload.
* Using a phone, connect to WiFi named "MeshyMcLighting" or whatever is set as **HOSTNAME** in `definitions.h`
* Enter router WiFi SSID and Password
* After sucessful connection, look in "Serial Monitor" for WiFi channel of the connected WiFi. It will be same channel set on your WiFi router.

**CLIENTS** (Connects to Server)

* Take a note of this WiFi channel and enter it in `#define STATION_WIFI_CHANNEL wifi_channel_from_previous_step` in `definitions.h` in clients
* Compile **Clients** with this updated value
* Do not enable webserver for clients


#### Libraries to install

In Arduino, Goto Sketch -> Include Library -> Add .ZIP Library... and point to the zip file downloaded.

* [Painless Mesh](https://gitlab.com/painlessMesh/painlessMesh/-/archive/master/painlessMesh-master.zip)
* [Arduino JSON](https://github.com/bblanchon/ArduinoJson/archive/master.zip)
* [Task Scheduler](https://github.com/arkhipenko/TaskScheduler/archive/master.zip)
* [Async TCP](https://github.com/me-no-dev/ESPAsyncTCP/archive/master.zip)
* [Async Web Server](https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip)
* [Async MQTT Client](https://github.com/marvinroger/async-mqtt-client/archive/master.zip)
* [Async WiFiManager](https://github.com/alanswx/ESPAsyncWiFiManager/archive/master.zip)

#### LED libraries
In definitions.h: `#define USE_WS2812FX` See changes needed with [WS2818FX](https://github.com/kitesurfer1404/WS2812FX/issues/89)

* [WS2812FX](https://github.com/kitesurfer1404/WS2812FX/archive/master.zip)
* [Adafruit NeoPixels](https://github.com/adafruit/Adafruit_NeoPixel/archive/master.zip)

or

In definitions.h: `#define USE_NEOANIMATIONFX`
* [NeoAnimationFX](https://github.com/debsahu/NeoAnimationFX/archive/master.zip)
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus/archive/master.zip)
