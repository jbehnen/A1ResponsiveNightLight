/*
 * This code operates a "night light" setup with two RGB LEDs. 
 * 
 * This night light operates in four modes, which can be changed by pressing 
 * the button connected to INPUT_BUTTON_MODE_PIN.
 * 
 * Modes:
 * 1) The brightness of the LEDs is inversely proportional to the light detected
 *    by a photoresistor [DEFAULT]
 * 2) The brightness of the LEDs is proportional to the noise detected by a
 *    microphone
 * 3) The color of the LEDs is selected by variations in a voltage divider circuit
 * 4) The color of the LEDs is selected by magnetic variations detected by a Hall 
 *    Effect sensor
 *      - This color selection only happens when the button connected to 
 *        INPUT_MAGNET_BUTTON_PIN is pressed.
 * When these modes change, the lights pulse white with the same number of
 * pulses as the number of the mode.
 *        
 * Each of the two lights can also be independently operated in each mode.
 * There are three light modes:
 *   1) Both lights are affected by the effects of the new mode [DEFAULT]
 *   2) Only the "first" light is affected.
 *   3) Only the "second" light is affected.
 * When the light mode is changed, the newly active light(s) pulse once.
 * 
 * A great deal of code in this project is influenced by the class code at 
 * https://makeabilitylab.github.io/physcomp/arduino
 *   
 * By Julia Behnen
 */

#include "src/AnalogConfigureColor/AnalogConfigureColor.h"
#include "src/AnalogConfigureIntensity/AnalogConfigureIntensity.h"
#include "src/MicrophoneConfigureIntensity/MicrophoneConfigureIntensity.h"
#include "src/RGBConverter/RGBConverter.h"

// The pin for the button that switches input modes.
const int INPUT_BUTTON_MODE_PIN = 2;
// The pin for the button that switches light modes.
const int INPUT_LIGHT_SELECT_BUTTON_PIN = 4;
// The pin for the button that allows magnetic input
// to influence color selection.
const int INPUT_MAGNET_BUTTON_PIN = 7;

// True if the RGB LEDs are common anode, false
// if they are common cathode.
const boolean COMMON_ANODE = false; 

// Pins for the "front" RGB LED.
const int RGB1_RED_PIN = 3;
const int RGB1_GREEN_PIN  = 5;
const int RGB1_BLUE_PIN  = 6;

// Pins for the "back" RGB LED.
const int RGB2_RED_PIN = 9;
const int RGB2_GREEN_PIN  = 10;
const int RGB2_BLUE_PIN  = 11;

const int DELAY_MS = 50; // interval in ms between updates
const byte MAX_RGB_VALUE = 255;

const int NUM_MODES = 4; // The number of input modes (see class comment). 

const int NUM_LIGHT_MODES = 3; // 0 is both lights, 1 is front light, 2 is back light.
const int PULSE_MS = 500; // light pulse length in ms
const int PULSE_SPACE_MS = 250; // length of no light between pulses in ms


const float CROSSFADE_STEP_SIZE = 0.01; // the amount that the HSL value changes each loop
                                      // when crossfading

const int INPUT_MIN_5V = 0;
const int INPUT_MAX_5V = 1024;

// All configurer values that are not constants are hand-tuned.

// Processes light input for mode 1.
AnalogConfigureIntensity _lightIntensityConfigurer(
  /* analogInputPin= */ A1, 
  /* minValue= */ INPUT_MIN_5V, 
  /* maxValue= */ INPUT_MAX_5V, 
  /* invert= */ true);
  
// Processes sound input for mode 2.
MicrophoneConfigureIntensity _microphoneIntensityConfigurer(
  /* analogInputPin= */ A2, 
  /* minValue= */ INPUT_MAX_5V/2, 
  /* maxValue= */ 900);

// Processes voltage divider (potentiometer) input for mode 3.
AnalogConfigureColor _loFiPotConfigurer(
  /* analogInputPin= */ A5,  
  /* minValue= */950, 
  /* maxValue= */ INPUT_MAX_5V);

// Processes Hall Effect (magnetic) input for mode 4.
AnalogConfigureColor _magneticConfigurer(
  /* analogInputPin= */ A0,  
  /* minValue= */515, 
  /* maxValue= */ 550);

RGBConverter _rgbConverter;

float _baseHsl1;
float _baseHsl2;
int _mode;
int _lightMode;

