
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//

#include "OLEDDriver.h"
#include "define.h"
#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "MyFont.h"
#include "Arduino_GB2312_library_24.h"
#include "Arduino_GB2312_library_32.h"
#ifndef NO_GFX
 #include <Fonts/FreeSans9pt7b.h>
 #include <Fonts/FreeSans12pt7b.h>
 #include <Fonts/FreeSans18pt7b.h>
 #include <Fonts/FreeSans24pt7b.h>
#endif

const GFXfont *font = &FreeSans24pt7b;

#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define  B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5 
#define D_PIN 17
#define E_PIN -1 // required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

// MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;
// 定义显示数组
uint16_t ledtab[128][64];
uint16_t ledtab_old[64][64];

int screen_num = 0;       // 显示控制，让元素在两个屏幕上跳动
uint16_t colorl = 0xff00; // 滚动条

#define TFT_SILVER 0xC618 /* 192, 192, 192 */

char s12[] = "12";
char s24[] = "24";
PROGMEM const FNT_SJ24 shuzi24[]{
    {1, sz_1, s12}, {2, sz_2, s12}, {3, sz_3, s12}, {4, sz_4, s12}, {5, sz_5, s12}, {6, sz_6, s12}, {7, sz_7, s12}, {8, sz_8, s12}, {9, sz_9, s12}, {0, sz_0, s24}};
char s30[] = "30";
PROGMEM const FNT_SZ30 shuzi30[]{
    {1, sz30_1, s30}, {2, sz30_2, s30}, {3, sz30_3, s30}, {4, sz30_4, s30}, {5, sz30_5, s30}, {6, sz30_6, s30}, {7, sz30_7, s30}, {8, sz30_8, s30}, {9, sz30_9, s30}, {0, sz30_0, s30}};
char s7[] = "7";
PROGMEM const FNT_SJ14 shuzi14[]{
    {1, shuzi_1, s7}, {2, shuzi_2, s7}, {3, shuzi_3, s7}, {4, shuzi_4, s7}, {5, shuzi_5, s7}, {6, shuzi_6, s7}, {7, shuzi_7, s7}, {8, shuzi_8, s7}, {9, shuzi_9, s7}, {0, shuzi_0, s7}};
char s20[] = "20";
PROGMEM const FNT_TQ20 tq20[] // 61
    {
        {100, tianqi100, s20}, {101, tianqi101, s20}, {102, tianqi102, s20}, {103, tianqi103, s20}, {104, tianqi104, s20}, {150, tianqi150, s20}, {153, tianqi153, s20}, {154, tianqi154, s20}, {300, tianqi300, s20}, {301, tianqi301, s20}, {302, tianqi302, s20}, {303, tianqi303, s20}, {304, tianqi304, s20}, {305, tianqi305, s20}, {306, tianqi306, s20}, {307, tianqi307, s20}, {308, tianqi308, s20}, {309, tianqi309, s20}, {310, tianqi310, s20}, {311, tianqi311, s20}, {312, tianqi312, s20}, {313, tianqi313, s20}, {314, tianqi314, s20}, {315, tianqi315, s20}, {316, tianqi316, s20}, {317, tianqi317, s20}, {318, tianqi318, s20}, {350, tianqi350, s20}, {351, tianqi351, s20}, {399, tianqi399, s20}, {400, tianqi400, s20}, {401, tianqi401, s20}, {402, tianqi402, s20}, {403, tianqi403, s20}, {404, tianqi404, s20}, {405, tianqi405, s20}, {406, tianqi406, s20}, {407, tianqi407, s20}, {408, tianqi408, s20}, {409, tianqi409, s20}, {410, tianqi410, s20}, {456, tianqi456, s20}, {457, tianqi457, s20}, {499, tianqi499, s20}, {500, tianqi500, s20}, {501, tianqi501, s20}, {502, tianqi502, s20}, {503, tianqi503, s20}, {504, tianqi504, s20}, {507, tianqi507, s20}, {508, tianqi508, s20}, {509, tianqi509, s20}, {510, tianqi510, s20}, {511, tianqi511, s20}, {512, tianqi512, s20}, {513, tianqi513, s20}, {514, tianqi514, s20}, {515, tianqi515, s20}, {900, tianqi900, s20}, {901, tianqi901, s20}, {999, tianqi999, s20}};
