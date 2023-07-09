#ifndef SERVER_H_H      
#define SERVER_H_H

#include<Arduino.h>

time_t getNtpTime();

void updateServer();

void handleApiRequest();

#endif