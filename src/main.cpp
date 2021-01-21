#include <Arduino.h>
#include <FastLED.h>
#include <Wire.h>

#define NUM_LEDS 19
#define LED_DATA 10

//globals
const int lightPins[12] = {A1,A2,4,5,6,7,8,9,16,14,15,18};
int lightStates[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
int encCount = 0;
int lastEncCount = encCount;
int lightMode = 0;
int lastMode = -1;
int aState = 0;
int bState = 0;
int aLastState = 0;
bool modeLastPressed = false;
unsigned long currentMillis;
unsigned long prevMillis = 0;

//prototypes
void writeLights(int[12]);
void shiftLedsRight();
void shiftLedsLeft();

//global
byte temp;
CRGB leds[NUM_LEDS];

//constants
const int ENC_SENSITIVITY = 50;

void setup() {
  //init led strip
  FastLED.addLeds<NEOPIXEL,LED_DATA>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  for (int pin = 0; pin < 12; pin++) {
    pinMode(lightPins[pin], OUTPUT);
  }

  Wire.begin();  //join I2C as master
  Wire.setClock(400000);
}

void loop() {
  /**
   * I2C LOGIC
   */
  Wire.requestFrom(8,14);

  if (Wire.available() == 14) {
    for (int i = 0; i < 12; i++) {
      lightStates[i] = Wire.read();
    }
    
    aState = Wire.read();
    bState = Wire.read();

  //if the current state of the a output is different from the previous state, a pulse has occurred
  if (aState != aLastState) {
    if (bState != aState) {
      encCount++;
    }
    else {
      encCount--;
    }
  }

  //set the current a state as the previous a state
  aLastState = aState;

    writeLights(lightStates);
  }


  /**
   * RGB STRIP LOGIC
  */
  //timer for delays
  currentMillis = millis();

  //rainbow moving
  if (lightMode == 0) {
    //setup this light mode if it has just been switched to
    if (lastMode != 0) {
      fill_rainbow(leds, NUM_LEDS, 0, 255/NUM_LEDS*1);
      FastLED.show();
      
    }
    if (currentMillis - prevMillis >= 100) {
      prevMillis = currentMillis;
      shiftLedsRight();
      FastLED.show();
    }
  }

  //rainbow turntable tracking
  if (lightMode == 1) {
    if (encCount/ENC_SENSITIVITY > lastEncCount/ENC_SENSITIVITY) {
      shiftLedsRight();
      FastLED.show();
    }
    else if (encCount/ENC_SENSITIVITY < lastEncCount/ENC_SENSITIVITY) {
      shiftLedsLeft();
      FastLED.show();
    }
  }

  //change the light mode
  if (lightStates[11] && !modeLastPressed) {
    lightMode++;
    if (lightMode == 2) lightMode = 0;
  }
  modeLastPressed = lightStates[11];  //double press prevention
  lastMode = lightMode;

  lastEncCount = encCount;
}


void writeLights(int lightState[12]) {
  for (int i = 0; i < 12; i++) {
    if (lightState[i])
      digitalWrite(lightPins[i], HIGH);
    else
      digitalWrite(lightPins[i], LOW); 
  }
}

void shiftLedsRight() {
  CRGB first = leds[0];
  for (int i = 1; i < NUM_LEDS; i++) {
    leds[i-1] = leds[i];
  }
  leds[NUM_LEDS-1] = first;
}

void shiftLedsLeft() {
  CRGB last = leds[NUM_LEDS-1];
  for (int i = NUM_LEDS-2; i >= 0; i--) {
    leds[i+1] = leds[i];
  }
  leds[0] = last;
}