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
#define OUTPUT_PIN        9 // to next game
Button2 buttonU, buttonD, buttonL, buttonR, buttonG, buttonS;

#include <Adafruit_NeoPixel.h>
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
#define LED_PIN    7
#define LED_COUNT 64
#define BRIGHTNESS_MIN 80
#define BRIGHTNESS_DEFAULT 200
#define BRIGHTNESS_STEP 20
#define BRIGHTNESS_ADDRESS 0
#define FADE_MIN 20
#define BAR_BLINK 300
uint8_t brightness = BRIGHTNESS_DEFAULT;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t red(uint8_t intensity = 255) {
  return strip.Color((intensity*brightness) >> 8, 0, 0);
}

uint32_t green(uint8_t intensity = 255) {
  return strip.Color(0, (intensity*brightness) >> 8, 0);
}

uint32_t blue(uint8_t intensity = 255) {
  return strip.Color(0, 0, (intensity*brightness) >> 8);
}

uint16_t firstPixelHue = 0;
void rainbow() {
  strip.rainbow(firstPixelHue, 1, 255, brightness, true);
  strip.show();
  firstPixelHue -= 256;
}

uint8_t theaterChase_i = 0;
void theaterChase(uint32_t color) {
    strip.clear();
    for(uint16_t c=theaterChase_i; c<strip.numPixels(); c += 3) {
      strip.setPixelColor(c, color);
    }
    strip.show();
    theaterChase_i++;
    if (theaterChase_i==3) theaterChase_i=0;
    delay(30);
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
  state_play,
  state_fail,
  state_success
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
  strip.clear();
  strip.show();
  delay(500);
  barPos = 1;
  barUp = true;
  barTime = millis();
  for (uint8_t i=0; i < 8; i++)
    bar[i] = dirEmpty;
  state = state_program;
}
void program(dirType dir) {
  if (barPos <= 8) {
    strip.setPixelColor(getbarN(barPos), blue());
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
  } else if (state == state_fail) {
    softReset();
  } else if (state == state_success) {
    softReset();
  }
}

void pressedS(Button2& btn) {
  if (state == state_init) {
    state = state_enter_program;
  }
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
  pinMode(OUTPUT_PIN, OUTPUT);
  digitalWrite(OUTPUT_PIN, LOW);
  state = state_init;
//  state = state_enter_program;
  Serial.begin(115200);
  brightness = EEPROM.read(BRIGHTNESS_ADDRESS);
  if (brightness < BRIGHTNESS_MIN) {
    brightness = BRIGHTNESS_DEFAULT;
    EEPROM.write(BRIGHTNESS_ADDRESS, brightness);
  }
  strip.begin();
  strip.show();            // Turn OFF all pixels ASAP
  button_setup();
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
    strip.setPixelColor(getmapN(XSTART, YSTART), green(fade));
    strip.show();
    fadeUp ? fade++ : fade--;
    if (fade == 255) fadeUp = false;
    if (fade == FADE_MIN ) fadeUp = true;
    // cursor
    if ((barPos <= 8) && (millis() - barTime > BAR_BLINK)) {
      strip.setPixelColor(getbarN(barPos), blue(barUp ? 255 : 0));
      barTime = millis();
      barUp = !barUp;
    }
  }
}

// map (0,0) is at bottom left. True = wall
bool mapwall[8][5] = {
  {0,1,1,0,0},
  {0,1,1,0,1},
  {0,1,0,0,0},
  {0,0,0,1,0},
  {0,1,0,1,0},
  {0,1,0,1,0},
  {0,1,0,0,0},
  {0,1,1,1,1}
};

bool mappath[8][5];

bool next_pos(uint8_t x, uint8_t y, dirType dir, uint8_t *nxp, uint8_t *nyp) {
  // if next pos is invalid, we return current pos
  if (dir == dirU) {
    *nxp = x;
    *nyp = y < 4 ? y + 1 : y;
  } else if (dir == dirD) {
    *nxp = x;
    *nyp = y > 0 ? y - 1 : y;
  } else if (dir == dirL) {
    *nxp = x > 0 ? x - 1 : x;
    *nyp = y;
  } else if (dir == dirR) {
    *nxp = x < 7 ? x + 1 : x;
    *nyp = y;
  }
  if (mapwall[*nxp][*nyp]) {
    *nxp = x;
    *nyp = y;
  }
  // true if new pos, false if error
  return (*nxp != x)||(*nyp != y);
}

#define SKIP 64
#define REMANENT 128
void move(uint8_t x, uint8_t y, uint8_t next_x, uint8_t next_y) {
  // fade move
  for (uint16_t i=0; i<256; i+=1) {
    if (255-i >= REMANENT)
      strip.setPixelColor(getmapN(x, y), green((uint8_t)(255-i)));
    if (i > SKIP) {
      strip.setPixelColor(getmapN(next_x, next_y), green((uint8_t)(i-SKIP)));
    }
    strip.show();
  }
  for (uint16_t i=0; i<SKIP; i+=1) {
    strip.setPixelColor(getmapN(next_x, next_y), green((uint8_t)(i+256-SKIP)));
    strip.show();
  }
}

void play() {
  uint8_t x = XSTART;
  uint8_t y = YSTART;
  for (uint8_t i=0; i<8; i++)
    for (uint8_t j=0; j<5; j++)
      mappath[i][j]=0;
  strip.clear();
  for (uint8_t i=1; i < barPos; i++)
    strip.setPixelColor(getbarN(i), blue());
  strip.setPixelColor(getmapN(x, y), green());
  mappath[x][y]=1;
  strip.show();
  for (uint8_t i=0; i < 8; i++) {
    uint8_t next_x, next_y;
    if ((bar[i] != dirEmpty) && (next_pos(x, y, bar[i], &next_x, &next_y))) {
        move(x,y,next_x, next_y);
        x = next_x;
        y = next_y;
        mappath[x][y]=1;
    } else {
      strip.setPixelColor(getmapN(x, y), red());
      for (uint8_t j=i+1; j<=barPos; j++)
        strip.setPixelColor(getbarN(j), red());
      strip.show();
      state = state_fail;
      return;
    }
  }
  state = state_success;
  digitalWrite(OUTPUT_PIN, HIGH);
  delay(1000);
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
  } else if (state == state_fail) {
  } else if (state == state_success) {
//    theaterChase(strip.Color(brightness, brightness, brightness));
  }
  button_loop();
}

