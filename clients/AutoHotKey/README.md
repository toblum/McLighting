!!! Make sure that the ip address is edited to match the ip address of your McLighting server !!!

If you are not familiar with AutoHotKey, this tutorial will get you started: https://autohotkey.com/docs/Tutorial.htm
Once you have AutoHotKey installed, double click it to start the script. 

If the Mclighting server goes down for some reason, a socket error notification will appear and you will have to restart the script to reconnect.

The script is not aware of the current state of the McLighting server and will override what is currently being displayed with its own internal values.

This has been tested on windows 7 and windows 10.
Set the notificationTime variable to the time in seconds that you want the notification to stay on the screen.
You can prevent notifications from appearing by setting the notificationEnable variable to false.


The shortcuts only work on the numberpad and only when num lock is on! If you don't have a numberpad you must change the shortcuts to something else. Just be aware that you might be overrriding previously existing shortcuts! Here is the list of windows shortcuts: https://support.microsoft.com/en-us/help/12445/windows-keyboard-shortcuts

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
