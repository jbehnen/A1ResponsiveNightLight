#ifndef ANALOG_CONFIGURE_INTENSITY_H
#define ANALOG_CONFIGURE_INTENSITY_H

/**
* Calculates an intensity modifier based on analog input.
*   
* By Julia Behnen
*/ 
class AnalogConfigureIntensity {
  private:
    const int _analogInputPin; // analog input pin
	const int _minValue; // min analog input value to be mapped to intensity
	const int _maxValue; // max analog input value to be mapped to intensity
	const bool _invert; // true if intensity should be inverted (1/raw intensity), false otherwise
    
  public:
    // Constructor
    AnalogConfigureIntensity(int analogInputPin, int minValue, int maxValue, bool invert);
    /**
    * Calculates and returns a multiplicative intensity modifier between 0 and 1 
	* based on the value from the _analogInputPin relative to the _minValue and _maxValue.
	* The returned intensity is inverted (1/raw intensity) if _invert is true.
    * Call this function once per loop() if this intensity value is required.
    */ 
    float update();
};

#endif