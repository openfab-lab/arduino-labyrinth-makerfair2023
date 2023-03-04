#include <Button2.h>

// A basic everyday NeoPixel matrix test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel matrix's + and - connections.
// - NeoPixel matrix's DATA-IN should pass through a 300-500 OHM RESISTOR.

#include <Adafruit_NeoPixel.h>
#define LED_PIN    7
#define LED_COUNT 64

Adafruit_NeoPixel matrix(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel matrix
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)



void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    // matrix.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    matrix.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // matrix.rainbow(firstPixelHue, 1, 255, 255, true);
    matrix.show(); // Update matrix with new contents
    delay(wait);  // Pause for a moment
  }
}

void setup() {
  matrix.begin();           // INITIALIZE NeoPixel matrix object (REQUIRED)
  matrix.show();            // Turn OFF all pixels ASAP
  matrix.setBrightness(20); // Set BRIGHTNESS to about 1/5 (max = 255)
}


void loop() {
  rainbow(10);             // Flowing rainbow cycle along the whole matrix
}