char s3[] = "3";
PROGMEM const FNT_SMSZ smsz[]{
    {1, smsj_1, s3}, {2, smsj_2, s3}, {3, smsj_3, s3}, {4, smsj_4, s3}, {5, smsj_5, s3}, {6, smsj_6, s3}, {7, smsj_7, s3}, {8, smsj_8, s3}, {9, smsj_9, s3}, {0, smsj_0, s3}};
PROGMEM const FNT_SMCHAR smchar[]{
    {'-', smsj_jian, s3}, {'1', smsj_1, s3}, {'2', smsj_2, s3}, {'3', smsj_3, s3}, {'4', smsj_4, s3}, {'5', smsj_5, s3}, {'6', smsj_6, s3}, {'7', smsj_7, s3}, {'8', smsj_8, s3}, {'9', smsj_9, s3}, {'0', smsj_0, s3}};

uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);


MatrixPanel_I2S_DMA *get_dma_display()
{
  return dma_display;
}

int mx_width() {
  return dma_display->getCfg().mx_width * dma_display->getCfg().chain_length;
}

int mx_height() {
  return dma_display->getCfg().mx_height;
}

void setTextColor(uint16_t c)
{
  dma_display->setTextColor(c);
}

void set_screen_num(int val)
{
  screen_num = val;
}

int get_screen_num()
{
  return screen_num;
}

void cleanTab()
{
  for (int i = 0; i < 128; i++)
  {
    for (int j = 0; j < 64; j++)
    {
      ledtab[i][j] = 0x0000;
    }
  }
}
void fillTab(int x, int y, uint16_t color, boolean isnight)
{
  if (!isnight)
  {
    ledtab[x][y] = color;
  }
  else
  {
    ledtab[x][y] = 0x8800;
  }
}


void draw_gb2312_24(int xx, int yy, unsigned char *names, uint32_t color, boolean isnight)
{
  LOG_DEBUG("draw_gb2312_24:" + String(sizeof(names)));

  unsigned char buffs[24];
  // 建立缓存空间
  getfont24(names, sizeof(names), buffs);
  int kj = 0;
  int x = 0;
  int y = 0;
  for (int i = 0; i < 24; i++)
  {
    if ((i) % 2 == 0)
    {
      Serial.print(String(i)); 
    }
    
    for (int s = 7; s >= 0; s--)
    {
      //fillTab(7 - s + xx, y + yy, 0, false);
      if (buffs[i] & (0x01 << s))
      {
        if (7 - s + xx >= 0)
        {
          if (i % 2 == 0)
          {
            fillTab(7 - s + xx, y + yy, color, isnight);
          }
          else
          {
            fillTab(15 - s + xx, y + yy, color, isnight);
          }
          Serial.print("*");
        } else {
          Serial.println("warnning:" + String(s) + " " + " " + String(x) + " " + String(xx));
        }
      } else {
        if (7 - s + xx >= 0)
        {
          if (i % 2 == 0)
          {
            fillTab(7 - s + xx, y + yy, 0, isnight);
          }
          else
          {
            fillTab(15 - s + xx, y + yy, 0, isnight);
          }
          Serial.print("."); 
        } else {
          Serial.println("warnning:" + String(s) + " " + " " + String(x) + " " + String(xx));
        }
      }
    }

    if ((i + 1) % 2 == 0)
    {
      y += 1;
      Serial.println(""); 
    }
  }
}

