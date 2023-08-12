#include "element\command\command.h"
#include "OLEDDriver.h"
#include <ezTime.h>

String command;
String args;

int delay_sec = 0;
static int secs_ = 0;
static int init_timestamp_ = 0;

bool do_action = false;

void element_command_setup(String &command_, String &args_, int delay_) {
  Serial.println("element_command_setup:" + command_);

  command = command_;
  args = args_;
  delay_sec = delay_;

  do_action = false;
  init_timestamp_ = now();
}

bool element_command_loop() {
  secs_ = delay_sec - (now() - init_timestamp_);
  if (secs_ <= 0) {
    return true;
  }
  if (do_action) {
    return false;
  }

  char *token;
  int i = 0;
  token = strtok(const_cast<char*>(args.c_str()), ",");

  if (command == "fill_rect") {
    int x = atoi(token);
    token = strtok(NULL, ",");
    int y = atoi(token);
    token = strtok(NULL, ",");
    int w = atoi(token);
    token = strtok(NULL, ",");
    int h = atoi(token);
    token = strtok(NULL, ",");
    String color = String(token);

    fill_rect(x, y, w, h, color.c_str());
  } else {
    bool clear = atoi(token) == 1 ? true : false;
    token = strtok(NULL, ","); 
    int x = atoi(token);
    token = strtok(NULL, ",");
    int y = atoi(token);
    token = strtok(NULL, ",");
    String color = String(token);
    token = strtok(NULL, ",");
    int fsize = atoi(token);
    token = strtok(NULL, ",");
    String font = String(token);
    token = strtok(NULL, ",");

    Serial.println(String("text:") + command + " " + args + " " + String(x) + String(y) + String(clear) + color + String(fsize));
    text(command, clear, x, y, color.c_str(), fsize, font.c_str());
  }
  do_action = true;

  return false;
}