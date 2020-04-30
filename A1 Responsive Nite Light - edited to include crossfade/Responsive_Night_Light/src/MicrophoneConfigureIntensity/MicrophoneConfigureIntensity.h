#ifndef MICROPHONE_CONFIGURE_INTENSITY_H
#define MICROPHONE_CONFIGURE_INTENSITY_H

/**
* Calculates an intensity modifier based on microphone input.
* Adapted from https://learn.adafruit.com/adafruit-microphone-amplifier-breakout/measuring-sound-levels
*   
* By Julia Behnen
*/ 
class MicrophoneConfigureIntensity {
  private:
    const int _sampleWindow = 50; // The width of the sample window in ms
	
	const int _analogInputPin; // microphone input pin
	const int _minValue; // min microphone input value to be mapped to hue
	const int _maxValue; // max microphone input value to be mapped to hue
    
  public:
    // Constructor
    MicrophoneConfigureIntensity(int analogInputPin, int minValue, int maxValue);
     /**
    * Calculates and returns a multiplicative intensity modifier between 0 and 1 
	* based on the value from the _analogInputPin relative to the _minValue and _maxValue.
    * Call this function once per loop() if this intensity value is required.
    */ 
    float update();
};

#endif