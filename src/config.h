#ifndef MYCONFIG_H_H      
#define MYCONFIG_H_H

#include <ArduinoJson.h>
#include <Arduino.h>
#include "define.h"

struct CONF;


void initconfig(int *curr_idx_);

void loadconfig(CONF *c, StaticJsonDocument<MAX_STRING_LENGTH> *doc);

void saveconfig(String &conf);

#endif