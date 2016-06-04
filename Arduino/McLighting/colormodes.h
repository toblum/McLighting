// ***************************************************************************
// Color modes
// ***************************************************************************

int dipInterval = 10;
int darkTime = 250;
unsigned long currentDipTime;
unsigned long dipStartTime;
unsigned long currentMillis;
int ledState = LOW;
long previousMillis = 0; 
int led = 5;
int interval = 2000;
int twitch = 50;
int dipCount = 0;
int analogLevel = 100;
boolean timeToDip = false;
int ledStates[NUMLEDS];

void hsb2rgbAN1(uint16_t index, uint8_t sat, uint8_t bright, uint8_t myled) {
    // Source: https://blog.adafruit.com/2012/03/14/constant-brightness-hsb-to-rgb-algorithm/
    uint8_t temp[5], n = (index >> 8) % 3;
    temp[0] = temp[3] = (uint8_t)((                                        (sat ^ 255)  * bright) / 255);
    temp[1] = temp[4] = (uint8_t)((((( (index & 255)        * sat) / 255) + (sat ^ 255)) * bright) / 255);
    temp[2] =          (uint8_t)(((((((index & 255) ^ 255) * sat) / 255) + (sat ^ 255)) * bright) / 255);

    strip.setPixelColor(myled, temp[n + 2], temp[n + 1], temp[n]);
}


void updateLed (int led, int brightness) {
	ledStates[led] = brightness;
	
	for (int i=0; i<NUMLEDS; i++)
	{
		uint16_t index = (i%3 == 0) ? 400 : random(0,767);
		hsb2rgbAN1(index, 200, ledStates[i], i);
	}
	strip.show();
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}


// See: http://forum.mysensors.org/topic/85/phoneytv-for-vera-is-here/13
void tv() {
	checkForRequests();
	if (exit_func) {
		exit_func = false;
		return;
	}
	
	if (timeToDip == false)
	{
		currentMillis = millis();
		if(currentMillis-previousMillis > interval) 
		{
			previousMillis = currentMillis;
			interval = random(750,4001);//Adjusts the interval for more/less frequent random light changes
			twitch = random(40,100);// Twitch provides motion effect but can be a bit much if too high
			dipCount = dipCount++;
		}
		if(currentMillis-previousMillis<twitch)
		{
			led=random(0, (strip.numPixels()-1));
			analogLevel=random(50,255);// set the range of the 3 pwm leds
			ledState = ledState == LOW ? HIGH: LOW; // if the LED is off turn it on and vice-versa:
			
			updateLed(led, (ledState) ? 255 : 0);
			
			if (dipCount > dipInterval)
			{ 
				DBG_OUTPUT_PORT.println("dip");
				timeToDip = true;
				dipCount = 0;
				dipStartTime = millis();
				darkTime = random(50,150);
				dipInterval = random(5,250);// cycles of flicker
			}
			//strip.show();
		} 
	}
	else
	{
		DBG_OUTPUT_PORT.println("Dip Time");
		currentDipTime = millis();
		if (currentDipTime - dipStartTime < darkTime)
		{
			for (int i=3; i<strip.numPixels(); i++)
			{
				updateLed(i, 0);
			}
		}
		else
		{
			timeToDip = false;
		}
		strip.show();
	}
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  uint16_t i;
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
    
	for (uint16_t i = 0; i < strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
		strip.show();
		
		checkForRequests();
		if (exit_func) {
			exit_func = false;
			return;
		}
		
		delay(wait);
	}
	mode = HOLD;
}

void rainbow(uint8_t wait) {
	uint16_t i, j;
	
	for (j = 0; j < 256; j++) {
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel((i + j) & 255));
		}
		strip.show();
		
		checkForRequests();
		if (exit_func) {
			exit_func = false;
			return;
		}
		
		delay(wait);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
	uint16_t i, j;
	
	for (j = 0; j < 256; j++) { // 1 cycle of all colors on wheel
		for (i = 0; i < strip.numPixels(); i++) {
			strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
		}
		strip.show();
		
		checkForRequests();
		if (exit_func) {
			exit_func = false;
			return;
		}
		
		delay(wait);
	}
}

// Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
	for (int q = 0; q < 3; q++) {
		for (int i = 0; i < strip.numPixels(); i = i + 3) {
			strip.setPixelColor(i + q, c);  //turn every third pixel on
		}
		strip.show();
		
		checkForRequests();
		if (exit_func) {
			exit_func = false;
			return;
		}
		delay(wait);
		
		for (int i = 0; i < strip.numPixels(); i = i + 3) {
			strip.setPixelColor(i + q, 0);      //turn every third pixel off
		}
	}
}

// Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
	for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
		for (int q = 0; q < 3; q++) {
			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
			}
			strip.show();
			
			checkForRequests();
			if (exit_func) {
				exit_func = false;
				return;
			}
			delay(wait);
			
			for (int i = 0; i < strip.numPixels(); i = i + 3) {
				strip.setPixelColor(i + q, 0);      //turn every third pixel off
			}
		}
	}
}