void draw_gb2312_32(int xx, int yy, unsigned char *names, uint32_t color, boolean isnight)
{

  unsigned char buffs[32];
  // 建立缓存空间
  getfont32(names, sizeof(names), buffs);
  int kj = 0;
  int x = 0;
  int y = 0;
  for (int i = 0; i < 32; i++)
  {
    if ((i) % 2 == 0)
    {
      Serial.print(String(i)); 
    }
    
    for (int s = 7; s >= 0; s--)
    {
      //fillTab(7 - s + xx, y + yy, 0, false);
      if (buffs[i] & (0x01 << s))
      {
        if (7 - s + xx >= 0)
        {
          if (i % 2 == 0)
          {
            fillTab(7 - s + xx, y + yy, color, isnight);
          }
          else
          {
            fillTab(15 - s + xx, y + yy, color, isnight);
          }
          Serial.print("*");
        } else {
          Serial.println("warnning:" + String(s) + " " + " " + String(x) + " " + String(xx));
        }
      } else {
        if (7 - s + xx >= 0)
        {
          if (i % 2 == 0)
          {
            fillTab(7 - s + xx, y + yy, 0, isnight);
          }
          else
          {
            fillTab(15 - s + xx, y + yy, 0, isnight);
          }
          Serial.print("."); 
        } else {
          Serial.println("warnning:" + String(s) + " " + " " + String(x) + " " + String(xx));
        }
      }
    }

    if ((i + 1) % 2 == 0)
    {
      y += 1;
      Serial.println(""); 
    }
  }
}


int drawHanziS(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight)
{
  int x0 = x;
  unsigned char b[3];
  for (int i = 0; i < strlen(str); i = i + 3)
  {
    Serial.println(String(strlen(str)) + " " + String(i) + " " + String(str[i])); 
    b[0] = str[i];
    if (i + 1 < strlen(str)) {
      b[1] = str[i + 1];
    } else {
      b[1] = 0;
    }
    if (i + 2 < strlen(str)) {
      b[2] = str[i + 2];
    } else {
      b[2] = 0;
    }
    draw_gb2312_32(x0, y, b, color, isnight);
    x0 += (16 - 4); // 减小字间距，字体实际宽12
  }
  return x0;
}

void fillCircle(int x, int y, int r, int color, boolean isnight)
{
  for (int i = x - r; i < x + r; i++)
  {
    for (int j = y - r; j < y + r; j++)
    {
      if (((i - x) * (i - x) + (j - y) * (j - y)) <= (r * r))
      {
        fillTab(i, j, color, isnight);
      }
    }
  }
}
void drawBit(int x, int y, const uint8_t *bitmap, int width, int height, uint16_t color, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))
      {
        // dma_display->drawPixel(x + i, y + j, color);
        fillTab(x + i, y + j, color, isnight);
      }
    }
  }
}
void drawLine(int x0, int y0, int x1, int sec, boolean isnight)
{
  for (int i = x0; i < x1; i++)
  { // 横向
    colorl = rand() % 0xffff;
    if (!isnight)
    {
      // dma_display->drawPixel(x0 + i, y0, colorl);
      fillTab(x0 + i, y0, colorl, isnight);
    }
  }
}
void drawHLine(int x0, int y0, int y1, int sec, boolean isnight)
{
  for (int i = 0; i < y1; i++)
  { // 横向
    colorl = rand() % 0xffff;
    if (!isnight)
    {
      // dma_display->drawPixel(x0, y0+i, colorl);
      fillTab(x0, y0 + i, colorl, isnight);
    }
  }
}
void drawBit2(int x, int y, const uint8_t *bitmap, int width, int height, uint16_t color, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;
  int first = 0;
  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))
      {
        if (first == 0 & !isnight)
        {
          // dma_display->drawPixel(x + i, y + j, color+TFT_SILVER);
          fillTab(x + i, y + j, color + TFT_SILVER, isnight);
        }
        else
        {
          // dma_display->drawPixel(x + i, y + j, color);
          fillTab(x + i, y + j, color, isnight);
        }
        first++;
      }
    }
    first = 0;
  }
}
void drawSmBit(int x, int y, const uint8_t *bitmap, int width, int height, uint16_t color, boolean isnight)
{
  int32_t i, j;
  unsigned char a[3];
  unsigned char temp;
  for (j = 0; j < height; j++)
  {
    temp = bitmap[j];
    for (i = width; i > 0; i--)
    {
      a[i - 1] = temp & 0x01;
      temp >>= 1;
      if (a[i - 1] == 1)
      {
        // dma_display->drawPixel(x + i, y + j, color);
        fillTab(x + i, y + j, color, isnight);
      }
    }
  }
}
void display30Number(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++)
  {
    if (shuzi30[k].Index == c)
    {
      drawBit(x, y, shuzi30[k].sz30_Id, 15, 30, color, isnight);
    }
  }
}
void display30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0)
  {
    display30Number(numbers, x, y, color, isnight);
  }
  while (numbers)
  {
    temp = numbers % 10;
    if (numbers < 10)
    {
      temp = numbers;
    }

    display30Number(temp, x - (count * 16), y, color, isnight);
    numbers = numbers / 10;
    count++; // count表示val是一个几位数
  }
}
void drawColorBit3(int x, int y, const uint16_t *bitmap, int width, int height)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      if (bitmap[i + j * width] != 0)
      {
        dma_display->drawPixel(x + i, y + j, bitmap[i + j * width]);
      }
    }
  }
}
void drawColorBit(int x, int y, const uint16_t *bitmap, int width, int height, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      if (bitmap[i + j * width] != 0)
      {
        fillTab(x + i, y + j, bitmap[i + j * width], isnight);
      }
    }
  }
}
void drawColorBit2(int x, int y, const uint16_t *bitmap, int width, int height, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++)
  {
    for (i = 0; i < width; i++)
    {
      if (bitmap[i + j * width] != 0)
      {
        fillTab(x + i, y + j, bitmap[i + j * width], isnight);
      }
    }
  }
}
void showTQ(int c, int x, int y, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 61; k++)
  {
    if (tq20[k].Index == c)
    {
      drawColorBit2(x, y, tq20[k].tq20_Id, 20, 20, isnight);
    }
  }
}

