#ifndef SHOWDISPLAY_H_H      
#define SHOWDISPLAY_H_H

#include<Arduino.h>
#include "define.h"

void showTime(DATATIME &t, CONF &conf, DATACLOCK &cinfo, WEATHER &winfo, NIGHTCOLOR &ncolor, boolean isnight);

void showTigger(DATATIME &t, CONF &conf, DATACLOCK &cinfo, boolean isnight);

void onlyShowTime(DATATIME &t, CONF &conf, NIGHTCOLOR &ncolor, boolean isnight);

void onlyShowTime2(DATATIME &t, CONF &conf, NIGHTCOLOR &ncolor, boolean isnight);

void showTigger(DATATIME &t, CONF &conf, DATACLOCK &cinfo, boolean isnight);

void drawBmp(const char *filename, int16_t x, int16_t y);


#endif