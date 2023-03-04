#include <EEPROM.h>

#include <Button2.h>
#define DEBOUNCE_TIME    40
#define BUTTON_UP_PIN     2
#define BUTTON_DOWN_PIN   3
#define BUTTON_LEFT_PIN   4
#define BUTTON_RIGHT_PIN  5
#define BUTTON_GO_PIN     6
#define BUTTON_START_PIN  8 // grounded reeds
Button2 buttonU, buttonD, buttonL, buttonR, buttonG, buttonS;

#include <Adafruit_NeoPixel.h>
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel matrix's + and - connections.
// - NeoPixel matrix's DATA-IN should pass through a 300-500 OHM RESISTOR.
#define LED_PIN    7
#define LED_COUNT 64
#define BRIGHTNESS_MIN 80
#define BRIGHTNESS_DEFAULT 200
#define BRIGHTNESS_STEP 20
#define BRIGHTNESS_ADDRESS 0
uint8_t brightness = BRIGHTNESS_DEFAULT;

Adafruit_NeoPixel matrix(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

long firstPixelHue = 0;
void rainbow() {
  matrix.rainbow(firstPixelHue, 1, 255, brightness, true);
  matrix.show(); // Update matrix with new contents
  if (firstPixelHue == 0)
    firstPixelHue = 5*256*256;
  firstPixelHue -= 256;
}

void(* softReset) (void) = 0;

enum stateType {
  state_init,
  state_play,
};

stateType state = state_init;

void pressedG(Button2& btn) {
  if (buttonL.isPressed()) {
    softReset();
  }
  if (state == state_init) {
    if (buttonR.isPressed() && buttonU.isPressed() &&
    (! buttonD.isPressed()) && (! buttonL.isPressed())) {
      state = state_play;
    } else if (buttonU.isPressed()) {
      if ((uint16_t)brightness + BRIGHTNESS_STEP <= 255){
        brightness += BRIGHTNESS_STEP;
      } else {
        brightness = 255;
      }
      EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  //    Serial.print("brightness:");
  //    Serial.println(brightness);
    } else if (buttonD.isPressed()) {
      if ((uint16_t)brightness - BRIGHTNESS_STEP >= BRIGHTNESS_MIN){
        brightness -= BRIGHTNESS_STEP;
      } else {
        brightness = BRIGHTNESS_MIN;
      }
      EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  //    Serial.print("brightness:");
  //    Serial.println(brightness);
    }
  }
}

void pressedS(Button2& btn) {
  state = state_play;
}

// TODO: remove
void click(Button2& btn) {
    Serial.println("click!");
    if (btn == buttonU) {
      Serial.println("UP");
    } else if (btn == buttonD) {
      Serial.println("DOWN");
    } else if (btn == buttonL) {
      Serial.println("LEFT");
    } else if (btn == buttonR) {
      Serial.println("RIGHT");
    }
}

void button_setup() {
  buttonU.setDebounceTime(DEBOUNCE_TIME);
  buttonU.begin(BUTTON_UP_PIN);
  buttonU.setPressedHandler(click);
  buttonD.setDebounceTime(DEBOUNCE_TIME);
  buttonD.begin(BUTTON_DOWN_PIN);
  buttonD.setPressedHandler(click);
  buttonL.setDebounceTime(DEBOUNCE_TIME);
  buttonL.begin(BUTTON_LEFT_PIN);
  buttonL.setPressedHandler(click);
  buttonR.setDebounceTime(DEBOUNCE_TIME);
  buttonR.begin(BUTTON_RIGHT_PIN);
  buttonR.setPressedHandler(click);
  buttonG.setDebounceTime(DEBOUNCE_TIME);
  buttonG.begin(BUTTON_GO_PIN);
  buttonG.setPressedHandler(pressedG);
  buttonS.setDebounceTime(DEBOUNCE_TIME);
  buttonS.begin(BUTTON_START_PIN);
  buttonS.setPressedHandler(pressedS);
}

void setup() {
  state = state_init;
  Serial.begin(115200);
  brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if (brightness < BRIGHTNESS_MIN) {
    brightness = BRIGHTNESS_DEFAULT;
    EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  }
  matrix.begin();
  matrix.show();            // Turn OFF all pixels ASAP
  matrix.setBrightness(brightness);
  button_setup();
  Serial.println("Started...");
}

void button_loop() {
  buttonU.loop();
  buttonD.loop();
  buttonL.loop();
  buttonR.loop();
  buttonG.loop();
  buttonS.loop();
}

// TODO: remove
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<matrix.numPixels(); i++) { // For each pixel in strip...
    matrix.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    matrix.show();                          //  Update strip to match
  }
}

void update_screen() {
  colorWipe(matrix.Color(255,   0,   0), 10); // Red
}

void loop() {
  if (state == state_init) {
    rainbow();
  } else if (state == state_play) {
    update_screen();
  }
  button_loop();
}
