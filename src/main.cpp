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
#define FINAL_LED_PIN     9
Button2 buttonU, buttonD, buttonL, buttonR, buttonG, buttonS;
#define DEBUG_BUTTONS 0

#include <Adafruit_NeoPixel.h>
// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
#define LED_PIN    7
#define LED_COUNT 64
#define BRIGHTNESS_MIN 10
#define BRIGHTNESS_DEFAULT 200
#define BRIGHTNESS_STEP 15
#define BRIGHTNESS_ADDRESS 0
#define FADE_MIN 20
#define BAR_BLINK 300
#define PATHRED_BLINK 100
#define PATHGREEN_BLINK 100
#define FADE_SKIP 64
#define PATH_REMANENT 32
#define SUCCESS_RAINBOW_TIME 4000
// 10 mins:
#define TIMEOUT 600000
#define FAIL_REPLAY_TIME 3000

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t red(uint8_t intensity = 255) {
  return strip.Color(intensity, 0, 0);
}

uint32_t green(uint8_t intensity = 255) {
  return strip.Color(0, intensity, 0);
}

uint32_t blue(uint8_t intensity = 255) {
  return strip.Color(0, 0, intensity);
}

uint32_t white(uint8_t intensity = 255) {
  return strip.Color(intensity/2, intensity/2, intensity/2);
}

uint16_t firstPixelHue = 0;
void rainbow() {
  strip.rainbow(firstPixelHue, 1, 255, 180);
  strip.show();
  firstPixelHue -= 256;
}

#define XSTART 0
#define YSTART 0
#define XEND 2
#define YEND 4

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
uint8_t demo;
bool demo_change = true;

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
bool pathUp;
unsigned long pathTime;
unsigned long stateTime;

void update_state(stateType new_state) {
  stateTime = millis();
  state = new_state;
}

void enter_program() {
  strip.clear();
  strip.show();
  barPos = 1;
  barUp = true;
  barTime = millis();
  for (uint8_t i=0; i < 8; i++)
    bar[i] = dirEmpty;
  update_state(state_program);
}
void program(dirType dir) {
  if (barPos <= 8) {
    strip.setPixelColor(getbarN(barPos), blue());
    bar[barPos++ - 1] = dir;
  }
}

void pressedU(Button2& btn) {
  if (DEBUG_BUTTONS) {strip.setPixelColor(getmapN(2,4), 255, 0, 0); strip.show();delay(100);};
  if (state == state_program) {
    program(dirU);
  }
}
void pressedD(Button2& btn) {
  if (DEBUG_BUTTONS) {strip.setPixelColor(getmapN(3,4), 255, 0, 0); strip.show();delay(100);};
  if (state == state_program) {
    program(dirD);
  }
}
void pressedL(Button2& btn) {
  if (DEBUG_BUTTONS) {strip.setPixelColor(getmapN(4,4), 255, 0, 0); strip.show();delay(100);};
  if (state == state_program) {
    program(dirL);
  }
}
void pressedR(Button2& btn) {
  if (DEBUG_BUTTONS) {strip.setPixelColor(getmapN(5,4), 255, 0, 0); strip.show();delay(100);};
  if (state == state_program) {
    program(dirR);
  }
}

void pressedG(Button2& btn) {
  if (DEBUG_BUTTONS) {strip.setPixelColor(getmapN(6,4), 255, 0, 0); strip.show();delay(100);};
  if (state == state_init) {
    if (buttonR.isPressed() && buttonU.isPressed() &&
    (! buttonD.isPressed()) && (! buttonL.isPressed())) {
      update_state(state_enter_program);
    } else if ((demo > 0) && buttonU.isPressed()) {
      uint8_t b = strip.getBrightness();
      if ((int16_t)b + BRIGHTNESS_STEP <= 255){
        b += BRIGHTNESS_STEP;
      } else {
        b = 255;
      }
      strip.setBrightness(b);
      EEPROM.write(BRIGHTNESS_ADDRESS, b);
    } else if ((demo > 0) && buttonD.isPressed()) {
      uint8_t b = strip.getBrightness();
      if ((int16_t)b - BRIGHTNESS_STEP >= BRIGHTNESS_MIN){
        b -= BRIGHTNESS_STEP;
      } else {
        b = BRIGHTNESS_MIN;
      }
      strip.setBrightness(b);
      EEPROM.write(BRIGHTNESS_ADDRESS, b);
    } else if (buttonL.isPressed()) {
      demo++;
      demo_change = true;
    }
  } else if (state == state_program) {
    if (buttonL.isPressed()) {
      softReset();
    }
    if (barPos > 1) {
      update_state(state_play);
    }
  } else if (state == state_success) {
    softReset();
  }
}

