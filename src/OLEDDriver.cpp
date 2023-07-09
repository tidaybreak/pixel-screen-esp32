
// Example sketch which shows how to display some patterns
// on a 64x32 LED matrix
//

#include "OLEDDriver.h"
#include "define.h"
#include<Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "MyFont.h"
#include "Arduino_GB2312_library.h"

//MatrixPanel_I2S_DMA dma_display;
MatrixPanel_I2S_DMA *dma_display = nullptr;
// 定义显示数组
uint16_t ledtab[128][64];
uint16_t ledtab_old[64][64];

int screen_num = 0; // 显示控制，让元素在两个屏幕上跳动
uint16_t colorl = 0xff00; // 滚动条

#define TFT_SILVER      0xC618      /* 192, 192, 192 */



struct  FNT_SJ24                 // 汉字字模数据结构
{
  int  Index;               // 汉字内码索引,存放内码,如"中",在UTF-8编码下，每个汉字占3个字节，第四个是结束符0
  const unsigned char* hz24_Id;                        // 点阵码数据       存放内码后对应的 点阵序列  每个字需要32个字节的点阵序列
  char *hz_width;
};
PROGMEM const FNT_SJ24 shuzi24[] 
{
  {1, sz_1,"12"}, {2, sz_2,"12"}, {3, sz_3,"12"}, {4, sz_4,"12"}, {5, sz_5,"12"}, {6, sz_6,"12"}, {7, sz_7,"12"}, {8, sz_8,"12"}, {9, sz_9,"12"}, {0, sz_0,"24"}
};
struct  FNT_SZ30                 // 汉字字模数据结构
{
  int  Index;               // 汉字内码索引,存放内码,如"中",在UTF-8编码下，每个汉字占3个字节，第四个是结束符0
  const unsigned char* sz30_Id;                        // 点阵码数据       存放内码后对应的 点阵序列  每个字需要32个字节的点阵序列
  char *hz_width;
};
PROGMEM const FNT_SZ30 shuzi30[] 
{
  {1, sz30_1,"30"}, {2, sz30_2,"30"}, {3, sz30_3,"30"}, {4, sz30_4,"30"}, {5, sz30_5,"30"}, {6, sz30_6,"30"}, {7, sz30_7,"30"}, {8, sz30_8,"30"}, {9, sz30_9,"30"}, {0, sz30_0,"30"}
};
struct  FNT_SJ14                 // 汉字字模数据结构
{
  int  Index;               // 汉字内码索引,存放内码,如"中",在UTF-8编码下，每个汉字占3个字节，第四个是结束符0
  const unsigned char* hz14_Id;                        // 点阵码数据       存放内码后对应的 点阵序列  每个字需要32个字节的点阵序列
  char *hz_width;
};
PROGMEM const FNT_SJ14 shuzi14[] 
{
  {1, shuzi_1,"7"}, {2, shuzi_2,"7"}, {3, shuzi_3,"7"}, {4, shuzi_4,"7"}, {5, shuzi_5,"7"}, {6, shuzi_6,"7"}, {7, shuzi_7,"7"}, {8, shuzi_8,"7"}, {9, shuzi_9,"7"}, {0, shuzi_0,"7"}
};
struct  FNT_TQ20                 // 天气
{
  int  Index;               // 天气索引
  const  uint16_t * tq20_Id; // 存放每个点的颜色
  char *hz_width;
};
PROGMEM const FNT_TQ20 tq20[] //61
{
 { 100,tianqi100,"20"},{ 101,tianqi101,"20"},{ 102,tianqi102,"20"},{ 103,tianqi103,"20"},{ 104,tianqi104,"20"},{ 150,tianqi150,"20"},{ 153,tianqi153,"20"},{ 154,tianqi154,"20"},{ 300,tianqi300,"20"},{ 301,tianqi301,"20"},{ 302,tianqi302,"20"},{ 303,tianqi303,"20"},{ 304,tianqi304,"20"},{ 305,tianqi305,"20"},{ 306,tianqi306,"20"},{ 307,tianqi307,"20"},{ 308,tianqi308,"20"},{ 309,tianqi309,"20"},{ 310,tianqi310,"20"},{ 311,tianqi311,"20"},{ 312,tianqi312,"20"},{ 313,tianqi313,"20"},{ 314,tianqi314,"20"},{ 315,tianqi315,"20"},{ 316,tianqi316,"20"},{ 317,tianqi317,"20"},{ 318,tianqi318,"20"},{ 350,tianqi350,"20"},{ 351,tianqi351,"20"},{ 399,tianqi399,"20"},{ 400,tianqi400,"20"},{ 401,tianqi401,"20"},{ 402,tianqi402,"20"},{ 403,tianqi403,"20"},{ 404,tianqi404,"20"},{ 405,tianqi405,"20"},{ 406,tianqi406,"20"},{ 407,tianqi407,"20"},{ 408,tianqi408,"20"},{ 409,tianqi409,"20"},{ 410,tianqi410,"20"},{ 456,tianqi456,"20"},{ 457,tianqi457,"20"},{ 499,tianqi499,"20"},{ 500,tianqi500,"20"},{ 501,tianqi501,"20"},{ 502,tianqi502,"20"},{ 503,tianqi503,"20"},{ 504,tianqi504,"20"},{ 507,tianqi507,"20"},{ 508,tianqi508,"20"},{ 509,tianqi509,"20"},{ 510,tianqi510,"20"},{ 511,tianqi511,"20"},{ 512,tianqi512,"20"},{ 513,tianqi513,"20"},{ 514,tianqi514,"20"},{ 515,tianqi515,"20"},{ 900,tianqi900,"20"},{ 901,tianqi901,"20"},{ 999,tianqi999,"20"}
};
struct  FNT_SMSZ                 // 汉字字模数据结构
{
  int  Index;               // 汉字内码索引,存放内码,如"中",在UTF-8编码下，每个汉字占3个字节，第四个是结束符0
  const unsigned char* smsz_Id;                        // 点阵码数据       存放内码后对应的 点阵序列  每个字需要32个字节的点阵序列
  char *hz_width;
};
PROGMEM const FNT_SMSZ smsz[] 
{
  {1, smsj_1,"3"}, {2, smsj_2,"3"}, {3, smsj_3,"3"}, {4, smsj_4,"3"}, {5, smsj_5,"3"}, {6, smsj_6,"3"}, {7, smsj_7,"3"}, {8, smsj_8,"3"}, {9, smsj_9,"3"}, {0, smsj_0,"3"}
};
struct  FNT_SMCHAR                 // 汉字字模数据结构
{
  char  Index;               // 汉字内码索引,存放内码,如"中",在UTF-8编码下，每个汉字占3个字节，第四个是结束符0
  const unsigned char* smchar_Id;                        // 点阵码数据       存放内码后对应的 点阵序列  每个字需要32个字节的点阵序列
  char *hz_width;
};
PROGMEM const FNT_SMCHAR smchar[] 
{
  {'-', smsj_jian,"3"},{'1', smsj_1,"3"}, {'2', smsj_2,"3"}, {'3', smsj_3,"3"}, {'4', smsj_4,"3"}, {'5', smsj_5,"3"}, {'6', smsj_6,"3"}, {'7', smsj_7,"3"}, {'8', smsj_8,"3"}, {'9', smsj_9,"3"}, {'0', smsj_0,"3"}
};


uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myRED = dma_display->color565(255, 0, 0);
uint16_t myGREEN = dma_display->color565(0, 255, 0);
uint16_t myBLUE = dma_display->color565(0, 0, 255);


MatrixPanel_I2S_DMA *get_dma_display() {
  return dma_display;
}

  void setTextColor(uint16_t c) {
    dma_display->setTextColor(c);
  }

void set_screen_num(int val) {
  screen_num = val;
}

int get_screen_num() {
  return screen_num;
}

void cleanTab() {
  for (int i = 0; i < 128; i++) {
    for (int j = 0; j < 64; j++) {
      ledtab[i][j] = 0x0000;
    }
  }
}
void fillTab(int x, int y, uint16_t color, boolean isnight) {
  if(!isnight){
      ledtab[x][y] = color;
  }else{
      ledtab[x][y] = 0x8800;
  }
}
void fillScreenTab(boolean twopannel, int minu, boolean isnight) {
//  for (int i = 0; i < 64; i++) {
//    ledtab[64][0 + i] = TFT_LIGHTGREY; //分隔线
//  }
  if(twopannel){
  if (minu % 2 == 0 && !isnight) {
    screen_num = 0;
  } else if(!isnight){
    screen_num = 64;
  }
  }else{
    screen_num = 0;
  }
  for (int i = 0; i < 128; i++) {
    for (int j = 0; j < 64; j++) {
      if(twopannel){
      if (i < 64) {

        dma_display->drawPixel(i + screen_num, j, ledtab[i][j]);

      } else {

        dma_display->drawPixel(i - screen_num, j, ledtab[i][j]);

      }
      }else{
        dma_display->drawPixel(i + screen_num, j, ledtab[i][j]);
      }
    }
  }
}
void fillCircle(int x, int y, int r, int color, boolean isnight) {
  for (int i = x - r; i < x + r; i++) {
    for (int j = y - r; j < y + r; j++) {
      if (((i - x) * (i - x) + (j - y) * (j - y)) <= (r * r)) {
        fillTab(i, j, color, isnight);
      }
    }
  }
}
void drawBit(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        //dma_display->drawPixel(x + i, y + j, color);
        fillTab(x + i, y + j, color, isnight);
      }
    }
  }
}
void drawLine(int x0, int y0, int x1, int sec, boolean isnight) {
  for (int i = x0; i < x1; i++) { //横向
    colorl = rand() % 0xffff;
    if (!isnight) {
      //dma_display->drawPixel(x0 + i, y0, colorl);
      fillTab(x0 + i, y0, colorl, isnight);
    }
  }
}
void drawHLine(int x0, int y0, int y1, int sec, boolean isnight) {
  for (int i = 0; i < y1; i++) { //横向
    colorl = rand() % 0xffff;
    if (!isnight) {
      //dma_display->drawPixel(x0, y0+i, colorl);
      fillTab(x0, y0 + i, colorl, isnight);
    }
  }
}
void drawBit2(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;
  int first = 0;
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        if (first == 0 & !isnight) {
          //dma_display->drawPixel(x + i, y + j, color+TFT_SILVER);
          fillTab(x + i, y + j, color + TFT_SILVER, isnight);
        } else {
          //dma_display->drawPixel(x + i, y + j, color);
          fillTab(x + i, y + j, color, isnight);
        }
        first++;
      }
    }
    first = 0;
  }
}
void drawSmBit(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight) {
  int32_t i, j;
  unsigned char a[3];
  unsigned char temp;
  for (j = 0; j < height; j++) {
    temp = bitmap[j];
    for (i = width; i > 0; i--) {
      a[i - 1] = temp & 0x01;
      temp >>= 1;
      if (a[i - 1] == 1) {
        // dma_display->drawPixel(x + i, y + j, color);
        fillTab(x + i, y + j, color, isnight);
      }
    }
  }
}
void display30Number(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++) {
    if (shuzi30[k].Index == c)   {
      drawBit(x, y, shuzi30[k].sz30_Id, 15 , 30, color, isnight);
    }
  }

}
void display30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0) {
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
    count++;  //count表示val是一个几位数
  }

}
void drawColorBit3(int x, int y, const uint16_t *bitmap , int width, int height)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++ ) {
      if (bitmap[i + j * width] != 0) {
        dma_display->drawPixel(x + i, y + j, bitmap[i + j * width]);
      }
    }
  }
}
void drawColorBit(int x, int y, const uint16_t *bitmap , int width, int height, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++ ) {
      if (bitmap[i + j * width] != 0) {
          fillTab(x + i, y + j, bitmap[i + j * width], isnight);
      }
    }
  }
}
void drawColorBit2(int x, int y, const uint16_t *bitmap , int width, int height, boolean isnight)
{

  int32_t i, j, byteWidth = (width + 7) / 8;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++ ) {
      if (bitmap[i + j * width] != 0) {
          fillTab(x + i, y + j, bitmap[i + j * width], isnight);
      }
    }
  }
}
void showTQ(int c, int x, int y, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 61; k++) {
    if (tq20[k].Index == c)   {
      drawColorBit2(x, y, tq20[k].tq20_Id, 20 , 20, isnight);
    }
  }

}
void drawText(String words, int x, int y)
{
  dma_display->setCursor(x, y);
  dma_display->setTextColor(dma_display->color565(255, 255, 0));
  dma_display->print(words);
}
void clearOLED() {
  dma_display->clearScreen();
}