bool hasChineseCharacter(const String& str) {
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char ch = static_cast<unsigned char>(str[i]);
        if (ch >= 0x80 && ch <= 0xFF) {
            // 如果字符的高位为1，则表示是中文字符
            return true;
        }
    }
    return false;
}

int hexToDecimal(String hexValue) {
  int decimalValue;
  sscanf(hexValue.c_str(), "%x", &decimalValue);
  return decimalValue;
}

uint16_t color_to_color565(const char *color) {

  String hexValue(color); 
  int red = hexToDecimal(hexValue.substring(0, 2));
  int green = hexToDecimal(hexValue.substring(2, 4));
  int blue = hexToDecimal(hexValue.substring(4, 6));
  //LOG_DEBUG(String(color) + " " + String(red) + " " + String(green) + " " + String(blue));
  return dma_display->color565(red, green, blue);
}


void fillScreenTab(int32_t x, int32_t y, int32_t width, int height)
{
  //LOG_DEBUG("fillScreenTab:" + String(x) + " " + String(y) + " " + String(x_max));
  for (int i = x; i < width; i++)
  {
    for (int j = y; j < height; j++)
    {
        dma_display->drawPixel(i + screen_num, j, ledtab[i][j]);
    }
  }
}

int draw_gb2312(int xx, int yy, unsigned char *names, uint32_t color, int fsize, int &width, int &height)
{
  int x_fill = 0; // x第二个8点数量
  int space = 1; 
  if (fsize == 1) {
    space = 2;
    width = 12;
    height = 12;
    fsize = 24;
    x_fill = 4;
  } else {
    width = 16;
    height = 16;
    fsize = 32;
    x_fill = 0;
  }

   if (xx + width >= mx_width()) {
    return space;
  }
  if (yy + height >= mx_height()) {
    return space;
  } 

  unsigned char buffs[fsize];
  // 建立缓存空间
  if (fsize == 24) {
    getfont24(names, sizeof(names), buffs);
  } else {
    getfont32(names, sizeof(names), buffs);
  }
  int kj = 0;
  int x = 0;
  int y = 0;
  for (int i = 0; i < fsize; i += 2)
  {
    Serial.print(String(i) + ":"); 

    for (int s = 7; s >= 0; s--) {
      if (buffs[i] & (0x01 << s))
      {
        fillTab(7 - s + xx, y + yy, color);
        Serial.print("*");
      } else {
        fillTab(7 - s + xx, y + yy, 0);
        Serial.print("."); 
      }
    }
    for (int s = 7; s >= x_fill; s--) {
      if (buffs[i+1] & (0x01 << s))
      {
        fillTab(15 - s + xx, y + yy, color);
        Serial.print("*");
      } else {
        fillTab(15 - s + xx, y + yy, 0);
        Serial.print("."); 
      }
    }
    y += 1;
    Serial.println(""); 
  }

  fillScreenTab(xx, yy, xx + width, yy + height);
  return space;

}