void setup() {
  // Set button pins to have internal pull-up resistors
  pinMode(INPUT_BUTTON_MODE_PIN, INPUT_PULLUP);
  pinMode(INPUT_LIGHT_SELECT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INPUT_MAGNET_BUTTON_PIN, INPUT_PULLUP);
  
  // Set the RGB pins to output
  pinMode(RGB1_RED_PIN, OUTPUT);
  pinMode(RGB1_GREEN_PIN, OUTPUT);
  pinMode(RGB1_BLUE_PIN, OUTPUT);

  pinMode(RGB2_RED_PIN, OUTPUT);
  pinMode(RGB2_GREEN_PIN, OUTPUT);
  pinMode(RGB2_BLUE_PIN, OUTPUT);

  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600);   

  _baseHsl1 = 0.5;
  _baseHsl2 = 0.5;
  _mode = 0;
  _lightMode = 0;
}

void loop() {
  if(isButtonPressed(INPUT_BUTTON_MODE_PIN)) {
    switchMode();
    return;
  }

  if(isButtonPressed(INPUT_LIGHT_SELECT_BUTTON_PIN)) {
    switchLightMode();
    return;
  }
  
  switch(_mode) {
    case 0: lightIntensityUpdate();
            break;
    case 1: microphoneIntensityUpdate();
            break;
    case 2: loFiPotentiometerUpdate();
            break;
    case 3: magneticConfigurerUpdate();
            break;
  }

  delay(DELAY_MS);
}

/*
 * Returns true if the button is pressed, false otherwise.
 * Assumes a pull-up resistor setup.
 * Adapted from https://makeabilitylab.github.io/physcomp/arduino/piano.html#our-simplepiano-code-on-github
 */
boolean isButtonPressed(int btnPin){
  int btnVal = digitalRead(btnPin);
  
  if(btnVal == LOW){
    // button is hooked up with pull-up resistor
    // and is in a pressed state
    digitalWrite(LED_BUILTIN, HIGH);
    return true;
  }

  // button is not pressed
  digitalWrite(LED_BUILTIN, LOW);
  return false;
}

/*
 * Switches from the current input mode to the next one 
 * and pulses white with the same number of
 * pulses as the number of the new mode.
 */
void switchMode() {
  _mode = (_mode + 1) % NUM_MODES;
  int pulseCount = _mode + 1;
  pulseColor(pulseCount, 255, 255, 255);
}

/*
 * Switches from the current light mode to the next one,
 * changing which lights are considered "active". 
 * The updated active lights each pulse once.
 */
void switchLightMode() {
  _lightMode = (_lightMode + 1) % NUM_LIGHT_MODES;
  Serial.print("_lightMode=");
  Serial.println(_lightMode);
  byte rgb[3];
  _rgbConverter.hslToRgb(getActiveBaseHsl(), 1, 0.5, rgb);
  pulseColor(/* pulseCount= */ 1, rgb[0], rgb[1], rgb[2]);
}

/*
 * Gets the base HSL (pure strong hue with no
 * decrease in intensity) for the active lights.
 */
float getActiveBaseHsl() {
  return _lightMode == 2 ? _baseHsl2 : _baseHsl1;
}

/*
 * Sets the base HSL (pure strong hue with no
 * decrease in intensity) for the active lights.
 */
float setActiveBaseHsl(float newBaseHsl) {
  if (_lightMode == 0 || _lightMode == 1) {
    _baseHsl1 = newBaseHsl;
  }

  if (_lightMode == 0 || _lightMode == 2) {
    _baseHsl2 = newBaseHsl;
  }
}

/*
 * Pulses a color with the given RGB value the number of times
 * indicated by pulseCount.
 */
void pulseColor(int pulseCount, int red, int green, int blue) {
  setColor(0, 0, 0);
  delay(PULSE_SPACE_MS);
  for (int i = 0; i < pulseCount; i++) {
    setColor(red, green, blue);
    delay(PULSE_MS);
    setColor(0, 0, 0);
    delay(PULSE_SPACE_MS);
  }
  setColor();
}

/*
 * Runs the update cycle for the light detection mode,
 * gets the returned intensity modifier, and applies
 * this modifier to the active lights.
 */
void lightIntensityUpdate() {
  float newHsl = getActiveBaseHsl() + CROSSFADE_STEP_SIZE;
  if (newHsl > 1) {
    newHsl = 0;
  }
  setActiveBaseHsl(newHsl);
  float intensity = _lightIntensityConfigurer.update();
  setHsl(newHsl, intensity);
}

