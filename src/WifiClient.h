#ifndef WIFICLIENT_H_H      
#define WIFICLIENT_H_H

#include<Arduino.h>

int wifi_check();

bool connectToWiFi(int timeOut_s);

void handleWiFiRequest();


#endif