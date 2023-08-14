#ifndef DEFINE_H_H      
#define DEFINE_H_H


#include <pgmspace.h>
// #include <TimeLib.h>
// #include "time.h"
#include <ezTime.h>

#define TFT_BLACK       0x0      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */
#define PANEL_RES_X 64      // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64     // Number of pixels tall of each INDIVIDUAL panel module.

#define MYTIMEZONE "Asia/Shanghai"

#define MAX_STRING_LENGTH 1 * 1024


PROGMEM const unsigned char iconwifi[]= 
{0x00,0x00,0x0F,0x00,0x30,0xC0,0x40,0x20,0x0F,0x00,0x30,0xC0,0x06,0x00,0x09,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,0x00};/*"C:\Users\dudle\Pictures\wifi.bmp",0*/

struct CONF {
  int temp_mod = -3; // 温度微调参数
  int hum_mod = 0;   // 湿度微调参数
  int light = 0;
  int starnum = 20;
  // boolean soundon = true;
  // boolean caidaion = true;
  // boolean twopannel = true;
  // boolean isDoubleBuffer = true;
  // boolean isnightmode = true;
  
  // String zx_key = "";
  // String city = "";

  char str_json[MAX_STRING_LENGTH + 1];

};

struct DATATIME {
    int hou = 0;
    int minu = 0;
    int sec = 0;
    int year1 = 0; 
    int month1 = 0; 
    int day1 = 0; 
    int week = 0; 
    int sec_ten = 0; 
    int sec_one = 0;

    void GetTime() {
      year1 = year();
      month1 = month();
      day1 = day();

      hou = hour();
      minu = minute();
      sec = second();
      week = weekday() - 1; // 星期日数据是1
      if (week == 0)
        week = 15; // 星期日 显示'日'

      sec_ten = second() / 10;
      sec_one = second() % 10;
    }

    String GetTimeStr() {
      year1 = year();
      month1 = month();
      day1 = day();

      hou = hour();
      minu = minute();
      sec = second();
      week = weekday() - 1; // 星期日数据是1
      if (week == 0)
        week = 15; // 星期日 显示'日'

      sec_ten = second() / 10;
      sec_one = second() % 10;
      return String(year1) + "-" + String(year1) + "-" + String(day1) + " " + String(hou) + ":" + String(minu) + ":" + String(sec);
    }
};

struct DATACLOCK {
    String china_year;
    String china_month;
    String china_day;
    String jieqi;
    int wea_hm;
    char wea_temp1[4] = ""; // 室外
    String wea_temp2;
    String jieri;
    byte temperature = 0;
    byte humidity = 0;        // dht11读取
};

struct WEATHER { 
  int wea_code;
  int wea_code_day1;
  int wea_code_night1;
  int wea_code_day2;
  int wea_code_night2;
  int wea_code_day3;
  int wea_code_night3;

  char tem_day1_min[4] = "";
  char tem_day1_max[4] = "";
  char tem_day2_min[4] = "";
  char tem_day2_max[4] = "";
  char tem_day3_min[4] = "";
  char tem_day3_max[4] = "";
  char day1_date[11] = "";
  char day2_date[11] = "";
  char day3_date[11] = "";
  char temp_day_date[6] = "";
};

struct NIGHTCOLOR {
    uint16_t color = 0xf0b0;  // 时间
    uint16_t color2 = 0x780F; // 日期
    uint16_t color3 = 0xf000; // 农历
    uint16_t color4 = 0xfff0; // 温度
    uint16_t color5 = 0xFDA0;
    boolean isnight = true;

    void nightMode() {
      color = 0x8800; //时间
      color2 = 0x8800; //日期
      color3 = 0x8800;//农历
      color4 = 0x8800;//温度
      color5 = 0x8800;
      isnight = true;
    }

    void set_isnight(boolean val) {
      isnight = val;
    }
};

union para_value
{ // 联合体，保存int
  int val;
  byte val_b[2];
};


#define VAL(val) " " + String(val)
#define LOG_DEBUG(val) \
    do { \
        DATATIME t; \
        Serial.println(t.GetTimeStr() + String(" ") + String(val)); \
    } while (0)

#endif