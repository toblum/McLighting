/**
 * Welcome to Pebble.js!
 *
 * This is where you write your app.
 */

var UI = require('ui');
var Settings = require('settings');
//var ajax = require('ajax');


var wsUri = "ws://esp8266_01.local:81/";

var websocket;

var main = new UI.Card({
	title: 'Mc Lighting',
	icon: 'images/menu_icon.png',
	//subtitle: 'Hello World!',
	body: 'Loading ...'
});

var colors = [
	["White", "ffffff"],
	["Red", "ff0000"],
	["Blue", "0000ff"],
	["Green", "00ff00"],
	["Yellow", "ffff00"],
	["White", "ffffff"],
	["Pink", "ff1493"],
	["Maroon", "800000"],
	["Forest", "228b22"],
	["Orange", "ff4500"],
	["Cyan", "00ffff"]
];

var modes = ["Hold", "Off", "All", "Wipe", "Rainbow", "Rainbow Cycle", "Theater Chase", "Theater Chase Rainbow", "TV"];

// **********************************
// Init app
// **********************************
function init() {
	console.log("init()");
	
	//var options = Settings.option();
	//console.log("Settings: ", JSON.stringify(options));
	
	var esp_url = Settings.option('esp_url');
	
	if (esp_url === "") {
		//main.body('NO');
		esp_url = wsUri;
	} else {
		//main.body('YES'+esp_url);
	}
  
	// Connect Websocket
	websocket = new WebSocket(esp_url);
	websocket.onopen = function(evt) {
    	main.body('WebSocket connected!');
		console.log("init() CONN");
		
    	websocket.send("$");
  	};
  	websocket.onclose = function(evt) { 
		console.log("init() CLOSE", JSON.stringify(evt));
    	errorMessage("WebSocket connection closed");
  	};
  	websocket.onmessage = function(evt) { 
    	//console.log("WS received: "); 
		updateStatus(evt.data);
  	};
	websocket.onerror = function(evt) {
		console.log("init() ERROR", JSON.stringify(evt));
		errorMessage("WebSocket error");
	};
  
	main.show();
}

function updateStatus(msg) {
	console.log("updateStatus: ", msg);
	if (msg != "OK" && msg != "Connected") {
		try {
			var json = JSON.parse(msg);
			var str_status = '';
			if (json.mode !== undefined) {
				console.log("JSON mode: ", json.mode);

				str_status += 'Mode: ' + modes[json.mode] + "\n";
				str_status += 'Brightness: ' + Math.round(json.brightness / 2.55) + "%\n";
				str_status += 'Delay: ' + json.delay_ms + " ms\n";
				str_status += 'Color: #' + decimalToHex(json.color[0], 2) + decimalToHex(json.color[1], 2) + decimalToHex(json.color[2], 2) + "\n";

				main.body(str_status);
			}
		}
		catch(evt) {
			console.log("JSON parse error: ", JSON.stringify(evt)); 
		}
	}
}

function decimalToHex(decimal, chars) {
	return (decimal + Math.pow(16, chars)).toString(16).slice(-chars).toUpperCase();
}

function errorMessage(message) {
	var error_card = new UI.Card({
		title: 'Error occurred',
		icon: 'images/menu_icon.png',
		subtitle: 'Press any button.',
		body: message,
	});
	error_card.show();
}




