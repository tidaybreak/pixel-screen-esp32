// https://github.com/witnessmenow/ESP32-Trinity/blob/master/examples/Projects/WifiTetrisClock/WifiTetrisClock.ino

/*******************************************************************
    Tetris clock using a 64x32 RGB Matrix that fetches its
    time over WiFi using the EzTimeLibrary.

    For use with my I2S Matrix Shield.

    Parts Used:
    ESP32 D1 Mini * - https://s.click.aliexpress.com/e/_dSi824B
    ESP32 I2S Matrix Shield (From my Tindie) = https://www.tindie.com/products/brianlough/esp32-i2s-matrix-shield/

      = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/

    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 *******************************************************************/

#include "element\clock\tetris\tetris.h"

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------


// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "OLEDDriver.h"


// This is the library for interfacing with the display

// Can be installed from the library manager (Search for "ESP32 MATRIX DMA")
// https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA

#include "element/clock/tetris/TetrisMatrixDraw.h"
// This library draws out characters using a tetris block
// amimation
// Can be installed from the library manager
// https://github.com/toblum/TetrisAnimation

#include <ezTime.h>
// Library used for getting the time and adjusting for DST
// Search for "ezTime" in the Arduino Library manager
// https://github.com/ropg/ezTime

// ----------------------------
// Dependency Libraries - each one of these will need to be installed.
// ----------------------------

// Adafruit GFX library is a dependancy for the matrix Library
// Can be installed from the library manager
// https://github.com/adafruit/Adafruit-GFX-Library


// -------------------------------------
// ------- Replace the following! ------
// -------------------------------------

#include "MyFont.h"
#include "api.h"

// Set a timezone using the following list
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones

// -------------------------------------
// -------   Matrix Config   ------
// -------------------------------------

const int panelResX = 64;   // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 64;   // Number of pixels tall of each INDIVIDUAL panel module.
const int panel_chain = 2;  // Total number of panels chained one to another.

int X_OFFSET = 15;
#define MYTIMEZONE "Asia/Shanghai"

String info_url = "";
String wd_val = "00";   // 温度
String sd_val = "00";   // 湿度
String power_val = "00";  // 功率
String kd_val = "0";  // 快递数量
int iconDay = 0;


// -------------------------------------
// -------   Clock Config   ------
// -------------------------------------

// Sets whether the clock should be 12 hour format or not.
bool twelveHourFormat = false;

// If this is set to false, the number will only change if the value behind it changes
// e.g. the digit representing the least significant minute will be replaced every minute,
// but the most significant number will only be replaced every 10 minutes.
// When true, all digits will be replaced every minute.
bool forceRefresh = false;

// -----------------------------

static MatrixPanel_I2S_DMA *dma_display = get_oled();

portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * animationTimer = NULL;

unsigned long animationDue = 0;
unsigned long animationDelay = 1000; // Smaller number == faster

static uint16_t myBLACK = dma_display->color565(0, 0, 0);

TetrisMatrixDraw tetris(dma_display); // Main clock
TetrisMatrixDraw tetris_sec(dma_display); // Main clock second
TetrisMatrixDraw tetris_m(dma_display); // The "M" of AM/PM
TetrisMatrixDraw tetris_pa(dma_display); // The "P" or "A" of AM/PM

Timezone myTZ;
unsigned long oneSecondLoopDue = 0;

bool showColon = true;
volatile bool finishedAnimating = false;
bool displayIntro = true;

String lastDisplayedTime = "";
String lastDisplayedTimeSec = "";
String lastDisplayedAmPm = "";

const int y_offset = panelResY / 2;