int draw_ascii(String words, int x, int y, uint16_t color565, int fsize, int &width, int &height)
{
  int space = 1; 
  // 1 is default 6x8, 2 is 12x16, 3 is 18x24, etc
  width = 6;
  height = 8;
  if (fsize == 2) {
      width = 12;
      height = 16;
  } else if (fsize == 3) {
      width = 18;
      height = 24;
  }

  if (x + width >= mx_width()) {
    return space;
  }
  if (y + height >= mx_height()) {
    return space;
  } 

  dma_display->setCursor(x, y);
  // 设置背景色 覆盖已有内容
  if (color565 == 0) {
    color565 = dma_display->color565(255, 255, 0);
  }
  dma_display->setFont(font);
  dma_display->setTextSize(fsize);
  dma_display->setTextColor(color565, 0);
  dma_display->print(words);


  return space;
}

void text(const String &content, bool clear, int x, int y, const char *color, int fsize)
{
  /*

  */
  if (dma_display == nullptr) {
    return;
  }

  if (clear)
  {
    dma_display->clearScreen();
  }

  if (x == -1)
  {
    x = 0;//int(dma_display->getCfg().mx_width * dma_display->getCfg().chain_length / 2);
  }
  if (y == -1)
  {
    y = int(mx_height() / 2) - 8;
  }

  fsize = fsize > 3 ? 3 : fsize;

  // 默认黄色 #7D7DE6 255, 255, 0
  uint16_t color565 = 0;
  if (color == NULL) {
    color565 =  dma_display->color565(255, 255, 0);
  } else {
    color565 = color_to_color565(color);
  }

  int space = 0;
  int x_cursor = x;
  unsigned char b[3];
  const char *str = content.c_str();
  for (int i = 0; i < strlen(str);)
  {
    int width = 0, height = 0;
    unsigned char ch = static_cast<unsigned char>(str[i]);
    if (ch >= 0x80 && ch <= 0xFF) {
        // 如果字符的高位为1，则表示是中文字符
        b[0] = str[i];
        b[1] = str[i + 1];
        b[2] = str[i + 2];
        space = draw_gb2312(x_cursor, y, b, color565, fsize, width, height);
        i += 3;
    } else {
        space = draw_ascii(String(str[i]), x_cursor, y, color565, fsize, width, height);
        i++;
    }

    // 清空间距内容
    if (i + 1 < strlen(str)) {
      dma_display->fillRect(x_cursor + width, y, space, height, 0); // dma_display->color565(125, 125, 230)
      //LOG_DEBUG(VAL(x_cursor + width + 1) + VAL(x_cursor + width + space) + VAL(space) + VAL(height));
    }

    //LOG_DEBUG(" x_cursor:" + String(x_cursor) + " "  + String(s_x) + " "  + String(s_y) + " "  + String(space));

    // 减小字间距，字体实际宽12
    x_cursor += width + space;

    // 换行逻辑
    if (x_cursor + width >= mx_width()) {
        x_cursor = x;
        y = y + height;
    }
    if (y + height >= mx_height()) {
      break;
    }
  }
  dma_display->flipDMABuffer();

}

void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, const char *color) {
  dma_display->fillRect(x, y, w, h, color_to_color565(color));
  dma_display->flipDMABuffer();
}

MatrixPanel_I2S_DMA *get_oled() {
  return dma_display;
}

void clearOLED()
{
  dma_display->clearScreen();
}

