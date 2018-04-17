#NoEnv
#SingleInstance force ; only lets one run
SetBatchLines, -1

#Persistent ; prevents the script from exiting automatically

; Register a function to be called on exit:
OnExit("ExitFunc")

; make the notification box
notificationEnable := true ; set this to false to disable the notification popups
notificationTime := 1 ; seconds the notifications stay

n := new Notification("Starting led shortcuts!", 200,14, ,notificationTime)

; make the socket
socket := new McLightingServer("ws://192.168.1.33:81") ; replace with the ip address of the mclighting controller

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
	setNotification("Mode: " + mode)
	sendCmd(cmd) 
	; MsgBox %cmd%
	return


^#Numpad6:: ; switch modes up
	mode := mode + 1
	if(mode > maxMode)
		mode := 0
	setNotification("Mode: " + mode)
	cmd = /%mode%
	sendCmd(cmd)
	; MsgBox %cmd%
	return
	
^#Numpad5:: ; turn off
	setNotification("Leds are off")
	sendCmd("=off") ; = is the set control command
	return
	
^#Numpad0:: ; turn on to static color
	setNotification("Leds are on")
	cmd := "*"+toHexColor(red,green,blue) ; * is the set all command
	sendCmd(cmd) ; = is the set control command
	return
	
	
^#NumpadSub:: ; decreases brightness
	brightness := brightness - incrementAmount
	if(brightness < 0)
		brightness := 0
	cmd = `%%brightness% ;  % is the set brightness mode command. ` is the escape character
	setNotification("Brightness: " + brightness)
	sendCmd(cmd)
	; MsgBox %cmd%
	return
	
