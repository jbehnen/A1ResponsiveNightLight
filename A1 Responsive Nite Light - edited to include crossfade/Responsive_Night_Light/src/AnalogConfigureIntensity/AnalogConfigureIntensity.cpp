#include "AnalogConfigureIntensity.h"
#include <Arduino.h>

// Adapted from https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html#crossfading-in-the-hsl-color-space
AnalogConfigureIntensity :: AnalogConfigureIntensity(int analogInputPin, int minValue, int maxValue, bool invert)
  : _analogInputPin(analogInputPin), _minValue(minValue), _maxValue(maxValue), _invert(invert) {
	  pinMode(_analogInputPin, INPUT);
}

float AnalogConfigureIntensity :: update() {
	float intensity = 0;
	float smoothedAnalogValue = 0;

	int analogValue = analogRead(_analogInputPin);
	
	Serial.print(" analogValue=");
	Serial.print(analogValue);

	intensity = _invert ? 
		map(analogValue, _minValue, _maxValue, 255, 0) :
		map(analogValue, _minValue, _maxValue, 0, 255);
	intensity /= 255;
	
	Serial.print(" intensity=");
	Serial.println(intensity);

	return intensity > 0 ? intensity : 0;
}