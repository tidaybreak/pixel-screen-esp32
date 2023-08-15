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

bool car_online = false;
String car_dis = "";
String info_url = "";
String wd_val = "00";   // 温度
String sd_val = "00";   // 湿度
String power_val = "00";  // 功率
String kd_val = "0";  // 快递数量

int wnow_icon = 0;
String wnow_temp = "";
String wnow_text = "";
int w3d_d1_icon = 0;
String w3d_d1_text = "";
String w3d_d1_tempMin = "";
String w3d_d1_tempMax = "";



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
  int offset = 0;

  // 先清楚秒方块痕迹
  while(!tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset, false, false)) {}
  while(!tetris.drawNumbers(2 + X_OFFSET, 10 + y_offset, false, false)) {}

  dma_display->fillScreen(myBLACK);

  // 天气
  offset = -1;
  showTQ(wnow_icon, 0, 0, false);
  text(wnow_temp, 0, 20 + offset, 3, "808080", 2, NULL, false); // myTZ.dateTime("d")
  drawColorBit(42 + offset, 1, wd_unit, 3, 3, 0);
  text(wnow_text, 0, 45 + offset, 4, "808080", 1, NULL, false); // myTZ.dateTime("d")
  if (strlen(w3d_d1_text.c_str()) == 1) {
    offset += 12;
  }
  showTQ(w3d_d1_icon, 70 + offset, 0, false);
  text(w3d_d1_tempMax, 0, 90 + offset, 1, "808080", 1, NULL, false); // myTZ.dateTime("d")
  drawColorBit(101 + offset, 0, wd_unit, 3, 3, 0);
  text(w3d_d1_tempMin, 0, 90 + offset, 10, "808080", 1, NULL, false); // myTZ.dateTime("d")
  drawColorBit(101 + offset, 8, wd_unit, 3, 3, 0);
  text(w3d_d1_text, 0, 104 + offset, 4, "808080", 1, NULL, false); // myTZ.dateTime("d")

  // 时间 m/d
  //text(myTZ.dateTime("Y:m:d"), 0, 1, 1, "ADFF2F", 1, NULL, false);
  if (myTZ.dateTime("n").length() == 1) {
    text(myTZ.dateTime("n"), 0, 1, 23, "808080", 1, NULL, false);
    //drawChars(1 + 3, 23, myTZ.dateTime("n").c_str(), 0xFFFF, false);  // myTZ.dateTime("n").c_str()
  } else {
    drawChars(1, 23, myTZ.dateTime("n").c_str(), 0xFFFF, false);  // myTZ.dateTime("n").c_str()
  }
  text(myTZ.dateTime("d"), 0, 5, 34, "808080", 1, NULL, false); // myTZ.dateTime("d")
  drawColorBit(3, 26, line_slash, 10, 10, 0);

  // 时间 h:i:s
  offset = 0;
  finishedAnimating = tetris.drawNumbers(2 + X_OFFSET, 10 + y_offset + offset, showColon);
  uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
  int y = 10 + y_offset - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
  int x = X_OFFSET + 16 * 2 + 2 + 2;
  tetris_sec.drawColon(x, y + offset, colour);
  tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset + offset);


  // 底部
  int bottom_x = 8;
  text(car_dis, 0, bottom_x, 55, car_online ? "FFFFFF" : "808080", 1,  NULL, false);
  bottom_x += 6;
  drawColorBit(bottom_x, 54, icon_car, 12, 10, car_online ? 0 : 0x8410);
  bottom_x += 15;
  text(power_val, 0, bottom_x + (6*4 - power_val.length()*6), 55, power_val.length() <= 3 ? "808080" : "FFFFFF", 1, NULL, false);
  bottom_x += 6 * 4;
  drawColorBit(bottom_x, 54, icon_power, 5, 10, power_val.length() <= 3 ? 0x8410 : 0);
  bottom_x += 6 + 4;
  text(kd_val, 0, bottom_x, 55, kd_val == "0" ? "808080" : "FFFFFF", 1, NULL, false);
  bottom_x += 6;
  drawColorBit(bottom_x, 54, icon_box, 5, 10, kd_val == "0" ? 0x8410 : 0);
  bottom_x += 6 + 4;
  text(wd_val, 0, bottom_x, 55, "808080", 1, NULL, 0);
  bottom_x += 13;
  drawColorBit(bottom_x, 54, tianqiwd, 5, 10, 0);
  bottom_x += 6 + 4;
  text(sd_val, 0, bottom_x, 55, "808080", 1, NULL, false);
  bottom_x += 13;
  drawColorBit(bottom_x, 54, tianqisd, 5, 10, 0);

  dma_display->flipDMABuffer();

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
  
  StaticJsonDocument<3272> doc;
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
    car_dis = doc["car_distance"].as<String>();
    car_online = doc["car"].as<String>() == "off" ? false : true;

    w3d_d1_icon = doc["weather"]["3d"]["daily"][0]["iconDay"].as<int>(); 
    w3d_d1_text = doc["weather"]["3d"]["daily"][0]["textDay"].as<String>(); 
    w3d_d1_tempMin = doc["weather"]["3d"]["daily"][0]["tempMin"].as<String>(); 
    w3d_d1_tempMax = doc["weather"]["3d"]["daily"][0]["tempMax"].as<String>(); 
    wnow_icon = doc["weather"]["now"]["now"]["icon"].as<int>();
    wnow_temp = doc["weather"]["now"]["now"]["temp"].as<String>(); 
    wnow_text = doc["weather"]["now"]["now"]["text"].as<String>();
    //LOG_DEBUG(VAL(wnow_icon));
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