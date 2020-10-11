/*
 * Modified example copied from FastLED 3.0 Branch - originally written by Daniel Garcia
 * This example shows how to use some of FastLED's functions with the SmartMatrix Library
 * using the SmartMatrix buffers directly instead of FastLED's buffers.
 * FastLED's dithering and color balance features can't be used this way, but SmartMatrix can draw in
 * 36-bit color and so dithering may not provide much advantage.
 *
 * This example is to show a very specific use case.  Use the FastLED_Functions example for a more widely applicable example
 *
 * To push the limits for large panel sizes with more complex graphics, and specifically for the ESP32 Port:
 * the draw buffer can be accessed immediately after returning from backgroundLayer.swapBuffers(false);
 * if the SM_BACKGROUND_OPTIONS_ALLOW_IMMEDIATE_SWAP flag is set.  This flag only functions properly on the ESP32 platform.
 * 
 * For multi-platform compatibility.  Use backgroundLayer.isDrawBufferAvailable() instead of !backgroundLayer.isSwapPending() to see if
 * the draw buffer is available for access through any drawing function, or through direct pointer access using backBuffer()
 *
 * Note: enabling this flag can cause the swap of the draw and refresh buffer in the middle of a panel refresh cycle, showing a mix
 * of the old and new frames at the same time.  This hasn't visibly shown any artifacts in limited testing.
 *
 * SM_BACKGROUND_OPTIONS_ALLOW_IMMEDIATE_SWAP only makes a difference if the time spent drawing new pixels is longer than the time 
 * spent in the matrixCalc Task preparing the matrix buffer for refreshing the current frame.  To see if it is making a difference, use
 * the matrix.countFPS() function to print the FPS and compare with the flag enabled and not.  To artificially increase the drawing time
 * in this sketch to see the SM_BACKGROUND_OPTIONS_ALLOW_IMMEDIATE_SWAP flags effect, change NUM_LOOPS_TO_INCREASE_DRAWING_TIME from the
 * default of 1 to something higher (The combination of 128x64/32 panel, 240MHz refresh rate, and NUM_LOOPS_TO_INCREASE_DRAWING_TIME=4 seems to work)
 *
 * This example requires FastLED 3.0 or higher.  If you are having trouble compiling, see
 * the troubleshooting instructions here:
 * https://github.com/pixelmatix/SmartMatrix/#external-libraries
 */

// MatrixHardware configuration header needs to be included before <SmartMatrix3.h> - only ESP32 is supported
#include <MatrixHardware_ESP32_V0.h>    // This file contains multiple ESP32 hardware configurations, edit the file to define GPIOPINOUT (or add #define GPIOPINOUT with a hardcoded number before this #include)
#include <SmartMatrix3.h>
#include <FastLED.h>

#define COLOR_DEPTH 24                  // This sketch and FastLED uses type `rgb24` directly, COLOR_DEPTH must be 24
const uint16_t kMatrixWidth = 32;        // known working: 32, 64, 96, 128
const uint16_t kMatrixHeight = 32;       // known working: 16, 32, 48, 64
const uint8_t kRefreshDepth = 36;       // known working: 24, 36, 48
const uint8_t kDmaBufferRows = 4;       // known working: 2-4, use 2 to save memory, more to keep from dropping frames and automatically lowering refresh rate
const uint8_t kPanelType = SMARTMATRIX_HUB75_32ROW_MOD16SCAN;   // use SMARTMATRIX_HUB75_16ROW_MOD8SCAN for common 16x32 panels
const uint32_t kMatrixOptions = (SMARTMATRIX_OPTIONS_NONE);      // see http://docs.pixelmatix.com/SmartMatrix for options
//const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_NONE);
const uint8_t kBackgroundLayerOptions = (SM_BACKGROUND_OPTIONS_ALLOW_IMMEDIATE_SWAP);
const uint8_t kScrollingLayerOptions = (SM_SCROLLING_OPTIONS_NONE);

