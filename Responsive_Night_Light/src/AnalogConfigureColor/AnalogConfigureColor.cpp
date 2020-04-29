#include "AnalogConfigureColor.h"
#include <Arduino.h>

// Adapted from https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade.html#crossfading-in-the-hsl-color-space
AnalogConfigureColor :: AnalogConfigureColor(int analogInputPin, int minValue, int maxValue)
  : _analogInputPin(analogInputPin), _minValue(minValue), _maxValue(maxValue) {
	  
  _inputReadingsIndex = 0;
  _nextInputReadingsIndex = 1;
  _inputReadingsTotal = 0;
  for (int i = 0; i < _numInputReadings; i++) {
	  _inputReadings[i] = 0;
  }
  
  pinMode(_analogInputPin, INPUT);
}

float AnalogConfigureColor :: update() {
  float hsl = -1;
  float smoothedAnalogValue = 0;
  
  int analogValue = analogRead(_analogInputPin);

  if (analogValue > 0.1) {
    // Smoothing influenced by https://www.arduino.cc/en/tutorial/smoothing
    float oldValue = _inputReadings[_inputReadingsIndex];
    _inputReadingsTotal -= oldValue;
    _inputReadingsTotal += analogValue;
    
    _inputReadings[_inputReadingsIndex] = analogValue;
    _inputReadingsIndex = (_inputReadingsIndex + 1) % _numInputReadings;
  
    _nextInputReadingsIndex = (_inputReadingsIndex + 1) % _numInputReadings;

    smoothedAnalogValue = _inputReadingsTotal / _numInputReadings;
  }
  
  Serial.print("analogValue=");
  Serial.print(analogValue);
  Serial.print(" smoothedAnalogValue=");
  Serial.println(smoothedAnalogValue);
  if (smoothedAnalogValue > 0.1 && _inputReadings[_nextInputReadingsIndex] > 0 && analogValue > 0) {
    hsl = map(smoothedAnalogValue, _minValue, _maxValue, 0, 360); // pot val max 1023
    hsl /= 360;
  }
  
  return hsl;
}


