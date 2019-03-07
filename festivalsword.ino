#include "FastLED.h"

FASTLED_USING_NAMESPACE


#define DATAPIN    4
#define CLOCKPIN   5
#define BUTTONPIN 2

#define CHIPSET APA102
#define COLOR_ORDER BGR

// #define FRAMES_PER_SECOND  120
// #define FRAMES_PER_SECOND  80
#define NUMPIXELS 89 // LEDS high density strip.

#define BRIGHTNESSHIGH 80
#define BRIGHTNESSLOW 20

// #define BRIGHTNESS 10

CRGB leds[NUMPIXELS];

CRGBPalette16 firePalette;

bool highBrightness = true;

int fps = 120;
int delayValue = 10;
volatile bool modeChanging = false;
void changeModeISR() {
  modeChanging = true;
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();

SimplePatternList gPatterns = { rainbow, flashingRainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm, Fire2012WithPalette};

void* currentPattern;

SimplePatternList setupFunctions = { rainbowSetup, flashingRainbowSetup, rainbowWithGlitterSetup, confettiSetup, sinelonSetup, juggleSetup, bpmSetup, Fire2012WithPaletteSetup};



uint8_t gCurrentPatternNumber = 0; // Index number of current pattern
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

uint8_t gHueDelta = 1; // set gHue delta to change rate of color shift.

void setup() {
  delay(3000);

  FastLED.addLeds<CHIPSET, DATAPIN, CLOCKPIN, COLOR_ORDER>(leds, NUMPIXELS).setCorrection(TypicalLEDStrip);

  // FastLED.setBrightness(BRIGHTNESSLOW);
  FastLED.setBrightness(BRIGHTNESSHIGH);

  // This first palette is the basic 'black body radiation' colors,
  // which run from black to red to bright yellow to white.
  // firePalette = HeatColors_p;

  // These are other ways to set up the color palette for the 'fire'.
  // First, a gradient from black to red to yellow to white -- similar to HeatColors_p
  //   firePalette = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);

  // Second, this palette is like the heat colors, but blue/aqua instead of red/yellow
  firePalette = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);

  attachInterrupt(digitalPinToInterrupt(BUTTONPIN), changeModeISR, RISING);

  setupFunctions[gCurrentPatternNumber]();
  delayValue = 1000/fps;
}

// to get these to flash, all I have to do is alternate flashing on and off each time and check if it's active before calling the relevant function.
bool flashOn = true;


void loop()
{

  if (flashOn) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();
  } else {
    fill_solid(leds, NUMPIXELS, CRGB::Black);
    flashOn = true; // this supports only some patterns using the flashing thing.
  }
  // gPatterns[gCurrentPatternNumber]();

  FastLED.show();

  delay(delayValue);

  EVERY_N_MILLISECONDS( 20 ) {
    gHue += gHueDelta;

    if (modeChanging) {
      modeChanging = false;
      nextPattern();
    }
   }

  // EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  if (gCurrentPatternNumber == ARRAY_SIZE(gPatterns) - 1) {
    highBrightness = !highBrightness;
    if (highBrightness) {
      FastLED.setBrightness(BRIGHTNESSHIGH);
    } else {
      FastLED.setBrightness(BRIGHTNESSLOW);
    }
  }
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
  // call setup function for pattern.
  setupFunctions[gCurrentPatternNumber]();
  delayValue = 1000/fps;
}

void rainbow()
{
  fill_rainbow( leds, NUMPIXELS, gHue, 7);
}

void flashingRainbow() {
  fill_rainbow( leds, NUMPIXELS, gHue, 7);
  flashOn = false;

}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUMPIXELS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUMPIXELS, 10);
  int pos = random16(NUMPIXELS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUMPIXELS, 20);
  // int pos = beatsin16( 13, 0, NUMPIXELS-1 );
  int pos = beatsin16( 30, 0, NUMPIXELS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  // uint8_t BeatsPerMinute = 62;
  uint8_t BeatsPerMinute = 160;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUMPIXELS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUMPIXELS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUMPIXELS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUMPIXELS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING  70

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUMPIXELS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUMPIXELS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUMPIXELS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUMPIXELS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUMPIXELS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);
      CRGB color = ColorFromPalette( firePalette, colorindex);
      int pixelnumber;

      pixelnumber = j;
      leds[pixelnumber] = color;
    }
}

void rainbowSetup() {
  gHueDelta = 10;
  fps = 120;
}

void flashingRainbowSetup() {
  gHueDelta = 3;
  fps = 50;
}

void rainbowWithGlitterSetup() {
  gHueDelta = 1;
  fps = 120;

}

void confettiSetup() {
  gHueDelta = 5;
  fps = 120;

}

void sinelonSetup() {
  gHueDelta = 1;
  fps = 120;

}

void juggleSetup() {
  gHueDelta = 1;
  fps = 120;

}

void bpmSetup() {
  // gHueDelta = 1;
  gHueDelta = 5;
  fps = 120;

}

void Fire2012WithPaletteSetup() {
  fps = 60;
}


// divide the whole thing into random blocks of one color that randomly appear before fading out? THat would be cool.

// randomly flash a color. # ideally only a saturated color, not bright white.
// will induce seizures.
// void randomFlash() {
//   for (size_t i = 0; i < NUMPIXELS; i++) {
//
//   }
// }
