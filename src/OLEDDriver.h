#ifndef OLEDDRIVER_H_H      
#define OLEDDRIVER_H_H

#include<Arduino.h>

class MatrixPanel_I2S_DMA;

void setTextColor(uint16_t c);

void set_screen_num(int val);
int get_screen_num();

void cleanTab();
void fillTab(int x, int y, uint16_t color, boolean isnight=false);

void fillCircle(int x, int y, int r, int color, boolean isnight);
void drawBit(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight);
void drawLine(int x0, int y0, int x1, int sec, boolean isnight);
void drawHLine(int x0, int y0, int y1, int sec, boolean isnight);
void drawBit2(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight);
void drawSmBit(int x, int y, const uint8_t *bitmap , int width, int height, uint16_t color, boolean isnight);
void display30Number(int c, int x, int y, uint16_t color, boolean isnight);
void display30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight);
void drawColorBit3(int x, int y, const uint16_t *bitmap , int width, int height, boolean isnight);
void drawColorBit(int x, int y, const uint16_t *bitmap , int width, int height, boolean isnight);
void drawColorBit2(int x, int y, const uint16_t *bitmap , int width, int height, boolean isnight);
void showTQ(int c, int x, int y, boolean isnight);

void initOLED(int panel_chain, int light);
MatrixPanel_I2S_DMA *get_oled();
void clearOLED();
void text(const String &content, bool clear=false, int x=-1, int y=-1, const char *color=NULL, int fsize=1, const char *font=NULL);
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, const char *color=NULL);

void setBrightness(int dianya, int light);
void displayNumber(int c, int x, int y, uint16_t color, boolean isnight);
void disSmallNumber(int c, int x, int y, uint16_t color, boolean isnight);
void disSmallNumbers(int numbers, int x, int y, uint16_t color, boolean isnight);
void dis30Number(int c, int x, int y, uint16_t color, boolean isnight);
void dis30Numbers(int numbers, int x, int y, uint16_t color, boolean isnight);
void displayNumbers(int numbers, int x, int y, uint16_t color, boolean isnight);
void disSmallChar(char c, int x, int y, uint16_t color, boolean isnight);
void drawChars(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight);
void displayNumber2(int c, int x, int y, uint16_t color, boolean isnight);
void displayNumbers2(int numbers, int x, int y, uint16_t color, boolean isnight);
int drawHanziS(int32_t x, int32_t y, const char str[], uint32_t color, boolean isnight);

#endif