// This method is for controlling the tetris library draw calls
void animationHandler()
{
  // Not clearing the display and redrawing it when you
  // dont need to improves how the refresh rate appears
 
  if (!finishedAnimating) {
    // 先清楚秒方块痕迹
    while(!tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset, false, false)) {}
    while(!tetris.drawNumbers(2 + X_OFFSET, 10 + y_offset, false, false)) {}

    dma_display->fillScreen(myBLACK);

    text(myTZ.dateTime("Y:m:d"), 0, 1, 1, "ADFF2F", 1, NULL, false);

    showTQ(iconDay, 128 - 20, 0, false); // 太阳 晴


    int bottom_x = 30;
    text(power_val, 0, bottom_x + (6*4 - power_val.length()*6), 55, "FFFFFF", 1, NULL, false);
    bottom_x += 6 * 4;
    drawColorBit(bottom_x, 54, icon_power, 5, 10, false);

    bottom_x += 6 + 4;
    text(kd_val, 0, bottom_x, 55, "FFFFFF", 1, NULL, false);
    bottom_x += 6;
    drawColorBit(bottom_x, 54, icon_box, 5, 10, false);

    bottom_x += 6 + 4;
    text(wd_val, 0, bottom_x, 55, "FFFFFF", 1, NULL, false);
    bottom_x += 13;
    drawColorBit(bottom_x, 54, tianqiwd, 5, 10, false);

    bottom_x += 6 + 4;
    text(sd_val, 0, bottom_x, 55, "FFFFFF", 1, NULL, false);
    bottom_x += 13;
    drawColorBit(bottom_x, 54, tianqisd, 5, 10, false);


    // if (displayIntro) {
    //   finishedAnimating = tetris.drawText(1 + X_OFFSET, 5 + y_offset);
    // } else {
      if (twelveHourFormat) {
        // Place holders for checking are any of the tetris objects
        // currently still animating.
        bool tetris1Done = false;
        bool tetris_mDone = false;
        bool tetris_paDone = false;

        tetris1Done = tetris.drawNumbers(-6 + X_OFFSET, 10 + y_offset, showColon);
        tetris_mDone = tetris_m.drawText(56 + X_OFFSET, 9 + y_offset);

        // Only draw the top letter once the bottom letter is finished.
        if (tetris_mDone) {
          tetris_paDone = tetris_pa.drawText(56 + X_OFFSET, -1 + y_offset);
        }

        finishedAnimating = tetris1Done && tetris_mDone && tetris_paDone;

      } else {
        finishedAnimating = tetris.drawNumbers(2 + X_OFFSET, 10 + y_offset, showColon);
      }


    uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
    int y = 10 + y_offset - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
    int x = X_OFFSET + 16 * 2 + 2 + 2;
    tetris_sec.drawColon(x, y, colour);

    tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset);

    dma_display->flipDMABuffer();

  }
}

