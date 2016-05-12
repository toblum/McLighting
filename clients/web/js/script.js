(function($){
  $(function(){
	  
	var host = "192.168.0.24";

    $('.button-collapse').sideNav();
	
	
	// Navlinks
	$('#mc-nav').on('click', '.mc-navlink', function(){
		console.log("Nav to: ", $(this).data("pane"));
		showPane($(this).data("pane"));
	});
	
	function showPane(pane) {
		$('.mc_pane').addClass('hide');
		$('#' + pane).removeClass('hide');
		$('.button-collapse').sideNav('hide');
	}
	
	
	// ******************************************************************
	// init()
	// ******************************************************************
	var connection = new WebSocket('ws://' + host + ':81', ['arduino']);
			
	// When the connection is open, send some data to the server
	connection.onopen = function () {
		//connection.send('Ping'); // Send the message 'Ping' to the server
		console.log('WebSocket Open');
		showPane('pane1');
	};

	// Log errors
	connection.onerror = function (error) {
		console.log('WebSocket Error ' + error);
		$('#mc-wsloader').addClass('hide');
		$('#mc-wserror').removeClass('hide');
	};

	// Log messages from the server
	connection.onmessage = function (e) {
		console.log('Server: ' + e.data);
	};
	
	
	// ******************************************************************
	// Modes
	// ******************************************************************	
	$("#pane2").on("click", ".btn_mode", function() {
		var mode = $(this).attr("data-mode");
		last_mode = mode;
		var btn = $(this);
		setMode(mode, function() {
			$(".btn_mode").removeClass("blue");
			btn.addClass("blue");
		});
	});
	
	function setMode(mode, finish_funtion) {
		var url = "http://" + host + "/" + mode;
		console.log("Mode: ", mode);
		
		var red = $("#rng_red").val();
		var green = $("#rng_green").val();
		var blue = $("#rng_blue").val();
		var delay = $("#rng_delay").val();
		
		var params = {"r":red, "g":green, "b":blue, "d":delay};
		connection.send("=" + mode);
		
		/*
		$.getJSON(url, params, function(data) {
			updateStatus(data);
			finish_funtion();
		});
		*/
	}
	
	function updateStatus(data) {
		console.log("Returned: ", data);
		$("#result").val("Mode: " + data.mode + "\nColor: "+ data.color[0] + "," + data.color[1] + "," + data.color[2] + "\nDelay:" + data.delay_ms + "\nBrightness:" + data.brightness);
		$('#result').trigger('autoresize');
	}
	
	
	// ******************************************************************
	// Colorwheel
	// ******************************************************************
	// this is supposed to work on mobiles (touch) as well as on a desktop (click)
	// since we couldn't find a decent one .. this try of writing one by myself
	// + google. swiping would be really nice - I will possibly implement it with
	// jquery later - or never.

	var canvas = document.getElementById("myCanvas");
	// FIX: Cancel touch end event and handle click via touchstart
	// canvas.addEventListener("touchend", function(e) { e.preventDefault(); }, false);
	canvas.addEventListener("touchmove", doTouch, false);
	canvas.addEventListener("click", doClick, false);
	
	var context = canvas.getContext('2d');
	var centerX = canvas.width / 2;
	var centerY = canvas.height / 2;
	var innerRadius = canvas.width / 4.5;
	var outerRadius = (canvas.width - 10) / 2

	//outer border
	context.beginPath();
	//outer circle
	context.arc(centerX, centerY, outerRadius, 0, 2 * Math.PI, false);
	//draw the outer border: (gets drawn around the circle!)
	context.lineWidth = 4;
	context.strokeStyle = '#000000';
	context.stroke();
	context.closePath();

	//fill with beautiful colors 
	//taken from here: http://stackoverflow.com/questions/18265804/building-a-color-wheel-in-html5
	for(var angle=0; angle<=360; angle+=1) {
		var startAngle = (angle-2)*Math.PI/180;
		var endAngle = angle * Math.PI/180;
		context.beginPath();
		context.moveTo(centerX, centerY);
		context.arc(centerX, centerY, outerRadius, startAngle, endAngle, false);
		context.closePath();
		context.fillStyle = 'hsl('+angle+', 100%, 50%)';
		context.fill();
		context.closePath();
	}

	//inner border
	context.beginPath();
	//context.arc(centerX, centerY, radius, startAngle, endAngle, counterClockwise);
	context.arc(centerX, centerY, innerRadius, 0, 2 * Math.PI, false);
	//fill the center
	var my_gradient=context.createLinearGradient(0,0,170,0);
	my_gradient.addColorStop(0,"black");
	my_gradient.addColorStop(1,"white");
	
	context.fillStyle = my_gradient;
	context.fillStyle = "white";
	context.fill();

	//draw the inner line
	context.lineWidth = 2;
	context.strokeStyle = '#000000';
	context.stroke();
	context.closePath();

	//get Mouse x/y canvas position
	function getMousePos(canvas, evt) {
		var rect = canvas.getBoundingClientRect();
		return {
			x: evt.clientX - rect.left,
			y: evt.clientY - rect.top
		};
	}

	//comp to Hex
	function componentToHex(c) {
		var hex = c.toString(16);
		return hex.length == 1 ? "0" + hex : hex;
	}

	//rgb/rgba to Hex
	function rgbToHex(rgb) {
		return "#" + componentToHex(rgb[0]) + componentToHex(rgb[1]) + componentToHex(rgb[2]);
	}

	//display the touch/click position and color info
	function showStatus(pos, color) {
		var hexColor = rgbToHex(color);
		
		connection.send("*" + componentToHex(color[0]) + componentToHex(color[1]) + componentToHex(color[2]));
			
		$('#status').css("backgroundColor", hexColor);
		$('#status_color').text(hexColor + " - R=" + color[0] + ", G=" + color[1] + ", B=" + color[2]);
		$('#status_pos').text("x: " + pos.x + " - y: " + pos.y);
		document.getElementById('status').style.backgroundColor=hexColor;
	}

	//handle the touch event
	function doTouch(event) {
		//to not also fire on click
		event.preventDefault();
		var el = event.target;
		
		//touch position
		var pos = {x: Math.round(event.targetTouches[0].pageX - el.offsetLeft),
				   y: Math.round(event.targetTouches[0].pageY - el.offsetTop)};
		//color
		var color = context.getImageData(pos.x, pos.y, 1, 1).data;

		showStatus(pos, color);
	}

	function doClick(event) {   
		//click position   
		var pos = getMousePos(canvas, event);
		//color
		var color = context.getImageData(pos.x, pos.y, 1, 1).data;
		
		console.log("click", pos.x, pos.y, color);
		showStatus(pos, color);
		
		//now do sth with the color rgbToHex(color);
		//don't do stuff when #000000 (outside circle and lines
	}
	
  }); // end of document ready
})(jQuery); // end of jQuery name space