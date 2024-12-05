// disco LED example
// Sets all LEDs to random colors

#include <LiteLED.h>

#define LED_TYPE        LED_STRIP_WS2812

#define LED_GPIO 14     // change this number to be the GPIO pin connected to the LED
#define LED_BRIGHT 30   // sets how bright the LED is. O is off; 255 is burn your eyeballs out (not recommended)
#define LED_COUNT 10    // 10 LEDs in the matrix

LiteLED myLED( LED_TYPE, 0 );    // create the LiteLED object; we're calling it "myLED"

void setup() {
    myLED.begin( LED_GPIO, 10 );        // initialze the myLED object.
    myLED.brightness( LED_BRIGHT );     // set the LED photon intensity level
    myLED.fill( 0x000000, 1 );           // set all of the LEDs to black
}

void loop() {
    for (int i = 0; i < LED_COUNT; i++) {
        byte red = random(256);
        byte green = random(256);
        byte blue = random(256);
        uint32_t color = (red << 16) | (green << 8) | blue;
        myLED.setPixel(i, color);  
    }
    myLED.show();
    delay( 100 );
} 