^#NumpadAdd:: ; increases brightness
	brightness := brightness + incrementAmount
	if(brightness > maxColor)
		brightness := maxColor
	cmd = `%%brightness% ; % is the set brightness mode command
	setNotification("Brightness: " + brightness)
	sendCmd(cmd)
	; MsgBox %cmd%
	return

^#NumpadDiv:: ; decreases speed
	speed := speed - incrementAmount
	if(speed < 0)
		speed := 0
	cmd = ?%speed% ;  ? is the set brightness mode command
	setNotification("Speed: "+ speed)
	sendCmd(cmd)
	; MsgBox %cmd%
	return
	
^#NumpadMult:: ; increases speed
	speed := speed + incrementAmount
	if(speed > maxColor)
		speed := maxColor
	cmd = ?%speed% ;  ? is the set brightness mode command
	setNotification("Speed: "+ speed)
	sendCmd(cmd)
	; MsgBox %cmd%
	return


^#Numpad7:: ; increase red amount
	red := red + incrementAmount
	if(red > maxColor)
		red := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Red: " + red)
	sendCmd(cmd)

	return
	
^#Numpad1:: ; decrease red amount
	red := red - incrementAmount
	if(red < 0)
		red := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Red: " + red)
	sendCmd(cmd)

	return

^#Numpad8:: ; increase green amount
	green := green + incrementAmount
	if(green > maxColor)
		green := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Green: " + green)
	sendCmd(cmd)

	return
	
^#Numpad2:: ; decrease green amount
	green := green - incrementAmount
	if(green < 0)
		green := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Green: " + green)
	sendCmd(cmd)

	return
	
^#Numpad9:: ; increase blue amount
	blue := blue + incrementAmount
	if(blue > maxColor)
		blue := maxColor
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Blue: " + blue)
	sendCmd(cmd)

	return
	
^#Numpad3:: ; decrease blue amount
	blue := blue - incrementAmount
	if(blue < 0)
		blue := 0
	
	cmd := "#"+toHexColor(red,green,blue) ; # is the set main color command
	; MsgBox %cmd%
	setNotification("Blue: " + blue)
	sendCmd(cmd)

	return

sendCmd(cmd)
{
	global socket
	; MsgBox connected: %connected%
	if( socket.connected )
	{
		socket.Send(cmd)
		; MsgBox %cmd%
	}
	else
		setNotification("Not connected to server!")
		
	return
}

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

setNotification(message)
{
	global notificationEnable
	global n ; the notification class
	if(notificationEnable)
	{
		n.updateMessage(message)
		n.nshow()
	}
		
	return
}

ExitFunc(ExitReason, ExitCode)
{
	; MsgBox Exiting
	global n ; notification class
	n.ndestroy() ; deletes the notification
	return 0 ; must return zero to exit
}

class McLightingServer extends WebSocket
{
	OnOpen(Event)
	{
		setNotification("Connection established!")
		; InputBox, Data, WebSocket, Enter some text to send through the websocket.
		; this.Send(Data)
		
		; global connected 
		this.connected := 1
		; MsgBox connection %connected%
	}
	
	OnMessage(Event)
	{
		; MsgBox, % "Received Data: " Event.data
		; this.Close()
	}
	
	OnClose(Event)
	{
		setNotification("Websocket Closed!")
		; MsgBox closed!
		this.connected := 0
		this.Disconnect()
		ExitApp
	}
	
	OnError(Event)
	{
		setNotification("Websocket Error")
		; MsgBox error!
		this.connected := 0
		this.Close()
	}
	
	__Delete()
	{
		; setNotification(" Exiting Led controller ") ; sticks after program closes
		ExitApp
	}
}

class WebSocket
{
	__New(WS_URL)
	{
		this.connected := false ; flag for connection status
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

class Notification
{
	__New(message, pnW=700, pnH=300, position="b r", time=10000)
	{
		this.showTime := time ; time that the notification shows up for
		
		global pn_msg
		this.pn_msg := message ; message to display
		
		Gui, Notify: +AlwaysOnTop +ToolWindow -SysMenu -Caption +LastFound
		this.PN_hwnd := WinExist()
		
		WinSet, ExStyle, +0x20
		WinSet, Transparent, 0 ; makes the box transparent so that it can fade in and out. 255 -> opaque; 0 -> transparent
		Gui, Notify: Color, 0x111111 ; background color
		Gui, Notify: Font, cWhite s10 w500, Terminal ; message color, size, weight, and font
		Gui, Notify: Add, Text, % " x" 20 " y" 12 " vpn_msg", % this.pn_msg ; add message
		RealW := pnW + 50 
		RealH := pnH + 20
		Gui, Notify: Show, W%RealW% H%RealH% NoActivate
		this.WinMove(this.PN_hwnd, position)
		global windowID := ("ahk_id "this.PN_hwnd) ; window id
		this.nshow()
	}
	
	ndestroy() {
		this.winfade(windowID,0,50) ; fades the box out
		Gui, Notify: Destroy
		return
	}

	nhide(){ ; called by the timer
		showTimer:
		SetTimer, showTimer, Off ; turn off the timer
		global windowID
		winfade(windowID,0,50) ; fades the box out
		return
	}

	nshow(){
		w:= ("ahk_id "+this.PN_hwnd)

		WinGet,s,Transparent,%w% ; makes the notification visible
		s:=(s="")?255:s ;prevent trans unset bug
		WinSet,Transparent,210,%w%

		Closetick := this.showTime*1000
		SetTimer, showTimer, % Closetick ; reset the hide timer
		return
	}

	updateMessage(message) {
		this.pn_msg := message
		GuiControl, Notify: Text, pn_msg, % this.pn_msg
		return
	}

	WinMove(hwnd,position) {
	   SysGet, Mon, MonitorWorkArea
	   WinGetPos,ix,iy,w,h, ahk_id %hwnd%
	   x := InStr(position,"l") ? MonLeft : InStr(position,"hc") ?  (MonRight-w)/2 : InStr(position,"r") ? MonRight - w : ix
	   y := InStr(position,"t") ? MonTop : InStr(position,"vc") ?  (MonBottom-h)/2 : InStr(position,"b") ? MonBottom - h : iy
	   WinMove, ahk_id %hwnd%,,x,y
	   return
	}
}

; winfade must be global for the timer to use it
winfade(w:="",t:=128,i:=1,d:=10) {
	w:=(w="")?("ahk_id " WinActive("A")):w
	t:=(t>255)?255:(t<0)?0:t
	WinGet,s,Transparent,%w%
	s:=(s="")?255:s ;prevent trans unset bug
	WinSet,Transparent,%s%,%w%
	i:=(s<t)?abs(i):-1*abs(i)
	while(k:=(i<0)?(s>t):(s<t)&&WinExist(w)) {
		WinGet,s,Transparent,%w%
		s+=i
		WinSet,Transparent,%s%,%w%
		sleep %d%
	}
	return
}