SMARTMATRIX_ALLOCATE_BUFFERS(matrix, kMatrixWidth, kMatrixHeight, kRefreshDepth, kDmaBufferRows, kPanelType, kMatrixOptions);
SMARTMATRIX_ALLOCATE_BACKGROUND_LAYER(backgroundLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kBackgroundLayerOptions);
SMARTMATRIX_ALLOCATE_SCROLLING_LAYER(scrollingLayer, kMatrixWidth, kMatrixHeight, COLOR_DEPTH, kScrollingLayerOptions);

// The 32bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

// We're using the x/y dimensions to map to the x/y pixels on the matrix.  We'll
// use the z-axis for "time".  speed determines how fast time moves forward.  Try
// 1 for a very slow moving effect, or 60 for something that ends up looking like
// water.
// uint16_t speed = 1; // almost looks like a painting, moves very slowly
uint16_t speed = 20; // a nice starting speed, mixes well with a scale of 100
// uint16_t speed = 33;
// uint16_t speed = 100; // wicked fast!

// Scale determines how far apart the pixels in our noise matrix are.  Try
// changing these values around to see how it affects the motion of the display.  The
// higher the value of scale, the more "zoomed out" the noise iwll be.  A value
// of 1 will be so zoomed in, you'll mostly see solid colors.

// uint16_t scale = 1; // mostly just solid colors
// uint16_t scale = 4011; // very zoomed out and shimmery
uint16_t scale = 31;

// This is the array that we keep our computed noise values in
uint8_t noise[kMatrixWidth][kMatrixHeight];

#define NUM_LOOPS_TO_INCREASE_DRAWING_TIME  1
#define PANEL_REFRESH_RATE_HZ               240

void setup() {
  // uncomment the following lines if you want to see FPS count information
  // Serial.println("resetting!");
  Serial.begin(115200);
  delay(3000);

  matrix.setRefreshRate(PANEL_REFRESH_RATE_HZ);

  matrix.addLayer(&backgroundLayer); 
  matrix.addLayer(&scrollingLayer); 
  matrix.begin();

  backgroundLayer.setBrightness(128);

  // Initialize our coordinates to some random values
  x = random16();
  y = random16();
  z = random16();

  // Show off smart matrix scrolling text
  scrollingLayer.setMode(wrapForward);
  scrollingLayer.setColor({0xff, 0xff, 0xff});
  scrollingLayer.setSpeed(15);
  scrollingLayer.setFont(font6x10);
  scrollingLayer.start("SmartMatrix & FastLED", -1);
  scrollingLayer.setOffsetFromTop((kMatrixHeight/2) - 5);
}

// Fill the x/y array of 8-bit noise values using the inoise8 function.
void fillnoise8() {
  for(int i = 0; i < kMatrixWidth; i++) {
    int ioffset = scale * i;
    for(int j = 0; j < kMatrixHeight; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset,y + joffset,z);
    }
  }
  z += speed;
}

void loop() {
  static uint8_t circlex = 0;
  static uint8_t circley = 0;

  // if sketch uses swapBuffers(false), wait to get a new backBuffer() pointer after the swap is done:
  while(!backgroundLayer.isDrawBufferAvailable());

  rgb24 *buffer = backgroundLayer.backBuffer();

  static uint8_t ihue=0;
  fillnoise8();
  for(int k = 0; k < NUM_LOOPS_TO_INCREASE_DRAWING_TIME; k++) {
    for(int i = 0; i < kMatrixWidth; i++) {
      for(int j = 0; j < kMatrixHeight; j++) {
        // We use the value at the (i,j) coordinate in the noise
        // array for our brightness, and the flipped value from (j,i)
        // for our pixel's hue.
        buffer[kMatrixWidth*j + i] = CRGB(CHSV(noise[j][i],255,noise[i][j]));

        // You can also explore other ways to constrain the hue used, like below
        // buffer[kMatrixHeight*j + i] = CRGB(CHSV(ihue + (noise[j][i]>>2),255,noise[i][j]));
      }
    }
  }
  ihue+=1;

  backgroundLayer.fillCircle(circlex % kMatrixWidth,circley % kMatrixHeight,6,CRGB(CHSV(ihue+128,255,255)));
  circlex += random16(2);
  circley += random16(2);

  // buffer is filled completely each time, use swapBuffers without buffer copy to save CPU cycles
  backgroundLayer.swapBuffers(false);
  //matrix.countFPS();      // print the loop() frames per second to Serial
}
