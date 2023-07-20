#include "ShowDisplay.h"
#include "define.h"
#include "OLEDDriver.h"
#include "MyFont.h"
#include "Sound.h"
#include <FS.h>
#include <SPIFFS.h>



// 老虎
const char *laohugif[] = {"/hlh1.bmp", "/hlh2.bmp", "/hlh3.bmp", "/hlh4.bmp", "/hlh5.bmp", "/hlh6.bmp", "/hlh7.bmp", "/hlh8.bmp", "/hlh9.bmp",
                          "/hlh10.bmp", "/hlh11.bmp", "/hlh12.bmp", "/hlh13.bmp", "/hlh14.bmp", "/hlh15.bmp", "/hlh16.bmp", "/hlh17.bmp", "/hlh18.bmp"}; //, hlh10, hlh11, hlh12, hlh13, hlh14, hlh15, hlh16, hlh17, hlh18};
int gif_i = 0;

int yy1 = 0, yy2 = 0, yy3 = 0; // 实现钟面不同布局
int scroll_x = 0;
boolean isGeneralStar = true;

int buffer_id = 0;
int star_x[20], star_y[20];
uint16_t star_color[20];




char *right(char *dst, char *src, int n)
{
  char *p = src;
  char *q = dst;
  int len = strlen(src);
  if (n > len)
    n = len;
  p += (len - n); /*从右边第n个字符开始*/
  while (*(q++) = *(p++))
    ;
  return dst;
}
//***send http 获取天气
void show3dayWeather(WEATHER &winfo, NIGHTCOLOR &ncolor, boolean isnight)
{
  // 第一行
  showTQ(winfo.wea_code_day1, 0 + 64, 0, isnight);
  drawChars(19 + 64, 3, right(winfo.temp_day_date, winfo.day1_date, 5), ncolor.color4, isnight);
  // 温度
  if (atoi(winfo.tem_day1_min) < -9)
  {
    drawChars(19 + 64, 10, winfo.tem_day1_min, ncolor.color5, isnight);
  }
  else
  {
    drawChars(19 + 64, 10, winfo.tem_day1_min, ncolor.color5, isnight);
    drawSmBit(27 + 64, 7, wd, 3, 8, ncolor.color4, isnight);
  }
  if (atoi(winfo.tem_day1_max) < -9)
  {
    drawChars(33 + 64, 10, winfo.tem_day1_max, ncolor.color5, isnight);
  }
  else
  {
    drawChars(33 + 64, 10, winfo.tem_day1_max, ncolor.color5, isnight);
    drawSmBit(41 + 64, 7, wd, 3, 8, ncolor.color4, isnight);
  }
  showTQ(winfo.wea_code_night1, 44 + 64, 0, isnight);

  // drawLine(0+64,21, 64, sec_one); //分隔线

  // 第二行
  showTQ(winfo.wea_code_day2, 0 + 64, 22, isnight);
  drawChars(19 + 64, 3 + 22, right(winfo.temp_day_date, winfo.day2_date, 5), ncolor.color4, isnight);
  // 温度
  if (atoi(winfo.tem_day2_min) < -9)
  {
    drawChars(19 + 64, 10 + 22, winfo.tem_day2_min, ncolor.color5, isnight);
  }
  else
  {
    drawChars(19 + 64, 10 + 22, winfo.tem_day2_min, ncolor.color5, isnight);
    drawSmBit(27 + 64, 7 + 22, wd, 3, 8, ncolor.color4, isnight);
  }
  if (atoi(winfo.tem_day2_max) < -9)
  {
    drawChars(33 + 64, 10 + 22, winfo.tem_day2_max, ncolor.color5, isnight);
  }
  else
  {
    drawChars(33 + 64, 10 + 22, winfo.tem_day2_max, ncolor.color5, isnight);
    drawSmBit(41 + 64, 7 + 22, wd, 3, 8, ncolor.color4, isnight);
  }
  showTQ(winfo.wea_code_night2, 44 + 64, 22, isnight);

  // 第三行
  showTQ(winfo.wea_code_day3, 0 + 64, 22 + 21, isnight);
  drawChars(19 + 64, 3 + 22 + 21, right(winfo.temp_day_date, winfo.day3_date, 5), ncolor.color4, isnight);
  if (atoi(winfo.tem_day3_min) < -9)
  {
    drawChars(19 + 64, 10 + 22 + 21, winfo.tem_day3_min, ncolor.color5, isnight);
  }
  else
  {
    drawChars(19 + 64, 10 + 22 + 21, winfo.tem_day3_min, ncolor.color5, isnight);
    drawSmBit(27 + 64, 7 + 22 + 21, wd, 3, 8, ncolor.color4, isnight);
  }
  if (atoi(winfo.tem_day3_max) < -9)
  {
    drawChars(33 + 64, 10 + 22 + 21, winfo.tem_day3_max, ncolor.color5, isnight);
  }
  else
  {
    drawChars(33 + 64, 10 + 22 + 21, winfo.tem_day3_max, ncolor.color5, isnight);
    drawSmBit(41 + 64, 7 + 22 + 21, wd, 3, 8, ncolor.color4, isnight);
  }
  // disSmallNumbers(12,26, 16+22+21, color5);
  // disSmallNumbers(50, 47,16+22+21, color5);
  showTQ(winfo.wea_code_night3, 44 + 64, 22 + 21, isnight);

  //  drawHLine(19, 0, 64, sec_one); //左竖线
  //  drawHLine(46, 0, 64, sec_one); //左竖线
}