void initOLED(int panel_chain, int light)
{

  // esp32
  //HUB75_I2S_CFG::i2s_pins _pins = {R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, 32, LAT_PIN, OE_PIN, CLK_PIN};
 
  // esp32 wrover kit
  //   ESP32管脚  JTAG信号
  // 1 EN          TRST_N
  // 2 MTMS/GPIO14 TMS
  // 3 MTDO/GPIO15 TDO
  // 4 MTDI/GPIO12 TDI
  // 5 MTCK/GPIO13 TCK
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, 12, 13, A_PIN, B_PIN, C_PIN, 18, 2, LAT_PIN, OE_PIN, 22};
 
  // Module configuration
  HUB75_I2S_CFG mxconfig(
      PANEL_RES_X, // module width
      PANEL_RES_Y, // module height
      panel_chain,  // Chain length
      _pins
  );

  mxconfig.double_buff = true;

  //mxconfig.gpio.e = 2;  
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup 会影响jtag调试 Bus_Parallel16::init中以下逻辑
  // for(int i = 0; i < bus_width; i++) 
  // _gpio_pin_init(pins[i]);
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(50); // 0-255
  dma_display->clearScreen();
  dma_display->fillScreen(myBLACK);
}

void setBrightness(int dianya, int light)
{
  dma_display->setBrightness8(light * dianya); // 0-255
}
void displayNumber(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++)
  {
    if (shuzi24[k].Index == c)
    {
      drawBit2(x, y, shuzi24[k].hz24_Id, 12, 24, color, isnight);
    }
  }
}
void disSmallNumber(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++)
  {
    if (smsz[k].Index == c)
    {
      //  Serial.println("run here");
      drawSmBit(x, y, smsz[k].smsz_Id, 3, 5, color, isnight);
    }
  }
}
void disSmallNumbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0)
  {
    disSmallNumber(numbers, x, y, color, isnight);
  }
  while (numbers)
  {
    temp = numbers % 10;
    if (numbers < 10)
    {
      temp = numbers;
    }

    disSmallNumber(temp, x - (count * 4), y, color, isnight);
    numbers = numbers / 10;
    count++; // count表示val是一个几位数
  }
}
void dis30Number(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++)
  {
    if (shuzi30[k].Index == c)
    {
      //  Serial.println("run here");
      drawBit(x, y, shuzi30[k].sz30_Id, 30, 60, color, isnight);
    }
  }
}
void dis30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0)
  {
    dis30Number(numbers, x, y, color, isnight);
  }
  while (numbers)
  {
    temp = numbers % 10;
    if (numbers < 10)
    {
      temp = numbers;
    }

    dis30Number(temp, x - (count * 30), y, color, isnight);
    numbers = numbers / 10;
    count++; // count表示val是一个几位数
  }
}
void displayNumbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0)
  {
    displayNumber(numbers, x, y, color, isnight);
  }
  while (numbers)
  {
    temp = numbers % 10;
    if (numbers < 10)
    {
      temp = numbers;
    }

    displayNumber(temp, x - (count * 12), y, color, isnight);
    numbers = numbers / 10;
    count++; // count表示val是一个几位数
  }
}
void disSmallChar(char c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 11; k++)
  {
    if (smchar[k].Index == c)
    {
      // Serial.println("run here!DrawChar");
      drawSmBit(x, y, smchar[k].smchar_Id, 3, 5, color, isnight);
    }
  }
}
void drawChars(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight)
{
  int x0 = x;
  for (int i = 0; i < strlen(str); i++)
  {
    // Serial.print("当前char:");
    // Serial.println(str[i]);
    disSmallChar(str[i], x0, y, color, isnight);
    x0 += 4;
  }
}
void displayNumber2(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++)
  {
    if (shuzi14[k].Index == c)
    {
      drawBit(x, y, shuzi14[k].hz14_Id, 8, 14, color, isnight);
    }
  }
}
void displayNumbers2(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0)
  {
    displayNumber2(numbers, x, y, color, isnight);
  }
  while (numbers)
  {
    temp = numbers % 10;
    if (numbers < 10)
    {
      temp = numbers;
    }

    displayNumber2(temp, x - (count * 7), y, color, isnight);
    numbers = numbers / 10;
    count++; // count表示val是一个几位数
  }
}
