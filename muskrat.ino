#include <Bounce.h>
#include <Adafruit_NeoPixel.h>

// Pin definitions
#define LED_DATA 13
#define BUTTON_1 12
#define BUTTON_2 11

// LED strip
Adafruit_NeoPixel ledStrip = Adafruit_NeoPixel(8, LED_DATA, NEO_GRB + NEO_KHZ800);
uint8_t ledStrip_BrightnessLevels[] = { 1, 5, 10, 25, 75, 150, 255 };
uint8_t ledStrip_BrightnessMax = 5; // Change to 6 to enable maximum brightness
uint8_t *ledStrip_BrightnessLevel = &ledStrip_BrightnessLevels[1];

enum {
  BRIGHTNESS_UP = 0,
  BRIGHTNESS_DOWN
};

// Button debounce
Bounce button1 = Bounce(BUTTON_1, 20);
Bounce button2 = Bounce(BUTTON_2, 20);

enum {
  FN_RAINBOW,
  FN_AUDIO,
  FN_LARSON,
  FN_LAST,
};

int function = FN_RAINBOW; // Startup task
boolean fnChanged = false;

void setup()
{
  pinMode(BUTTON_1, INPUT_PULLUP); // Configure BUTTON_1 as input with pullup
  pinMode(BUTTON_2, INPUT_PULLUP); // Configure BUTTON_2 as input with pullup
  
  ledStrip.begin();
  ledStrip.setBrightness(*ledStrip_BrightnessLevel); // Set default brightness
  ledStrip.show();  // Initialize all pixels to 'off'

  TaskAudio_setup();
  Serial.begin(9600);
}

void loop()
{
  switch(function) {
    case FN_RAINBOW:
      TaskRainbow_loop();
      break;
    case FN_AUDIO:
      TaskAudio_loop();
      break;
    case FN_LARSON:
      TaskLarson_loop();
      break;
    default:
      Serial.println("broken");
      break;
  }
}

void input()
{
  button1.update();
  button2.update();

  if (!button1.read() && !button2.read()) {
    if(!fnChanged) {
      Serial.println("change fn");
      nextFunction();
      fnChanged = true;
    }
  } else {
    fnChanged = false;
  }
  
  if (button1.fallingEdge()) {
    ledStrip_BrightnessChange(BRIGHTNESS_DOWN);
  } else if (button2.fallingEdge()) {
    ledStrip_BrightnessChange(BRIGHTNESS_UP);
  }
}

void nextFunction()
{
  function += 1;
  function %= FN_LAST;
}

void yield(uint32_t t)
{
  input();
  delay(t);
}

void ledStrip_BrightnessChange(uint8_t changeDirection)
{
  static uint8_t brightnessIndex = 0;
  
  if ((changeDirection == BRIGHTNESS_UP) &&
       (brightnessIndex < ledStrip_BrightnessMax)) {
       brightnessIndex++;
  } else if ((changeDirection == BRIGHTNESS_DOWN) &&
             (brightnessIndex > 0)) {
    brightnessIndex--;
  }

  ledStrip_BrightnessLevel = &ledStrip_BrightnessLevels[brightnessIndex];
  ledStrip.setBrightness(*ledStrip_BrightnessLevel);
}

// Slightly different, this makes the rainbow equally distributed throughout
void TaskRainbow_loop()
{
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i< ledStrip.numPixels(); i++) {
      ledStrip.setPixelColor(i, Wheel((i * 256 / ledStrip.numPixels() + j) & 255));
    }
    ledStrip.show();
    if(function != FN_RAINBOW) {
      return;
    }
    yield(5);
  }
}

void TaskLarson_loop() 
{
  uint8_t i;
  uint8_t wait = 50;
  
  for (i = 0; i < 8; i++) {
    ledStrip.setPixelColor(i-3, 0x0);
    ledStrip.setPixelColor(i-2, 0x110000);
    ledStrip.setPixelColor(i-1, 0x330000);
    ledStrip.setPixelColor(i,   0xFF0000);
    ledStrip.setPixelColor(i+1, 0x330000);
    ledStrip.setPixelColor(i+2, 0x110000);
   
    ledStrip.show();
    yield(wait);
  }
  
  for (i = 7; i > 0; i--) {
    ledStrip.setPixelColor(i-2, 0x110000);
    ledStrip.setPixelColor(i-1, 0x330000);
    ledStrip.setPixelColor(i,   0xFF0000);
    ledStrip.setPixelColor(i+1, 0x330000);
    ledStrip.setPixelColor(i+2, 0x110000);
    ledStrip.setPixelColor(i+3, 0x0);

    ledStrip.show();
    yield(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return ledStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return ledStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return ledStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

