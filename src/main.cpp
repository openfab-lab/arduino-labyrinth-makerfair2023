#include <EEPROM.h>

void(* softReset) (void) = 0;

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
#define FADE_MIN 20
#define BAR_BLINK 300
uint8_t brightness = BRIGHTNESS_DEFAULT;

Adafruit_NeoPixel matrix(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t firstPixelHue = 0;
void rainbow() {
  matrix.rainbow(firstPixelHue, 1, 255, brightness, true);
  matrix.show();
  firstPixelHue -= 256;
}

#define XSTART 0
#define YSTART 0

uint16_t getbarN(uint8_t pos) {
  if ((pos < 1) || (pos > 8))
    return LED_COUNT; // error
  return ((pos - 1) * 8);
}

uint16_t getmapN(uint8_t x, uint8_t y) {
  if ((x > 7) || (y > 4))
    return LED_COUNT; // error
  return 3 + y + (x * 8);
}

enum stateType {
  state_init,
  state_enter_program,
  state_program,
  state_play
};

stateType state;

enum dirType {
  dirU,
  dirD,
  dirL,
  dirR,
  dirEmpty
};

bool barUp;
unsigned long barTime;
uint8_t barPos;
dirType bar[8];

void enter_program() {
  matrix.clear();
  matrix.show();
  barPos = 1;
  barUp = true;
  barTime = millis();
  for (uint8_t i=0; i < 8; i++)
    bar[i] = dirEmpty;
  state = state_program;
}
void program(dirType dir) {
  if (barPos <= 8) {
    matrix.setPixelColor(getbarN(barPos), 0, 0, brightness);
    bar[barPos++ - 1] = dir;
  }
}

void pressedU(Button2& btn) {
  if (state == state_program) {
    program(dirU);
  }
}
void pressedD(Button2& btn) {
  if (state == state_program) {
    program(dirD);
  }
}
void pressedL(Button2& btn) {
  if (state == state_program) {
    program(dirL);
  }
}
void pressedR(Button2& btn) {
  if (state == state_program) {
    program(dirR);
  }
}

void pressedG(Button2& btn) {
  if (buttonL.isPressed()) {
    softReset();
  }
  if (state == state_init) {
    if (buttonR.isPressed() && buttonU.isPressed() &&
    (! buttonD.isPressed()) && (! buttonL.isPressed())) {
      state = state_enter_program;
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
  } else if (state == state_program) {
    if (barPos > 1)
      state = state_play;
  }
}

void pressedS(Button2& btn) {
  state = state_enter_program;
}

void button_setup() {
  buttonU.setDebounceTime(DEBOUNCE_TIME);
  buttonU.begin(BUTTON_UP_PIN);
  buttonU.setPressedHandler(pressedU);
  buttonD.setDebounceTime(DEBOUNCE_TIME);
  buttonD.begin(BUTTON_DOWN_PIN);
  buttonD.setPressedHandler(pressedD);
  buttonL.setDebounceTime(DEBOUNCE_TIME);
  buttonL.begin(BUTTON_LEFT_PIN);
  buttonL.setPressedHandler(pressedL);
  buttonR.setDebounceTime(DEBOUNCE_TIME);
  buttonR.begin(BUTTON_RIGHT_PIN);
  buttonR.setPressedHandler(pressedR);
  buttonG.setDebounceTime(DEBOUNCE_TIME);
  buttonG.begin(BUTTON_GO_PIN);
  buttonG.setPressedHandler(pressedG);
  buttonS.setDebounceTime(DEBOUNCE_TIME);
  buttonS.begin(BUTTON_START_PIN);
  buttonS.setPressedHandler(pressedS);
}

void setup() {
  //state = state_init; FIXME:
  state = state_enter_program;
  Serial.begin(115200);
  brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if (brightness < BRIGHTNESS_MIN) {
    brightness = BRIGHTNESS_DEFAULT;
    EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  }
  matrix.begin();
  matrix.show();            // Turn OFF all pixels ASAP
//  matrix.setBrightness(brightness);
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

uint8_t fade = FADE_MIN;
bool fadeUp = true;
void update_screen() {
  if (state == state_program) {
    // fade green on start pixel
    matrix.setPixelColor(getmapN(XSTART, YSTART), 0, (fade*brightness) >> 8, 0);
    matrix.show();
    fadeUp ? fade++ : fade--;
    if (fade == 255) fadeUp = false;
    if (fade == FADE_MIN ) fadeUp = true;
    // cursor
    if ((barPos <= 8) && (millis() - barTime > BAR_BLINK)) {
      matrix.setPixelColor(getbarN(barPos), 0, 0, barUp ?brightness : 0);
      barTime = millis();
      barUp = !barUp;
    }
  }
}

void play() {
  matrix.clear();
  for (uint8_t i=1; i < barPos; i++)
    matrix.setPixelColor(getbarN(i), 0, 0, brightness);
  matrix.show();

}

void loop() {
  if (state == state_init) {
    rainbow();
  } else if (state == state_enter_program) {
    enter_program();
  } else if (state == state_program) {
    update_screen();
  } else if (state == state_play) {
    play();
  }
  button_loop();
}