/*
 * Runs the update cycle for the sound detection mode,
 * gets the returned intensity modifier, and applies
 * this modifier to the active lights.
 */
void microphoneIntensityUpdate() {
  float intensity = _microphoneIntensityConfigurer.update();
  setHsl(getActiveBaseHsl(), intensity);
}

/*
 * Runs the update cycle for the potentiometer mode,
 * gets the returned HSL value, sets it as the base
 * HSL value for the active lights, and applies it
 * to the displayed color of the active lights.
 */
void loFiPotentiometerUpdate() {
  float hsl = _loFiPotConfigurer.update();
  if (hsl > 0) {
    setActiveBaseHsl(hsl);
    setHsl(hsl, /*intensity= */ 1);
  }
}

/*
 * Runs the update cycle for the Hall Effect mode,
 * gets the returned HSL value, sets it as the base
 * HSL value for the active lights, and applies it
 * to the displayed color of the active lights.
 */
void magneticConfigurerUpdate() {
  float hsl = _magneticConfigurer.update();
  if (hsl > 0 && isButtonPressed(INPUT_MAGNET_BUTTON_PIN)) {
    setActiveBaseHsl(hsl);
    setHsl(hsl, /*intensity= */ 1);
  }
}

/*
 * Extracts the RGB value from the HSL, 
 * applies the intensity modifier to each
 * component, and sets the color of the
 * active lights to this RGB value.
 */
void setHsl(float hsl, float intensity) {
  byte rgb[3];
  _rgbConverter.hslToRgb(hsl, 1, 0.5, rgb);
  float red = rgb[0] * intensity;
  float green = rgb[1] * intensity;
  float blue = rgb[2] * intensity;
  setColor(red, green, blue); 
}

/**
 * The input RGB values are applied to active lights. Non-active lights are
 * set to their base HSL values.
 * 
 * Adapted from https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade (including
 * the remainder of the function comment).
 * 
 * setColor takes in values between 0 - 255 for the amount of red, green, and blue, respectively
 * where 255 is the maximum amount of that color and 0 is none of that color. You can illuminate
 * all colors by intermixing different combinations of red, green, and blue. 
 * 
 * This function is based on https://gist.github.com/jamesotron/766994
 */
void setColor(int red, int green, int blue)
{
  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = MAX_RGB_VALUE - red;
    green = MAX_RGB_VALUE - green;
    blue = MAX_RGB_VALUE - blue;
  }

  if (_lightMode == 0 || _lightMode == 1) {
    analogWrite(RGB1_RED_PIN, red);
    analogWrite(RGB1_GREEN_PIN, green);
    analogWrite(RGB1_BLUE_PIN, blue);  
  } else {
    byte rgb[3];
    _rgbConverter.hslToRgb(_baseHsl1, 1, 0.5, rgb);
    analogWrite(RGB1_RED_PIN, rgb[0]);
    analogWrite(RGB1_GREEN_PIN, rgb[1]);
    analogWrite(RGB1_BLUE_PIN, rgb[2]);  
  }

  if (_lightMode == 0 || _lightMode == 2) {
    analogWrite(RGB2_RED_PIN, red);
    analogWrite(RGB2_GREEN_PIN, green);
    analogWrite(RGB2_BLUE_PIN, blue);  
  } else {
    byte rgb[3];
    _rgbConverter.hslToRgb(_baseHsl2, 1, 0.5, rgb);
    analogWrite(RGB2_RED_PIN, rgb[0]);
    analogWrite(RGB2_GREEN_PIN, rgb[1]);
    analogWrite(RGB2_BLUE_PIN, rgb[2]);  
  }
}

/**
 * Sets the colors of both lights to their base HSL
 * values.
 */
void setColor() {
  byte rgb[3];
  
  _rgbConverter.hslToRgb(_baseHsl1, 1, 0.5, rgb);
  analogWrite(RGB1_RED_PIN, rgb[0]);
  analogWrite(RGB1_GREEN_PIN, rgb[1]);
  analogWrite(RGB1_BLUE_PIN, rgb[2]);  

  _rgbConverter.hslToRgb(_baseHsl2, 1, 0.5, rgb);
  analogWrite(RGB2_RED_PIN, rgb[0]);
  analogWrite(RGB2_GREEN_PIN, rgb[1]);
  analogWrite(RGB2_BLUE_PIN, rgb[2]);  
}
