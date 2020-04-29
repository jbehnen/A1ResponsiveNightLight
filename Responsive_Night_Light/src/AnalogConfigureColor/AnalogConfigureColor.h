#ifndef ANALOG_CONFIGURE_COLOR_H
#define ANALOG_CONFIGURE_COLOR_H

/**
* Calculates an HSL value based on analog input.
*   
* By Julia Behnen
*/ 
class AnalogConfigureColor {
  private:
    const static int _numInputReadings = 10;
  
    const int _analogInputPin; // analog input pin
	const int _minValue; // min analog input value to be mapped to hue
	const int _maxValue; // max analog input value to be mapped to hue
	
	int _inputReadingsIndex;
	int _nextInputReadingsIndex;
	float _inputReadingsTotal;
	float _inputReadings[_numInputReadings] = {0};
    
  public:
    // Constructor
    AnalogConfigureColor(int analogInputPin, int minValue, int maxValue);
    /**
    * Calculates and returns an HSL value between 0 and 1, or -1 if no appropriate
	* HSL value can be derived, based on the value from the _analogInputPin 
	* relative to the _minValue and _maxValue. 
    * Call this function once per loop() if this HSL value is required.
    */ 
    float update();
};

#endif