void pressedS(Button2& btn) {
  if (state == state_init) {
    update_state(state_enter_program);
  }
}

void button_setup() {
  buttonU.setDebounceTime(DEBOUNCE_TIME);
  buttonU.begin(BUTTON_UP_PIN);
  buttonD.setDebounceTime(DEBOUNCE_TIME);
  buttonD.begin(BUTTON_DOWN_PIN);
  buttonL.setDebounceTime(DEBOUNCE_TIME);
  buttonL.begin(BUTTON_LEFT_PIN);
  buttonR.setDebounceTime(DEBOUNCE_TIME);
  buttonR.begin(BUTTON_RIGHT_PIN);
  buttonG.setDebounceTime(DEBOUNCE_TIME);
  buttonG.begin(BUTTON_GO_PIN);
  buttonS.setDebounceTime(DEBOUNCE_TIME);
  buttonS.begin(BUTTON_START_PIN);
  // wait for Go button to be released
  while(buttonG.isPressedRaw());
  buttonU.setPressedHandler(pressedU);
  buttonD.setPressedHandler(pressedD);
  buttonL.setPressedHandler(pressedL);
  buttonR.setPressedHandler(pressedR);
  buttonG.setPressedHandler(pressedG);
  buttonS.setPressedHandler(pressedS);
}

void button_loop() {
  buttonU.loop();
  buttonD.loop();
  buttonL.loop();
  buttonR.loop();
  buttonG.loop();
  buttonS.loop();
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
uint8_t x;
uint8_t y;

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

void move(uint8_t x, uint8_t y, uint8_t next_x, uint8_t next_y) {
  // fade move
  for (uint16_t i=0; i<256; i+=1) {
    if (255-i >= PATH_REMANENT)
      strip.setPixelColor(getmapN(x, y), green((uint8_t)(255-i)));
    if (i > FADE_SKIP) {
      strip.setPixelColor(getmapN(next_x, next_y), green((uint8_t)(i-FADE_SKIP)));
    }
    strip.show();
  }
  for (uint16_t i=0; i<FADE_SKIP; i+=1) {
    strip.setPixelColor(getmapN(next_x, next_y), green((uint8_t)(i+256-FADE_SKIP)));
    strip.show();
  }
}

void pathred(bool on=true) {
  for (uint8_t i=0; i<8; i++)
    for (uint8_t j=0; j<5; j++)
      if (mappath[i][j])
        strip.setPixelColor(getmapN(i, j), red(on?PATH_REMANENT:0));
  strip.setPixelColor(getmapN(x, y), on?red():red(0));
  strip.show();
}

uint8_t pathgreen_pos;

void pathgreenstep(uint8_t x, uint8_t y, uint8_t pos){
  uint32_t color = green(PATH_REMANENT);
  if ((pathgreen_pos > 0) && (pos == pathgreen_pos - 1)) color = green(PATH_REMANENT + (255 - PATH_REMANENT)/3);
  if (pos == pathgreen_pos    ) color = green(255);
  if ((pathgreen_pos < 8) && (pos == pathgreen_pos + 1)) color = green(PATH_REMANENT + (255 - PATH_REMANENT)/3);
  strip.setPixelColor(getmapN(x, y), color);
}
void pathgreen() {
  uint8_t pos = 0;
  // hacky loop to follow hardcoded correct path
  pathgreenstep(0, 0, pos++);
  pathgreenstep(1, 0, pos++);
  pathgreenstep(2, 0, pos++);
  pathgreenstep(3, 0, pos++);
  pathgreenstep(3, 1, pos++);
  pathgreenstep(3, 2, pos++);
  pathgreenstep(2, 2, pos++);
  pathgreenstep(2, 3, pos++);
  pathgreenstep(2, 4, pos++);
  strip.show();
  if (pathgreen_pos++ > 8) pathgreen_pos = 0;
}

void play() {
  x = XSTART;
  y = YSTART;
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
      if (bar[i] == dirEmpty) {
        // path too short
          strip.setPixelColor(getbarN(i+1), red());
      } else {
        // error in middle of path
        for (uint8_t j=i+1; j<barPos; j++)
          strip.setPixelColor(getbarN(j), red());
      }
      strip.show();
      pathred();
      pathTime = millis();
      update_state(state_fail);
      return;
    }
  }
  if ((x != XEND) || (y != YEND)) {
    //all bar in red?
    // for (uint8_t j=1; j<barPos; j++)
    //   strip.setPixelColor(getbarN(j), red());
    //8th command in red?
    //strip.setPixelColor(getbarN(barPos-1), red());
    pathred();
    strip.show();
    pathTime = millis();
    update_state(state_fail);
    return;
  }
  pathTime = millis();
  update_state(state_success);
  pathgreen_pos = 0;
  digitalWrite(FINAL_LED_PIN, HIGH);
}

