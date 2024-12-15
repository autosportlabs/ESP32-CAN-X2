// disco LED example
// Sets all LEDs to random colors
#include <FastLED.h>

#define LED_TYPE        LED_STRIP_WS2812

#define LED_PIN 14         // change this number to be the GPIO pin connected to the LED
#define LED_BRIGHTNESS 30   // sets how bright the LED is. O is off; 255 is burn your eyeballs out (not recommended)
#define LED_COUNT 10        // 10 LEDs in the matrix

CRGB leds[LED_COUNT];           // Array to hold the LED data

void setup() {
    // Initialize the LED strip
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS);
    fill_solid(leds, LED_COUNT, CRGB::Black); // Turn off all LEDs initially
    FastLED.show();
}

void loop() {
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        byte red = random(256);
        byte green = random(256);
        byte blue = random(256);
        leds[i] = CRGB(red, green, blue);
        FastLED.show();        
    }
    delay( 100 );
} 
