#include "MicrophoneConfigureIntensity.h"
#include <Arduino.h>

// Adapted from https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
MicrophoneConfigureIntensity :: MicrophoneConfigureIntensity(int analogInputPin, int minValue, int maxValue)
  : _analogInputPin(analogInputPin), _minValue(minValue), _maxValue(maxValue) {
	  pinMode(_analogInputPin, INPUT);
}

float MicrophoneConfigureIntensity :: update() {
	float intensity = 0;

	unsigned long startMillis= millis();  // Start of sample window
	unsigned int peakToPeak = 0;   // peak-to-peak level

	unsigned int signalMax = 0;
	unsigned int signalMin = 1024;

	int analogValue;

	// collect data for 50 mS
	while (millis() - startMillis < _sampleWindow)
	{
	  analogValue = analogRead(_analogInputPin);
	  if (analogValue < 1024)  // toss out spurious readings
	  {
		 if (analogValue > signalMax)
		 {
			signalMax = analogValue;  // save just the max levels
		 }
		 else if (analogValue < signalMin)
		 {
			signalMin = analogValue;  // save just the min levels
		 }
	  }
	}
	peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
	double volts = (peakToPeak * 5.0) / 1024;  // convert to volts

	Serial.print(" analogValue=");
	Serial.print(analogValue);

	Serial.print(" peakToPeak=");
	Serial.print(peakToPeak);

	intensity = (float) peakToPeak / 1024;
	// intensity /= 1024;

	Serial.print(" intensity=");
	Serial.println(intensity);

	return intensity > 0 ? intensity : 0;
}