/////// heartbeat demo
// modified version of Firionus, cf https://forum.arduino.cc/t/making-a-heartbeat/306084
//Algorithm for fading LEDs via keyframes
//including linearity-correction (cubic)

// total cycle in ms
#define HB_FADE 480
#define HB_IMIN 80
#define HB_HMIN 190
#define HB_HMAX 200
uint8_t frames [][3] = {
  //first column: time of duration in percent
  //second column: intensity 0-255
    {  0, HB_IMIN},
    { 11, 255},
    { 20, HB_IMIN},
    { 36, HB_IMIN},
    { 48, 150},
    { 66, HB_IMIN},
    {100, HB_IMIN}
  };

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    //  return green(intensity);

   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

// void heartbeat_fill(uint8_t intensity) {
//   uint32_t c = Wheel(120-(intensity/4));
//   strip.fill(strip.ColorHSV(c,255,intensity));
//   strip.show();
// }

void heartbeat_fill(uint8_t intensity) {
  for (uint8_t x=0; x<8; x++)
    for (uint8_t y=0; y<8; y++) {
      uint8_t i = intensity;
      float cx = x-3.5;
      float cy = y-3.5;
      float d2 = cx*cx+cy*cy;
      uint8_t beatdiam = 20; // max 24.5
      if (d2 < beatdiam)
        i = ((i - 5) * (beatdiam - d2) / beatdiam) + 5;
      else
        i = 5;
      uint32_t c = Wheel(120-(i/5));
      strip.setPixelColor(y + (x * 8), strip.ColorHSV(c,255,i));
    }
  strip.show();
}

uint8_t heartbeat_c = 0;
void heartbeat() {
  float Tp1 = frames[heartbeat_c][0] / 100.0 * HB_FADE;
  float Ip1 = frames[heartbeat_c][1];
  if (heartbeat_c++ >= (sizeof(frames) / sizeof(frames[0]))) heartbeat_c=0;
  float Tp2 = frames[heartbeat_c][0] / 100.0 * HB_FADE; //get the 2 points
  float Ip2 = frames[heartbeat_c][1];
  float m = (Ip2 - Ip1) / (Tp2 - Tp1); //calculate slope of line
  if (Ip1 < Ip2) //if fade gets brighter
  {
    float T0 = (m * Tp1 - Ip1) / m;
    float Tmax = (255 + m * Tp1 - Ip1) / m;
    float k = 170 / pow(Tmax - T0, 3); //calculating constants of the cubic function
    for (float t=Tp1; t <= Tp2; t+=2) //while second point is not reached rewrite the intensity every millisecond
    {
      uint8_t i = k * pow(t - T0, 3);
      heartbeat_fill(i);
//      delay(1);
      button_loop();
    }
  }
  if (Ip1 > Ip2) //if fade gets darker
  {
    float Tmax = (m * Tp1 - Ip1) / m;
    float T0 = (255 + m * Tp1 - Ip1) / m;
    float k = 170 / pow(Tmax - T0, 3); //calc constants
    for (float t=Tp1; t <= Tp2; t+=2)
    {
      uint8_t i = k * pow(Tmax - t, 3);
      heartbeat_fill(i);
//      delay(1);
      button_loop();
    }
  }
}
/////// end of heartbeat demo