// **********************************
// Main Menu
// **********************************
main.on('click', 'select', function(e) {
	var menu = new UI.Menu({
		sections: [{
			items: [
				{
					title: 'Mode',
					//icon: 'images/menu_icon.png',
					subtitle: 'Choose working mode'
				}, {
					title: 'Brightness',
					subtitle: 'Let it shine'
				}, {
					title: 'Color',
					subtitle: 'Bring color to the world'
				}, {
					title: 'Delay',
					subtitle: 'Forever faster'
				}
			]
		}]
	});
  
  
  
	// **********************************
	// Main Menu Selected
	// **********************************
	menu.on('select', function(e) {
		console.log('Selected item #' + e.itemIndex + ' of section #' + e.sectionIndex);
		console.log('The item is titled "' + e.item.title + '"');

		// Mode selection
		var mode_menu;
		if (e.itemIndex === 0) {

			mode_menu = new UI.Menu({
				sections: [{
					items: [
						{
							title: 'Off',
							subtitle: 'Turn it off'
						}, {
							title: 'All',
							subtitle: 'All LEDs on'
						}, {
							title: 'Wipe',
							subtitle: 'Wipe it bright'
						}, {
							title: 'Rainbow',
							subtitle: 'Bring color to the world'
						}, {
							title: 'Rainbow Cycle',
							subtitle: 'Like a unicorn'
						},{
							title: 'Theater Chase',
							subtitle: 'Disco'
						},{
							title: 'Theater Chase Rainbow',
							subtitle: 'Unicorn disco'
						}, {
							title: 'TV',
							subtitle: 'Video killed the radio star'
						}
					]
				}]
			});
			mode_menu.show();

			// **********************************
			// Mode Selected
			// **********************************
			mode_menu.on('select', function(e) {
				// Select mode
				console.log('Selected mode #' + e.itemIndex + ' of section #' + e.sectionIndex);

				setMode(e.itemIndex);
			});
		}


		// Brightness selection
		var brightness_menu;
		if (e.itemIndex === 1) {
			var brightness_items  = [];

			for (var i = 0; i<=10; i++) {
				var brightness_item = {
					title: (i*10) + " %",
				};
				brightness_items.push(brightness_item);
			}

			brightness_menu = new UI.Menu({
				sections: [{
					items: brightness_items
				}]
			});
			brightness_menu.show();

			// **********************************
			// Brightness Selected
			// **********************************
			brightness_menu.on('select', function(e) {
				// Select mode
				console.log('Selected mode #' + e.itemIndex + ' of section #' + e.sectionIndex);

				setBrightness(e.itemIndex);
			});
		}


		// Color selection
		var color_menu;
		if (e.itemIndex === 2) {
			var color_items  = [];

			for (var k = 0; k < colors.length; k++) {
				var color_item = {
					title: colors[k][0]
				};
				color_items.push(color_item);
			}

			color_menu = new UI.Menu({
				sections: [{
					items: color_items
				}]
			});
			color_menu.show();

			// **********************************
			// Brightness Selected
			// **********************************
			color_menu.on('select', function(e) {
				// Select mode
				console.log('Selected mode #' + e.itemIndex + ' of section #' + e.sectionIndex);

				setColor(e.itemIndex);
			});
		}


		// Delay selection
		var delay_menu;
		if (e.itemIndex === 3) {
			var delay_items  = [];

			for (var j = 0; j<=15; j++) {
				var delay_item = {
					title: Math.round(j*10) + " ms",
				};
				delay_items.push(delay_item);
			}

			delay_menu = new UI.Menu({
				sections: [{
					items: delay_items
				}]
			});
			delay_menu.show();

			// **********************************
			// Brightness Selected
			// **********************************
			delay_menu.on('select', function(e) {
				// Select mode
				console.log('Selected mode #' + e.itemIndex + ' of section #' + e.sectionIndex);

				setDelay(e.itemIndex);
			});
		}


	});
	menu.show();
});



// **********************************
// Requests
// **********************************
function setMode(mode) {
	if (mode === 0) {
		performRequest("=off");
	}
	if (mode === 1) {
		performRequest("=all");
	}
	if (mode === 2) {
		performRequest("=wipe");
	}
	if (mode === 3) {
		performRequest("=rainbow");
	}
	if (mode === 4) {
		performRequest("=rainbowCycle");
	}
	if (mode === 5) {
		performRequest("=theaterchase");
	}
	if (mode === 6) {
		performRequest("=theaterchaseRainbow");
	}
	if (mode === 7) {
		performRequest("=tv");
	}
}

function setBrightness(index) {
	var value = Math.round(index * 25.5);
	performRequest("%"+value);
}

function setColor(index) {
	var value = colors[index][1];
	performRequest("*"+value);
}

function setDelay(index) {
	var value = Math.round(index * 10);
	performRequest("?"+value);
}



function performRequest(message) {
	websocket.send(message);
}


main.on('click', 'up', function(e) {
	// Restart websocket connection
	//websocket.send("$");
	websocket.close();
	init();
});

main.on('click', 'down', function(e) {
	// Update status
	websocket.send("$");
});



// **********************************
// Settings
// **********************************
// https://pebble.github.io/pebblejs/#settings
// https://github.com/pebble/slate
// Set a configurable with just the close callback
Settings.config(
	{ url: 'https://mclighting-pebble-config.firebaseapp.com' },
	function(e) {
		console.log('closed configurable');

		// Show the parsed response
		console.log(JSON.stringify(e.options));
		init();

		// Show the raw response if parsing failed
		if (e.failed) {
			console.log(e.response);
		}
	}
);



init();
