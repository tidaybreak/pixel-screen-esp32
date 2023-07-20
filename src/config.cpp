#include "config.h"
#include <Arduino.h>
#include "define.h"
#include <ArduinoJson.h>
#include <EEPROM.h>

static CONF *conf = NULL;

//****载入配置
void loadconfig(CONF *c, StaticJsonDocument<MAX_STRING_LENGTH> *doc)
{
  conf = c;

  EEPROM.begin(sizeof(CONF));
  EEPROM.get(0, *c);
  c->str_json[MAX_STRING_LENGTH] = '\0';
  Serial.println("db conf:" + String(c->str_json));
  DeserializationError error = deserializeJson(*doc, c->str_json);
  if (error) {
    Serial.println("deserializeJson() failed, init!");
    String configStr = "{\"action\":\"push/del/cover\",\"i1\":1,\"nodes\":[{\"element\":\"clock_teris\",\"args\":\"val\"},{\"element\":\"countdown\",\"secs\":120}]}";
    strncpy(c->str_json, configStr.c_str(), configStr.length());
    c->str_json[configStr.length()] = '\0';
    Serial.println("init conf:" + String(c->str_json));
  }

  // e_int.val_b[0] = EEPROM.read(0);
  // e_int.val_b[1] = EEPROM.read(1);
  // conf->temp_mod = e_int.val;
  // if (conf->temp_mod >= 0 && conf->temp_mod < 32768)
  //   conf->temp_mod = conf->temp_mod;
  // if (conf->temp_mod >= 32768)
  //   conf->temp_mod = conf->temp_mod - 65536;
  // e_int.val_b[0] = EEPROM.read(2);
  // e_int.val_b[1] = EEPROM.read(3);
  // conf->hum_mod = e_int.val;
  // if (conf->hum_mod >= 0 && conf->hum_mod < 32768)
  //   conf->hum_mod = conf->hum_mod;
  // if (conf->hum_mod >= 32768)
  //   conf->hum_mod = conf->hum_mod - 65536;

  // // 读取城市8~len_city
  // int len_city = EEPROM.read(4);
  // int len_key = EEPROM.read(5);
  // conf->city = "";
  // conf->zx_key = "";
  // for (int i = 0; i < len_city; i++)
  // {
  //   conf->city += char(EEPROM.read(6 + i));
  // }
  // // 读取key 11+len_city~11+len_city+len_key
  // for (int i = 0; i < len_key; i++)
  // {
  //   conf->zx_key += char(EEPROM.read(7 + len_city + i));
  // }
  // e_int.val_b[0] = EEPROM.read(8 + len_city + len_key); // temo
  // e_int.val_b[1] = EEPROM.read(9 + len_city + len_key);
  // conf->light = e_int.val;
  // if (conf->light >= 0 && conf->light < 32768)
  //   conf->light = conf->light;
  // if (conf->light >= 32768)
  //   conf->light = conf->light - 65536;
  // conf->soundon = EEPROM.read(10 + len_city + len_key);
  // conf->caidaion = EEPROM.read(11 + len_city + len_key);
  // conf->isDoubleBuffer = EEPROM.read(12 + len_city + len_key);
  // conf->twopannel = EEPROM.read(13 + len_city + len_key);
  // if (conf->twopannel)
  // {
  //   PANEL_CHAIN = 2;
  // }
  // else
  // {
  //   PANEL_CHAIN = 1;
  // }
  // conf->isnightmode = EEPROM.read(14 + len_city + len_key);
  // Serial.println("load config success!");
  // Serial.println(String("config city:") + String(conf->city));
  // Serial.println(String("config zx_key:") + String(conf->zx_key));
}

void saveconfig(String &str)
{
  if (str.length() >= MAX_STRING_LENGTH) {
    Serial.println("to large length:" + String(str.length()));
    return ;
  }

  EEPROM.begin(sizeof(CONF));
  strncpy(conf->str_json, str.c_str(), str.length());
  conf->str_json[str.length()] = '\0';
  EEPROM.put(0, *conf);
  EEPROM.commit();
  Serial.println("save config success! json length:" + String(str.length()) + VAL(conf->str_json));


  // e_int.val = conf.temp_mod;
  // EEPROM.write(0, e_int.val_b[0]); // temo
  // EEPROM.write(1, e_int.val_b[1]);
  // e_int.val = conf.hum_mod;
  // EEPROM.write(2, e_int.val_b[0]); // hum
  // EEPROM.write(3, e_int.val_b[1]);

  // // 保存城市和key 用8，9存储长度
  // int i = 0;
  // int len_city = conf.city.length();
  // int len_key = conf.zx_key.length();
  // EEPROM.write(4, len_city);
  // EEPROM.write(5, len_key);
  // char citychar[conf.city.length()];
  // strcpy(citychar, conf.city.c_str());
  // for (; i < strlen(citychar); i++)
  // {
  //   EEPROM.write(6 + i, citychar[i]);
  // }
  // Serial.println(conf.city);
  // // 保存key
  // char keychar[len_key];
  // strcpy(keychar, conf.zx_key.c_str());
  // for (int j = 0; j < strlen(keychar); j++)
  // {
  //   EEPROM.write(7 + len_city + j, keychar[j]);
  // }
  // // 保存亮度
  // e_int.val = conf.light;
  // EEPROM.write(8 + len_city + len_key, e_int.val_b[0]); // temo
  // EEPROM.write(9 + len_city + len_key, e_int.val_b[1]);
  // EEPROM.write(10 + len_city + len_key, conf.soundon);
  // EEPROM.write(11 + len_city + len_key, conf.caidaion);
  // EEPROM.write(12 + len_city + len_key, conf.isDoubleBuffer);
  // EEPROM.write(13 + len_city + len_key, conf.twopannel);
  // EEPROM.write(14 + len_city + len_key, conf.isnightmode);

}