/////// labyrinth explorer demo
dirType autoplay_dir = dirL;
void autoplay() {
  uint8_t next_x = x;
  uint8_t next_y = y;
  dirType dirs[4] = {dirEmpty, dirEmpty, dirEmpty, dirEmpty};
  uint8_t idirs = 0;
  // Open paths?
  if ((autoplay_dir != dirD) && (y < 4) && (!mapwall[x][y+1])) {
    dirs[idirs++] = dirU;
  }
  if ((autoplay_dir != dirU) && (y > 0) && (!mapwall[x][y-1])) {
    dirs[idirs++] = dirD;
  }
  if ((autoplay_dir != dirL) && (x < 7) && (!mapwall[x+1][y])) {
    dirs[idirs++] = dirR;
  }
  if ((autoplay_dir != dirR) && (x > 0) && (!mapwall[x-1][y])) {
    dirs[idirs++] = dirL;
  }
  // Last resort: backwards
  if (idirs==0) {
    if (autoplay_dir == dirU) dirs[idirs++] = dirD;
    if (autoplay_dir == dirD) dirs[idirs++] = dirU;
    if (autoplay_dir == dirL) dirs[idirs++] = dirR;
    if (autoplay_dir == dirR) dirs[idirs++] = dirL;
  }
  // Choose one path
  autoplay_dir = dirs[millis()%idirs];
  if (autoplay_dir == dirU) {
    next_y = y + 1;
  } else if (autoplay_dir == dirD) {
    next_y = y - 1;
  } else if (autoplay_dir == dirL) {
    next_x = x - 1;
  } else if (autoplay_dir == dirR) {
    next_x = x + 1;
  }
  // fast fade move
  for (uint16_t i=0; i<256; i+=3) {
    if (255-i >= 0)
      strip.setPixelColor(getmapN(x, y), white((uint8_t)(255-i)));
    if (i > FADE_SKIP) {
      strip.setPixelColor(getmapN(next_x, next_y), white((uint8_t)(i-FADE_SKIP)));
    }
    strip.show();
    button_loop();
  }
  for (uint16_t i=0; i<FADE_SKIP; i+=3) {
    strip.setPixelColor(getmapN(next_x, next_y), white((uint8_t)(i+256-FADE_SKIP)));
    strip.show();
    button_loop();
  }
  x = next_x;
  y = next_y;
}
/////// end of labyrinth explorer demo

/////// bonfire demo
// modified & simplified version of https://github.com/toggledbits/MatrixFireFast
#define FIREBRIGHT 64       /* brightness; min 0 - 255 max -- high brightness requires a hefty power supply! Start low! */
#define FIREFPS 15          /* Refresh rate */
#define FLAREROWS 2         /* number of rows (from bottom) allowed to flare */
#define MAXFLARE 3          /* max number of simultaneous flares */
#define FLARECHANCE 50      /* chance (%) of a new flare (if there's room) */
#define FLAREDECAY 35       /* decay rate of flare radiation; 14 is good */

/* This is the map of colors from coolest (black) to hottest. Want blue flames? Go for it! */
const uint32_t fire_colors[] = {
  0x000000,
  0x100000,
  0x300000,
  0x600000,
  0x800000,
  0xA00000,
  0xC02000,
  0xC04000,
  0xC06000,
  0xC08000,
  0x807080
};
const uint8_t NCOLORS = (sizeof(fire_colors)/sizeof(fire_colors[0]));

uint8_t pix[8][8];
uint8_t nflare = 0;
uint32_t flare[MAXFLARE];

uint16_t pos( uint16_t col, uint16_t row ) {
  return row + col * 8;
}

uint32_t isqrt(uint32_t n) {
  if ( n < 2 ) return n;
  uint32_t s = isqrt(n >> 2) << 1;
  uint32_t l = s + 1;
  return (l*l > n) ? s : l;
}

// Set pixels to intensity around flare
void glow( int x, int y, int z ) {
  int b = z * 10 / FLAREDECAY + 1;
  for ( int i=(y-b); i<(y+b); ++i ) {
    for ( int j=(x-b); j<(x+b); ++j ) {
      if ( i >=0 && j >= 0 && i < (int)8 && j < (int)8 ) {
        int d = ( FLAREDECAY * isqrt((x-j)*(x-j) + (y-i)*(y-i)) + 5 ) / 10;
        uint8_t n = 0;
        if ( z > d ) n = z - d;
        if ( n > pix[i][j] ) { // can only get brighter
          pix[i][j] = n;
        }
      }
    }
  }
}

