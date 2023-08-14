#ifndef API_H_H      
#define API_H_H

#include<Arduino.h>
#include "define.h"

struct CONF;
struct DATACLOCK;
struct NIGHTCOLOR;
struct WEATHER;

String http_get(String &url);

// 获取API里的纪念日
void getBirth(DATACLOCK *cinfo, String &macAddr);

bool getConf(CONF *conf, String &macAddr);

//***send http 获取农历
void getNongli(DATACLOCK *cinfo, DATATIME &t);

//获取未来3天天气
void get3DayWeather(WEATHER *winfo, NIGHTCOLOR &nightcolor, CONF &conf);

void getWeather(DATACLOCK *cinfo, CONF &conf);

#endif