void drawIntro(int x = 0, int y = 0)
{
  dma_display->fillScreen(myBLACK);
  tetris.drawChar("P", x, y, tetris.tetrisCYAN);
  tetris.drawChar("o", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("w", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("e", x + 17, y, tetris.tetrisGREEN);
  tetris.drawChar("r", x + 22, y, tetris.tetrisBLUE);
  tetris.drawChar("e", x + 27, y, tetris.tetrisRED);
  tetris.drawChar("d", x + 32, y, tetris.tetrisWHITE);
  tetris.drawChar(" ", x + 37, y, tetris.tetrisMAGENTA);
  tetris.drawChar("b", x + 42, y, tetris.tetrisYELLOW);
  tetris.drawChar("y", x + 47, y, tetris.tetrisGREEN);
  //dma_display->flipDMABuffer();
}

void drawConnecting(int x = 0, int y = 0)
{
  dma_display->fillScreen(myBLACK);
  tetris.drawChar("C", x, y, tetris.tetrisCYAN);
  tetris.drawChar("o", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("n", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("n", x + 17, y, tetris.tetrisGREEN);
  tetris.drawChar("e", x + 22, y, tetris.tetrisBLUE);
  tetris.drawChar("c", x + 27, y, tetris.tetrisRED);
  tetris.drawChar("t", x + 32, y, tetris.tetrisWHITE);
  tetris.drawChar("i", x + 37, y, tetris.tetrisMAGENTA);
  tetris.drawChar("n", x + 42, y, tetris.tetrisYELLOW);
  tetris.drawChar("g", x + 47, y, tetris.tetrisGREEN);
  //dma_display->flipDMABuffer();
}



void handleColonAfterAnimation() {

  // It will draw the colon every time, but when the colour is black it
  // should look like its clearing it.
  uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
  // The x position that you draw the tetris animation object
  int x = twelveHourFormat ? -6 : 2;
  x += X_OFFSET;

  // The y position adjusted for where the blocks will fall from
  // (this could be better!)
  int y = 10 + y_offset - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
  tetris.drawColon(x, y, colour);
  x += 16 * 2 + 2;
  tetris.drawColon(x, y, colour);

  //dma_display->flipDMABuffer();  
}

void update_info() {
  String payload = http_get(info_url);
  //Serial.println(payload);
  if (payload.length() <= 0) {
    return;
  }
  
  StaticJsonDocument<3072> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  } else {
    power_val = doc["power"].as<String>(); 
    wd_val = doc["wd"].as<String>(); 
    sd_val = doc["sd"].as<String>();
    kd_val = doc["kd"].as<String>();

    iconDay = doc["weather"]["daily"][0]["iconDay"].as<int>(); 
    LOG_DEBUG(power_val);
  }
}


void setMatrixTime() {
  String timeString = "";
  String AmPmString = "";

  // Get time in format "01:15" or "22:15"(24 hour with leading 0)
  timeString = myTZ.dateTime("H:i");
  String hourString = myTZ.dateTime("H");
  int h = atoi(hourString.c_str());
  X_OFFSET = (h > 9 and h < 20) ? 11 : 15; 
  //Serial.println(String(h) + " " + String(X_OFFSET));
  
  //Serial.println(timeString);

  // Only update Time if its different
  if (lastDisplayedTime != timeString) {
    //Serial.println(timeString);
    lastDisplayedTime = timeString;
    tetris.setTime(timeString, forceRefresh);
    update_info();
    // Must set this to false so animation knows
    // to start again
  }

  String secString = myTZ.dateTime("s");
  if (secString.length() == 1) {
    secString = "0" + secString;
  }
  tetris_sec.setNumbers(secString, forceRefresh);

  finishedAnimating = false;

}

static bool isinit = false;
void element_clock_tetris_setup(JsonObject &node) {
  Serial.println("clock init!");

  info_url = node["url"].as<String>();
  update_info();

  if (isinit) {
    return ;
  }
  isinit = true;

  // May or may not be needed depending on your matrix
  // Example of what needing it looks like:
  // https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA/issues/134#issuecomment-866367216
  //mxconfig.clkphase = false;


  // Display Setup
  dma_display = get_oled();

  if (dma_display == nullptr) {
    return ;
  }

  dma_display->fillScreen(myBLACK);
  
  tetris.display = dma_display; // Main clock
  tetris_sec.display = dma_display; // Main clock
  tetris_m.display = dma_display; // The "M" of AM/PM
  tetris_pa.display = dma_display; // The "P" or "A" of AM/PM

  uint16_t tetrisBLACK = 0xFFFF;
  for (uint16_t i = 0; i< 9; ++i) {
    tetris.tetrisColors[i] = tetrisBLACK; 
    tetris_sec.tetrisColors[i] = tetrisBLACK; 
  }

  // // "connecting"
  // drawConnecting(45, -6 + y_offset);

  // Setup EZ Time
  //setDebug(INFO);
  setServer("ntp.aliyun.com");
  waitForSync(5000);

  Serial.println();
  Serial.println("UTC:             " + UTC.dateTime());

  myTZ.setLocation(F(MYTIMEZONE));
  Serial.print(F("Time in your set timezone:         "));
  Serial.println(myTZ.dateTime());

  // // "Powered By"
  // drawIntro(6, -4 + y_offset);
  // delay(2000);

  // // Start the Animation Timer
  // tetris.setText("TRINITY");

  // // Wait for the animation to finish
  // while (!finishedAnimating)
  // {
  //   delay(animationDelay);
  //   animationHandler();
  // }
  // delay(2000);

  finishedAnimating = false;
  displayIntro = false;
  tetris.scale = 2;
  tetris_sec.scale = 2;
 
  //tetris.setTime("00:00", true);
}

void element_clock_tetris_loop() {

  if (dma_display == nullptr) {
    return ;
  }
  unsigned long now = millis();
  if (now > oneSecondLoopDue) {
    // We can call this often, but it will only
    // update when it needs to
    setMatrixTime();



    showColon = !showColon;

    // To reduce flicker on the screen we stop clearing the screen
    // when the animation is finished, but we still need the colon to
    // to blink
    if (finishedAnimating) {
      //handleColonAfterAnimation();
    }

    oneSecondLoopDue = now + 1000;
  }
  now = millis();
  if (now >= animationDue) {
    animationHandler();
    animationDue = now + animationDelay;
  }
}