uint16_t read16(File f)
{
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}
uint32_t read32(File f)
{
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
// Bodmers BMP image rendering function
void drawBmp(const char *filename, int16_t x, int16_t y)
{
  Serial.println(filename);
  if ((x >= 64) || (y >= 64))
    return;

  fs::File bmpFS;

  // Open requested file on SD card
  bmpFS = SPIFFS.open(filename, "r");

  if (!bmpFS)
  {
    Serial.print("File not found");
    return;
  }

  uint32_t seekOffset;
  uint16_t w, h, row, col;
  uint8_t r, g, b;

  uint32_t startTime = millis();

  if (read16(bmpFS) == 0x4D42)
  {
    read32(bmpFS);
    read32(bmpFS);
    seekOffset = read32(bmpFS);
    read32(bmpFS);
    w = read32(bmpFS);
    h = read32(bmpFS);

    if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
    {
      y += h - 1;

      //    bool oldSwapBytes = tft.getSwapBytes();
      //    tft.setSwapBytes(true);

      bmpFS.seek(seekOffset);

      uint16_t padding = (4 - ((w * 3) & 3)) & 3;
      uint8_t lineBuffer[w * 3 + padding];

      for (row = 0; row < h; row++)
      {

        bmpFS.read(lineBuffer, sizeof(lineBuffer));
        uint8_t *bptr = lineBuffer;
        uint16_t *tptr = (uint16_t *)lineBuffer;
        // Convert 24 to 16 bit colours
        for (uint16_t col = 0; col < w; col++)
        {
          b = *bptr++;
          g = *bptr++;
          r = *bptr++;
          //   *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
          fillTab(col + 64, 63 - row, ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3), true);
        }
      }
      // tft.setSwapBytes(oldSwapBytes);
      Serial.print("Loaded in ");
      Serial.print(millis() - startTime);
      Serial.println(" ms");
    }
    else
      Serial.println("BMP format not recognized.");
  }
  bmpFS.close();
}
String formatBytes(size_t bytes)
{
  if (bytes < 1024)
  {
    return String(bytes) + "B";
  }
  else if (bytes < (1024 * 1024))
  {
    return String(bytes / 1024.0) + "KB";
  }
  else if (bytes < (1024 * 1024 * 1024))
  {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  else
  {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
// //文件格式转换
// String getContentType(String filename) {
//   if (server.hasArg("download")) return "application/octet-stream";
//   else if (filename.endsWith(".htm")) return "text/html";
//   else if (filename.endsWith(".html")) return "text/html";
//   else if (filename.endsWith(".css")) return "text/css";
//   else if (filename.endsWith(".js")) return "application/javascript";
//   else if (filename.endsWith(".png")) return "image/png";
//   else if (filename.endsWith(".gif")) return "image/gif";
//   else if (filename.endsWith(".jpg")) return "image/jpeg";
//   else if (filename.endsWith(".ico")) return "image/x-icon";
//   else if (filename.endsWith(".xml")) return "text/xml";
//   else if (filename.endsWith(".pdf")) return "application/x-pdf";
//   else if (filename.endsWith(".zip")) return "application/x-zip";
//   else if (filename.endsWith(".gz")) return "application/x-gzip";
//   return "text/plain";
// }
// //读取spiffs中的文件
// bool handleFileRead(String path) {
//   Serial.println("handleFileRead: " + path);
//   if (path.endsWith("/")) path += "index.htm";
//   String contentType = getContentType(path);
//   String pathWithGz = path + ".gz";
//   if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
//     if (SPIFFS.exists(pathWithGz))
//       path += ".gz";
//     File file = SPIFFS.open(path, "r");
//     size_t sent = server.streamFile(file, contentType);
//     file.close();
//     return true;
//   }
//   return false;
// }

void showlaohu()
{
  if (SPIFFS.exists(laohugif[gif_i]))
    drawBmp(laohugif[gif_i], 0, 0);
  if (gif_i < 17)
  {
    gif_i++;
  }
  else
  {
    gif_i = 0;
  }
}

const char *showWeek(NIGHTCOLOR *ncolor, int v_week)
{ // 显示日期
  //  uint32_t color2 = TFT_BLACK;
  switch (v_week)
  {
  case 1:
    ncolor->color = 0xf800;      // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 53;
    yy2 = -14;
    yy3 = -14;
    return "一";
  case 2:
    ncolor->color = 0xfb20;      // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 2;
    yy2 = 0;
    yy3 = 0;
    return "二";
  case 3:
    ncolor->color = 0xffec;      // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 52;
    yy2 = -42;
    yy3 = 8;
    return "三";
  case 4:
    ncolor->color = 0x7e0;       // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 52;
    yy2 = -14;
    yy3 = -14;
    return "四";
  case 5:
    ncolor->color = 0x7ec;       // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 2;
    yy2 = 0;
    yy3 = 0;
    return "五";
  case 6:
    ncolor->color = TFT_SKYBLUE; // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 52;
    yy2 = -42;
    yy3 = 8;
    return "六";
  case 15:
    ncolor->color = 0xf81f;      // 时间
    ncolor->color2 = TFT_ORANGE; // 日期
    ncolor->color3 = 0xf000;     // 农历
    ncolor->color4 = 0xfff0;     // 温度
    ncolor->color5 = TFT_ORANGE;
    yy1 = 52;
    yy2 = -14;
    yy3 = -14;
    return "日";
  }
  return "showWeek";
}


// void housound(void *parameter)
// {
//   common_play();
//   vTaskDelete(NULL);
// }


void showJieri(void *parameter, NIGHTCOLOR &ncolor, DATATIME &t, DATACLOCK &dclock, boolean isnight)
{
  if (t.sec % 20 < 15)
  {
    drawHanziS(0 + scroll_x, -1 + yy1, dclock.jieri.c_str(), ncolor.color3, isnight);
    if (scroll_x > -60 && dclock.jieri.length() > 15)
    {
      scroll_x -= 4;
    }
    else
    {
      scroll_x = 0;
    }
    //  color3=1;
  }
  else
  {
    scroll_x = 0;
    displayNumbers2(t.year1, 21, -2 + yy1, ncolor.color2, isnight);
    // dma_display->setCursor(27, 3 + yy1);
    // dma_display->setTextSize(1);
    // dma_display->print(year1);
    // dma_display->print(".");
    drawBit(28, yy1 - 2, dot, 7, 14, ncolor.color2, isnight);
    if (t.month1 < 10)
    {
      displayNumbers2(0, 32, -2 + yy1, ncolor.color2, isnight);
    }
    displayNumbers2(t.month1, 39, -2 + yy1, ncolor.color2, isnight);
    // dma_display->setCursor(45, 3 + yy1);
    // dma_display->print(".");
    drawBit(46, yy1 - 2, dot, 7, 14, ncolor.color2, isnight);
    if (t.day1 < 10)
    {
      displayNumbers2(0, 49, -2 + yy1, ncolor.color2, isnight);
    }
    displayNumbers2(t.day1, 56, -2 + yy1, ncolor.color2, isnight);
  }
  vTaskDelete(NULL);
}

void housound(void * parameter) {
  common_play();
  vTaskDelete(NULL);
}

void showTime(DATATIME &t, CONF &conf, DATACLOCK &cinfo, WEATHER &winfo, NIGHTCOLOR &ncolor, boolean isnight)
{

  const char *v_week = showWeek(&ncolor, t.week);
  if (t.minu == 0 && t.sec == 0 && t.hou < 21 && t.hou > 7)
  {
    xTaskCreate(
        housound,   /* Task function. */
        "housound", /* String with name of task. */
        10000,      /* Stack size in bytes. */
        NULL,       /* Parameter passed as input of the task */
        1,          /* Priority of the task. */
        NULL);      /* Task handle. */
  }
  
  // 显示日期
  //  displayNumbers2(strlen(jieri),0,0+yy1,color2);
  // drawHanziS(0, 0 + yy1, strlen(jieri), color2);
  // 显示节日和日期
  if (cinfo.jieri.length() < 1)
  {
    setTextColor(ncolor.color2);
    displayNumbers2(t.year1, 21, -2 + yy1, ncolor.color2, isnight);
    // dma_display->setCursor(27, 3 + yy1);
    // dma_display->setTextSize(1);
    // dma_display->print(year1);
    //  dma_display->print(".");
    // drawBit(29, 3 + yy1, dot, 7, 14, color2);
    drawBit(28, yy1 - 2, dot, 7, 14, ncolor.color2, isnight);
    if (t.month1 < 10)
    {
      displayNumbers2(0, 32, -2 + yy1, ncolor.color2, isnight);
    }
    displayNumbers2(t.month1, 39, -2 + yy1, ncolor.color2, isnight);
    // dma_display->print(month1);
    // dma_display->setCursor(45, 3 + yy1);
    // dma_display->print(".");
    drawBit(46, yy1 - 2, dot, 7, 14, ncolor.color2, isnight);
    if (t.day1 < 10)
    {
      displayNumbers2(0, 49, -2 + yy1, ncolor.color2, isnight);
    }
    displayNumbers2(t.day1, 56, -2 + yy1, ncolor.color2, isnight);
    // 显示节日
  }
  else
  {

    // xTaskCreate(
    //     showJieri,   /* Task function. */
    //     "showJieri", /* String with name of task. */
    //     10000,       /* Stack size in bytes. */
    //     NULL,        /* Parameter passed as input of the task */
    //     1,           /* Priority of the task. */
    //     NULL);       /* Task handle. */
  }



  // dma_display->print(day1);
  if (t.sec % 10 < 3)
  { 
    drawHanziS(20, 29 + yy3, cinfo.china_month.c_str(), ncolor.color3, isnight);
    drawHanziS(41, 29 + yy3, cinfo.china_day.c_str(), ncolor.color3, isnight);
  }
  else
  {
    if (strlen(cinfo.jieqi.c_str()) < 1)
    {
      drawHanziS(22, 29 + yy3, "星期", ncolor.color3, isnight);
      drawHanziS(46, 29 + yy3, v_week, ncolor.color3, isnight);
    }
    else
    {
      if (t.sec % 10 >= 3 && t.sec % 10 < 7)
      {
        drawHanziS(22, 29 + yy3, "星期", ncolor.color3, isnight);
        drawHanziS(46, 29 + yy3, v_week, ncolor.color3, isnight);
      }
      else
      {
        drawHanziS(22, 29 + yy3, cinfo.jieqi.c_str(), ncolor.color3, isnight);
      }
    }
  }


  /*显示时间*/
  if (t.hou < 10)
  {
    displayNumbers(0, 0, 41 + yy2, ncolor.color, isnight);
    displayNumbers(t.hou, 12, 41 + yy2, ncolor.color, isnight);
  }
  else
  {
    displayNumbers(t.hou, 12, 41 + yy2, ncolor.color, isnight);
  }
  if (t.minu < 10)
  {
    displayNumbers(0, 36, 41 + yy2, ncolor.color, isnight);
    displayNumbers(t.minu, 48, 41 + yy2, ncolor.color, isnight);
  }
  else
  {
    displayNumbers(t.minu, 48, 41 + yy2, ncolor.color, isnight);
  }
  disSmallNumbers(t.sec_ten, 28, 47 + yy2, ncolor.color, isnight);
  disSmallNumbers(t.sec_one, 28, 54 + yy2, ncolor.color, isnight);
  if (true)
  {
    drawLine(0, 0, 64, t.sec_one, isnight);        // 上线
    drawLine(0, 42 + yy2, 64, t.sec_one, isnight); // 分隔线
    drawHLine(0, 0, 64, t.sec_one, isnight);       // 左竖线
    drawLine(0, 63 + yy2, 64, t.sec_one, isnight); // 分隔线
    drawHLine(63, 0, 64, t.sec_one, isnight);      // 右竖线
    drawLine(0, 63, 64, t.sec_one, isnight);       // 下线
  }

  // 十分钟刷新天气
  showTQ(winfo.wea_code, 0, 17 + yy3, isnight);
  // 室外
  drawColorBit(22, 17 + yy3, tianqiwd, 5, 10, isnight);
  drawChars(28, 22 + yy3, cinfo.wea_temp1, ncolor.color4, isnight);

  //LOG_DEBUG("节日长度：" + String(cinfo.jieri.length() + String(" temp:") + String(cinfo.wea_temp1)));
  //Serial.println("temp:" + String(cinfo.wea_temp1));
  // 显示摄氏度
  if (atoi(cinfo.wea_temp1) > -10)
    drawSmBit(36, 19 + yy3, wd, 3, 8, ncolor.color4, isnight);
  // 室内温度
  disSmallNumbers(cinfo.temperature + conf.temp_mod, 32, 16 + yy3, ncolor.color5, isnight);
  drawSmBit(36, 13 + yy3, wd, 3, 8, ncolor.color5, isnight);
  drawColorBit(42, 17 + yy3, tianqisd, 5, 10, isnight);
  disSmallNumbers(cinfo.wea_hm, 52, 22 + yy3, ncolor.color4, isnight);
  // 显示H
  drawSmBit(56, 19 + yy3, sd, 3, 8, ncolor.color4, isnight);
  // 室内湿度
  disSmallNumbers(cinfo.humidity + conf.hum_mod, 52, 16 + yy3, ncolor.color5, isnight);
  // 显示H
  drawSmBit(56, 13 + yy3, sd, 3, 8, ncolor.color5, isnight);
}

void showTigger(DATATIME &t, CONF &conf, DATACLOCK &cinfo, boolean isnight)
{
  if (t.sec_ten % 2 == 0 && !isnight)
  {
    // 显示老虎
    showlaohu();
    if (true)
    {
      drawBit(65, 52, laba, 12, 12, TFT_GREEN, isnight);
    }
    else
    {
      drawBit(65, 52, laba, 12, 12, TFT_DARKGREY, isnight);
    }
    // 显示wifi图标
    drawBit(116, 52, iconwifi, 12, 12, TFT_GREEN, isnight);
    // 显示光点
    if (t.sec % 2 == 0 && isGeneralStar)
    {
      for (int i = 0; i < conf.starnum; i++)
      {
        star_x[i] = 64 + rand() % 63;
        star_y[i] = rand() % 63;
        star_color[i] = rand() % 0xffff;
      }
      isGeneralStar = !isGeneralStar;
    }
    if (t.sec % 2 != 0 && isGeneralStar == false)
    {
      isGeneralStar = true;
    }
    for (int i = 0; i < conf.starnum; i++)
    {
      // dma_display->drawPixel(star_x[i], star_y[i], star_color[i]);
      fillTab(star_x[i], star_y[i], star_color[i], isnight);
      fillTab(star_x[i] + 1, star_y[i], star_color[i], isnight);
      fillTab(star_x[i] - 1, star_y[i], star_color[i], isnight);
      //  fillTab(star_x[i]+2,star_y[i],star_color[i]);
      //  fillTab(star_x[i]-2,star_y[i],star_color[i]);
      fillTab(star_x[i], star_y[i] + 1, star_color[i], isnight);
      fillTab(star_x[i], star_y[i] - 1, star_color[i], isnight);
      //  fillTab(star_x[i],star_y[i]+2,star_color[i]);
      //  fillTab(star_x[i],star_y[i]-2,star_color[i]);
    }
  }
  else
  {
    if (gif_i > 17)
    {
      gif_i = 0;
    }
    //show3dayWeather(isnight);
  }
}

void onlyShowTime(DATATIME &t, CONF &conf, NIGHTCOLOR &ncolor, boolean isnight)
{
  /*显示时间*/
  t.GetTime();
  if (t.hou < 10)
  {
    dis30Numbers(0, -2, 2, ncolor.color3, isnight);
    dis30Numbers(t.hou, 28, 2, ncolor.color3, isnight);
  }
  else
  {
    // displayNumbers(hou, 12, 40 + yy2, color);
    dis30Numbers(t.hou, 28, 2, ncolor.color3, isnight);
  }
  drawBit(48, -1, sz30_maohao, 32, 63, ncolor.color3, isnight);
  if (t.minu < 10)
  {
    dis30Numbers(0, 70, 2, ncolor.color3, isnight);
    dis30Numbers(t.minu, 100, 2, ncolor.color3, isnight);
  }
  else
  {
    // displayNumbers(minu, 48, 40 + yy2, color);
    dis30Numbers(t.minu, 100, 2, ncolor.color3, isnight);
  }
  // 产生光点

  // 显示光点
  if (t.sec % 2 == 0 && isGeneralStar)
  {
    for (int i = 0; i < conf.starnum; i++)
    {
      star_x[i] = rand() % 127;
      star_y[i] = rand() % 63;
      star_color[i] = rand() % 0xffff;
    }
    isGeneralStar = !isGeneralStar;
  }
  if (t.sec % 2 != 0 && isGeneralStar == false)
  {
    isGeneralStar = true;
  }
  for (int i = 0; i < conf.starnum; i++)
  {
    // dma_display->drawPixel(star_x[i], star_y[i], star_color[i]);
    fillTab(star_x[i] + 1, star_y[i], star_color[i], isnight);
    fillTab(star_x[i], star_y[i], star_color[i], isnight);
    fillTab(star_x[i], star_y[i] + 1, star_color[i], isnight);
    fillTab(star_x[i] + 1, star_y[i] + 1, star_color[i], isnight);
  }
}
void onlyShowTime2(DATATIME &t, CONF &conf, NIGHTCOLOR &ncolor, boolean isnight)
{
  /*显示时间*/
  t.GetTime();
  if (t.hou < 10)
  {
    displayNumbers2(0, 8 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
    displayNumbers2(t.hou, 16 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
  }
  else
  {
    // displayNumbers(hou, 12, 40 + yy2, color);
    displayNumbers2(t.hou, 16 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
  }
  drawBit(24 + get_screen_num(), 10 + yy1, maohao, 7, 14, ncolor.color3, isnight);
  // drawHLine(32,40+yy2,30,color3);
  if (t.minu < 10)
  {
    displayNumbers2(0, 29 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
    displayNumbers2(t.minu, 37 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
  }
  else
  {
    // displayNumbers(minu, 48, 40 + yy2, color);
    displayNumbers2(t.minu, 37 + get_screen_num(), 10 + yy1, ncolor.color3, isnight);
  }
}
