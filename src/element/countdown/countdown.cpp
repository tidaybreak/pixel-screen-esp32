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

#include "element\countdown\countdown.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "OLEDDriver.h"
#include "element/clock/tetris/TetrisMatrixDraw.h"
#include <ezTime.h>


const int panelResX = 64;   // Number of pixels wide of each INDIVIDUAL panel module.
const int panelResY = 64;   // Number of pixels tall of each INDIVIDUAL panel module.
const int panel_chain = 2;  // Total number of panels chained one to another.

static int X_OFFSET = 15;
#define MYTIMEZONE "Asia/Shanghai"

int init_secs_ = 0;
int secs_ = 0;
int init_timestamp_ = 0;


static MatrixPanel_I2S_DMA *dma_display = get_oled();

static unsigned long animationDue = 0;
static unsigned long animationDelay = 0; // Smaller number == faster

static uint16_t myBLACK = dma_display->color565(0, 0, 0);

static TetrisMatrixDraw tetris(dma_display); // Main clock
static TetrisMatrixDraw tetris_sec(dma_display); // Main clock second
static TetrisMatrixDraw tetris_m(dma_display); // The "M" of AM/PM
static TetrisMatrixDraw tetris_pa(dma_display); // The "P" or "A" of AM/PM

static Timezone myTZ;
static unsigned long oneSecondLoopDue = 0;

static bool showColon = true;
static volatile bool finishedAnimating = false;
static bool displayIntro = true;

static String lastDisplayedTime = "";
static String lastDisplayedTimeSec = "";
static String lastDisplayedAmPm = "";

static const int y_offset = panelResY / 2;

// This method is for controlling the tetris library draw calls
static void animationHandler()
{

  if (!finishedAnimating) {
    // 先清楚秒方块痕迹
    while(!tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset)) {}

    dma_display->fillScreen(myBLACK);

    finishedAnimating = tetris.drawNumbers(2 + X_OFFSET, 10 + y_offset, showColon);

    uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
    int y = 10 + y_offset - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
    int x = X_OFFSET + 16 * 2 + 2 + 2;
    tetris_sec.drawColon(x, y, colour);

    tetris_sec.drawNumbers(70 + X_OFFSET, 10 + y_offset);

    dma_display->flipDMABuffer();

  }
}

String secondsToTimeFormat(unsigned long totalSeconds) {
  unsigned int h = totalSeconds / 3600;
  unsigned int i = (totalSeconds / 60) % 60;
  unsigned int s = totalSeconds % 60;

  
  // 使用 String 类型构建时间字符串
  String timeString = (h < 10 ? "0" : "") +  String(h) + ":" + (i < 10 ? "0" : "") + String(i);

  //Serial.println(timeString + " " + String(i));
 
  return timeString;
}

static String setMatrixTime() {
  String timeString = "";
  String AmPmString = "";
  
  timeString = secondsToTimeFormat(secs_);
  X_OFFSET = 15; 
  
  //Serial.println(timeString);

  if (lastDisplayedTime != timeString) {
    Serial.println(timeString);
    lastDisplayedTime = timeString;
    tetris.setTime(timeString, false);
  }
  
  String minString = String(int(secs_ / 60));
  if (minString.length() == 1) {
    minString = "0" + minString;
  }

  String secString = String(secs_ % 60);
  if (secString.length() == 1) {
    secString = "0" + secString;
  }

  //tetris_sec.setNumbers(secString, false);

  finishedAnimating = false;
  return minString + " " + secString;
}

static bool isinit = false;
void element_countdown_setup(int secs) {
  Serial.println("countdown:" + String(secs));

 
  dma_display = get_oled();

  if (dma_display == nullptr) {
    return ;
  }

  dma_display->fillScreen(myBLACK);
  tetris.display = dma_display; // Main clock
  tetris_sec.display = dma_display; // Main clock
  tetris_m.display = dma_display; // The "M" of AM/PM
  tetris_pa.display = dma_display; // The "P" or "A" of AM/PM


  setServer("ntp.aliyun.com");
  waitForSync(5000);

  init_secs_ = secs;
  secs_ = secs;
  init_timestamp_ = now();

  finishedAnimating = false;
  displayIntro = false;
  tetris.scale = 2;
  tetris_sec.scale = 2;
 
  //tetris.setTime("00:00", true);
}

bool element_countdown_loop() {
  int point_x = 32;
  int point_y = 10 + y_offset - (TETRIS_Y_DROP_DEFAULT * tetris.scale);

  secs_ = init_secs_ - (now() - init_timestamp_);
  if (secs_ <= 0) {
    text("00 00", 1, 15, 45, "FFFFFF", 1, "FreeSans18pt7b", false);
    tetris_sec.drawColon(point_x, point_y, tetris.tetrisWHITE);
    text("时间到啦", 0, 35, 2, "FFA500", 1, NULL, true);    
    delay(10000);
    return true;
  }
  
  if (dma_display == nullptr) {
    return true;
  }

  unsigned long now = millis();
  if (now > oneSecondLoopDue) {
    String secString = setMatrixTime();
    text(secString, 1, 15, 45, "FFFFFF", 1, "FreeSans18pt7b", false);

    uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
    tetris_sec.drawColon(point_x, point_y, colour);
    showColon = !showColon;

    text("倒计时", 0, 45, 2, "FFA500", 1, NULL, true);
    Serial.println(String(secs_));


    oneSecondLoopDue = now + 1000;
  }

  now = millis();
  if (now >= animationDue) {
    //animationHandler();
    animationDue = now + animationDelay;
  }
  return false;
}