void initOLED(int panel_chain, int light) {

  // Module configuration
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    panel_chain    // Chain length
  );

  mxconfig.gpio.e = 32;
  mxconfig.clkphase = false;
  mxconfig.driver = HUB75_I2S_CFG::FM6126A;

  // Display Setup
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(50); //0-255
  dma_display->clearScreen();
  dma_display->fillScreen(myBLACK);
}
void setBrightness(int dianya, int light) {
  dma_display->setBrightness8(light * dianya); //0-255
}
void displayNumber(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++) {
    if (shuzi24[k].Index == c)   {
      drawBit2(x, y, shuzi24[k].hz24_Id, 12 , 24, color, isnight);
    }
  }

}
void disSmallNumber(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++) {
    if (smsz[k].Index == c)   {
      //  Serial.println("run here");
      drawSmBit(x, y, smsz[k].smsz_Id, 3 , 5, color, isnight);
    }
  }
}
void disSmallNumbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0) {
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
    count++;  //count表示val是一个几位数
  }

}
void dis30Number(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++) {
    if (shuzi30[k].Index == c)   {
      //  Serial.println("run here");
      drawBit(x, y, shuzi30[k].sz30_Id, 30 , 60, color, isnight);
    }
  }
}
void dis30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0) {
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
    count++;  //count表示val是一个几位数
  }

}
void displayNumbers(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0) {
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
    count++;  //count表示val是一个几位数
  }

}
void disSmallChar(char c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 11; k++) {
    if (smchar[k].Index == c)   {
      //Serial.println("run here!DrawChar");
      drawSmBit(x, y, smchar[k].smchar_Id, 3 , 5, color, isnight);
    }
  }
}
void drawChars(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight) {
  int x0 = x;
  for (int i = 0; i < strlen(str); i++) {
    // Serial.print("当前char:");
    // Serial.println(str[i]);
    disSmallChar(str[i], x0, y, color, isnight);
    x0 += 4;
  }
}
void displayNumber2(int c, int x, int y, uint16_t color, boolean isnight)
{
  int hz_width;
  for (int k = 0; k < 10; k++) {
    if (shuzi14[k].Index == c)   {
      drawBit(x, y, shuzi14[k].hz14_Id, 8 , 14, color, isnight);
    }
  }

}
void displayNumbers2(int numbers, int x, int y, uint16_t color, boolean isnight)
{
  int count = 0;
  int temp = 0;
  char buffer[32];
  if (numbers == 0) {
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
    count++;  //count表示val是一个几位数
  }

}
void drawHz (int xx,int yy,unsigned char*names,uint32_t color, boolean isnight){
  unsigned char buffs[24];
  //建立缓存空间
  getfont(names, sizeof(names), buffs);
  int kj = 0;
  int x=0;
  int y=0;
  for (int i = 0; i < 24; i++) {
    for (int s = 7 ; s >= 0 ; s--)
    {
      if (buffs[i] & (0x01 << s))
      {  
         if(7-s+xx>=0){
         if(i%2==0){
         fillTab(7-s+xx,y+yy,color, isnight);
         }else{
         fillTab(15-s+xx,y+yy,color, isnight);
         }
         }
      }
    }
    if ((i + 1) % 2 == 0)
    {
      y+=1;
    }
  }
  }
void drawHanziS(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight) {
  int x0 = x;
  unsigned char b[3];
  for (int i = 0; i < strlen(str); i = i + 3) {
    b[0] = str[i];
    b[1] = str[i + 1];
    b[2] = str[i + 2];
    drawHz(x0, y, b, color, isnight);
    x0 += 11;
  }
}
