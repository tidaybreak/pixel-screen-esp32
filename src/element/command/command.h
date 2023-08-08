#ifndef COMMAND_H_H      
#define COMMAND_H_H

#include<Arduino.h>

void element_command_setup(String &command_, String &args_, int delay_);


bool element_command_loop();


#endif