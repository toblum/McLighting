#NoEnv
#SingleInstance force ; only lets one run
SetBatchLines, -1


socket := new Example("ws://192.168.1.33:81") ; replace with the ip address of the mclighting controller

incrementAmount := 10 ; how finely you adjust the color, speed, and brightness per keypress
maxColor := 255
maxMode := 56 ; the maximum mode id number

red := 127
green := 127
blue := 127

brightness := 127
speed := 127
mode := 1


^#Numpad4:: ; switch modes down
	mode := mode - 1
	if(mode < 0)
		mode := maxMode
	cmd = /%mode% ; / is the set effect mode command
	TrayTip, , Mode: %mode%
	socket.Send(cmd)
	; MsgBox %cmd%
	return


^#Numpad6:: ; switch modes up
	mode := mode + 1
	if(mode > maxMode)
		mode := 0
	TrayTip, , Mode: %mode%
	cmd = /%mode%
	socket.Send(cmd)
	; MsgBox %cmd%
	return
	
^#Numpad5:: ; turn off
	TrayTip, , Leds are off
	socket.Send("=off") ; = is the set control command
	return
	
^#Numpad0:: ; turn on to static color
	TrayTip, , Leds are on
	cmd := "*"+toHexColor(red,green,blue) ; * is the set all command
	socket.Send(cmd) ; = is the set control command
	return
	
	
^#NumpadSub:: ; decreases brightness
	brightness := brightness - incrementAmount
	if(brightness < 0)
		brightness := 0
	cmd = `%%brightness% ;  % is the set brightness mode command. ` is the escape character
	TrayTip, , Brightness: %brightness%
	socket.Send(cmd)
	; MsgBox %cmd%
	return
	
^#NumpadAdd:: ; increases brightness
	brightness := brightness + incrementAmount
	if(brightness > maxColor)
		brightness := maxColor
	cmd = `%%brightness% ; % is the set brightness mode command
	TrayTip, , Brightness: %brightness%
	socket.Send(cmd)
	; MsgBox %cmd%
	return

^#NumpadDiv:: ; decreases speed
	speed := speed - incrementAmount
	if(speed < 0)
		speed := 0
	cmd = ?%speed% ;  ? is the set brightness mode command
	TrayTip, , Speed: %speed%
	socket.Send(cmd)
	; MsgBox %cmd%
	return
	
^#NumpadMult:: ; increases speed
	speed := speed + incrementAmount
	if(speed > maxColor)
		speed := maxColor
	cmd = ?%speed% ;  ? is the set brightness mode command
	TrayTip, , Speed: %speed%
	socket.Send(cmd)
	; MsgBox %cmd%
	return


^#Numpad7:: ; increase red amount
	red := red + incrementAmount
	if(red > maxColor)
		red := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Red: %red%
	socket.Send(cmd)

	return
	
^#Numpad1:: ; decrease red amount
	red := red - incrementAmount
	if(red < 0)
		red := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Red: %red%
	socket.Send(cmd)

	return

^#Numpad8:: ; increase green amount
	green := green + incrementAmount
	if(green > maxColor)
		green := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Green: %green%
	socket.Send(cmd)

	return
	
^#Numpad2:: ; decrease green amount
	green := green - incrementAmount
	if(green < 0)
		green := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Green: %green%
	socket.Send(cmd)

	return
	
^#Numpad9:: ; increase blue amount
	blue := blue + incrementAmount
	if(blue > maxColor)
		blue := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Blue: %blue%
	socket.Send(cmd)

	return
	
^#Numpad3:: ; decrease blue amount
	blue := blue - incrementAmount
	if(blue < 0)
		blue := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	TrayTip, , Blue: %blue%
	socket.Send(cmd)

	return
	
toHexColor(r, g, b)
{
	SetFormat, IntegerFast, hex ; To print values as hexadecimal
	colorNum := r*65536 + g*256 + b
	color = 000000%colorNum% ; convert to string and add preceeding zeros
	color := StrReplace(color, "0x") ; replace the 0x from the middle
	color := SubStr( color, -5 ) ; get last 6 characters
	
	SetFormat, IntegerFast, dec ; sets the print mode back to decimal
	return ""+color ; 
}

class Example extends WebSocket
{
	OnOpen(Event)
	{
		TrayTip, ,Connection established to led controller!
		; InputBox, Data, WebSocket, Enter some text to send through the websocket.
		; this.Send(Data)
	}
	
	OnMessage(Event)
	{
		; MsgBox, % "Received Data: " Event.data
		; this.Close()
	}
	
	OnClose(Event)
	{
		TrayTip, , Websocket Closed
		this.Disconnect()
		ExitApp
	}
	
	OnError(Event)
	{
		TrayTip, , Websocket Error
		this.Close()
	}
	
	__Delete()
	{
		TrayTip, , Exiting
		ExitApp
	}
}

class WebSocket
{
	__New(WS_URL)
	{
		static wb
		
		; Create an IE instance
		Gui, +hWndhOld
		Gui, New, +hWndhWnd
		this.hWnd := hWnd
		Gui, Add, ActiveX, vWB, Shell.Explorer
		Gui, %hOld%: Default
		
		; Write an appropriate document
		WB.Navigate("about:<!DOCTYPE html><meta http-equiv='X-UA-Compatible'"
		. "content='IE=edge'><body></body>")
		while (WB.ReadyState < 4)
			sleep, 50
		this.document := WB.document
		
		; Add our handlers to the JavaScript namespace
		this.document.parentWindow.ahk_savews := this._SaveWS.Bind(this)
		this.document.parentWindow.ahk_event := this._Event.Bind(this)
		this.document.parentWindow.ahk_ws_url := WS_URL
		
		; Add some JavaScript to the page to open a socket
		Script := this.document.createElement("script")
		Script.text := "ws = new WebSocket(ahk_ws_url);`n"
		. "ws.onopen = function(event){ ahk_event('Open', event); };`n"
		. "ws.onclose = function(event){ ahk_event('Close', event); };`n"
		. "ws.onerror = function(event){ ahk_event('Error', event); };`n"
		. "ws.onmessage = function(event){ ahk_event('Message', event); };"
		this.document.body.appendChild(Script)
	}
	
	; Called by the JS in response to WS events
	_Event(EventName, Event)
	{
		this["On" EventName](Event)
	}
	
	; Sends data through the WebSocket
	Send(Data)
	{
		; MsgBox %Data%
		this.document.parentWindow.ws.send(Data)
	}
	
	; Closes the WebSocket connection
	Close(Code:=1000, Reason:="")
	{
		this.document.parentWindow.ws.close(Code, Reason)
	}
	
	; Closes and deletes the WebSocket, removing
	; references so the class can be garbage collected
	Disconnect()
	{
		if this.hWnd
		{
			this.Close()
			Gui, % this.hWnd ": Destroy"
			this.hWnd := False
		}
	}
}
