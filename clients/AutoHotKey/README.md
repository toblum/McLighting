If you are not familiar with AutoHotKey, this tutorial will get you started: https://autohotkey.com/docs/Tutorial.htm
Once you have AutoHotKey installed, double click it to start the script. 

If the Mclighting server goes down for some reason, a socket error notification will appear and you will have to restart the script to reconnect.

The script is not aware of the current state of the McLighting server and will override what is currently being displayed with its own internal values.

This was developed on Windows 7. The notification balloons look best and update in real time on Windows 7. On Windows 10, I reccommend changing the notification settings for AutoHotKey to not play a sound when they arrive. You must open the notification center on windows 10 to see the notifications in real time.

The shortcuts are as follows:
ctrl+windows+numpad4 decrements the animation mode
ctrl+windows+numpad6 increments the animation mode

ctrl+windows+numpad5 turns off the lights
ctrl+windows+numpad0 turns on all of the lights to the current color

ctrl+windows+numpadPlus increases the brightness
ctrl+windows+numpadSubtract decreases the brightness

ctrl+windows+numpadMultiply increases the speed
ctrl+windows+numpadDivide decreases the speed

ctrl+windows+numpad7 increments the red component of the current color
ctrl+windows+numpad1 decrements the red component of the current color

ctrl+windows+numpad8 increments the green component of the current color
ctrl+windows+numpad2 decrements the green component of the current color

ctrl+windows+numpad9 increments the blue component of the current color
ctrl+windows+numpad3 decrements the blue component of the current color