unsigned long fire_t = 0; /* keep time */
void make_fire() {
  uint16_t i, j;
  if ( fire_t > millis() ) return;
  fire_t = millis() + (1000 / FIREFPS);

  // First, move all existing heat points up the display and fade
  for ( i=8-1; i>0; --i ) {
    for ( j=0; j<8; ++j ) {
      uint8_t n = 0;
      if ( pix[i-1][j] > 0 )
        n = pix[i-1][j] - 1;
      pix[i][j] = n;
    }
  }

  // Heat the bottom row
  for ( j=0; j<8; ++j ) {
    i = pix[0][j];
    if ( i > 0 ) {
      pix[0][j] = random(NCOLORS-6, NCOLORS-2);
    }
  }

  // flare
  for ( i=0; i<nflare; ++i ) {
    int x = flare[i] & 0xff;
    int y = (flare[i] >> 8) & 0xff;
    int z = (flare[i] >> 16) & 0xff;
    glow( x, y, z );
    if ( z > 1 ) {
      flare[i] = (flare[i] & 0xffff) | ((uint32_t)(z-1)<<16);
    } else {
      // This flare is out
      for ( int j=i+1; j<nflare; ++j ) {
        flare[j-1] = flare[j];
      }
      --nflare;
    }
  }
  // newflare
  if ( nflare < MAXFLARE && random(1,101) <= FLARECHANCE ) {
    int x = random(0, 8);
    int y = random(0, FLAREROWS);
    int z = NCOLORS - 1;
    flare[nflare++] = ((uint32_t)z<<16) | (y<<8) | (x&0xff);
    glow( x, y, z );
  }

  // Set and draw
  for ( i=0; i<8; ++i ) {
    for ( j=0; j<8; ++j ) {
      strip.setPixelColor(pos(j,i), fire_colors[pix[i][j]]);
    }
  }
  strip.show();
}

void setup_fire(){
  for ( uint16_t i=0; i<8; ++i ) {
    for ( uint16_t j=0; j<8; ++j ) {
      if ( i == 0 ) pix[i][j] = NCOLORS - 1;
      else pix[i][j] = 0;
    }
  }
}

/////// end bonfire demo

void update_screen_state_init() {
  if (demo_change) {
    strip.clear();
    strip.show();
    demo_change = false;
  }
  switch(demo) {
    case 0:
      strip.show();
      break;
    case 1:
      rainbow();
      break;
    case 2:
      heartbeat();
      break;
    case 3:
      autoplay();
      break;
    case 4:
      make_fire();
      break;
    default:
      demo_change = true;
      demo = 0;
    break;
  }
}

uint8_t fade = FADE_MIN;
bool fadeUp = true;
void update_screen_state_program() {
  unsigned long now = millis();
  // fade green on start pixel
  strip.setPixelColor(getmapN(XSTART, YSTART), green(fade));
  strip.show();
  fadeUp ? fade++ : fade--;
  if (fade == 255) fadeUp = false;
  if (fade == FADE_MIN ) fadeUp = true;
  // cursor
  if ((barPos <= 8) && (now - barTime > BAR_BLINK)) {
    strip.setPixelColor(getbarN(barPos), blue(barUp ? 255 : 0));
    barTime = now;
    barUp = !barUp;
  }
  if ((now - stateTime) > TIMEOUT) {
    softReset();
  }
}

void update_screen_state_fail() {
  unsigned long now = millis();
  if ((now - stateTime) < FAIL_REPLAY_TIME) {
    if ((now - pathTime > PATHRED_BLINK)) {
      pathred(pathUp);
      pathTime = now;
      pathUp = !pathUp;
    }
  } else {
    update_state(state_enter_program);
  }
}

void update_screen_state_success() {
  unsigned long now = millis();
  if ((now - stateTime) < SUCCESS_RAINBOW_TIME) {
    if ((now - pathTime > PATHGREEN_BLINK)) {
      pathgreen();
      pathTime = now;
    }
  } else if ((now - stateTime) < TIMEOUT) {
    rainbow();
  } else {
    softReset();
  }
}

void setup() {
  pinMode(FINAL_LED_PIN, OUTPUT);
  digitalWrite(FINAL_LED_PIN, LOW);
  update_state(state_init);
  demo = 0;
//  update_state(state_enter_program);
  Serial.begin(115200);
  strip.begin();
  uint8_t b = EEPROM.read(BRIGHTNESS_ADDRESS);
  if (b < BRIGHTNESS_MIN) {
    b = BRIGHTNESS_DEFAULT;
    EEPROM.write(BRIGHTNESS_ADDRESS, b);
  }
  strip.setBrightness(b);
  strip.show();            // Turn OFF all pixels ASAP
  button_setup();
  setup_fire();
}

void loop() {
  switch(state) {
    case state_init:
      update_screen_state_init();
      break;
    case state_enter_program:
      enter_program();
      break;
    case state_program:
      update_screen_state_program();
      break;
    case state_play:
      play();
      break;
    case state_fail:
      update_screen_state_fail();
      break;
    case state_success:
      update_screen_state_success();
      break;
  }
  button_loop();
}
