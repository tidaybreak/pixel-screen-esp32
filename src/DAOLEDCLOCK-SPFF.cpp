#include <TimeLib.h>
#include "time.h"

#include <ArduinoJson.h>
#include <SPI.h>
#include "SPIFFS.h"
#include <FS.h>
#include <SimpleDHT.h>
#include <EEPROM.h>
#include "define.h"
#include "OLEDDriver.h"
#include "ShowDisplay.h"
#include "api.h"
#include "Server.h"
#include "WiFiClient.h"
#include <ESP32Ping.h>



int pinDHT11 = 21; // IO2
SimpleDHT11 dht11(pinDHT11);


boolean isnight = false;

// bdd
// const uint16_t* bddgif[] = {bdd1, bdd2, bdd3, bdd4, bdd5, bdd6, bdd7, bdd8, bdd9, bdd10, bdd11, bdd12, bdd13, bdd14,bdd15};
int PANEL_CHAIN = 2; // Total number of panels chained one to another

String macAddr = WiFi.macAddress();
String clockname = "";

DATATIME timenow;
CONF conf;
DATACLOCK clockinfo;
WEATHER weatherinfo;
NIGHTCOLOR nightcolor;

int len_city;
int len_key;
int netpage_wait = 0;

union para_value
{ // 联合体，保存int
  int val;
  byte val_b[2];
};
para_value e_int;
//****载入配置
void myconfig(CONF *conf)
{
  EEPROM.begin(1024);
  e_int.val_b[0] = EEPROM.read(0);
  e_int.val_b[1] = EEPROM.read(1);
  conf->temp_mod = e_int.val;
  if (conf->temp_mod >= 0 && conf->temp_mod < 32768)
    conf->temp_mod = conf->temp_mod;
  if (conf->temp_mod >= 32768)
    conf->temp_mod = conf->temp_mod - 65536;
  e_int.val_b[0] = EEPROM.read(2);
  e_int.val_b[1] = EEPROM.read(3);
  conf->hum_mod = e_int.val;
  if (conf->hum_mod >= 0 && conf->hum_mod < 32768)
    conf->hum_mod = conf->hum_mod;
  if (conf->hum_mod >= 32768)
    conf->hum_mod = conf->hum_mod - 65536;

  // 读取城市8~len_city
  len_city = EEPROM.read(4);
  len_key = EEPROM.read(5);
  conf->city = "";
  conf->zx_key = "";
  for (int i = 0; i < len_city; i++)
  {
    conf->city += char(EEPROM.read(6 + i));
  }
  // 读取key 11+len_city~11+len_city+len_key
  for (int i = 0; i < len_key; i++)
  {
    conf->zx_key += char(EEPROM.read(7 + len_city + i));
  }
  e_int.val_b[0] = EEPROM.read(8 + len_city + len_key); // temo
  e_int.val_b[1] = EEPROM.read(9 + len_city + len_key);
  conf->light = e_int.val;
  if (conf->light >= 0 && conf->light < 32768)
    conf->light = conf->light;
  if (conf->light >= 32768)
    conf->light = conf->light - 65536;
  conf->soundon = EEPROM.read(10 + len_city + len_key);
  conf->caidaion = EEPROM.read(11 + len_city + len_key);
  conf->isDoubleBuffer = EEPROM.read(12 + len_city + len_key);
  conf->twopannel = EEPROM.read(13 + len_city + len_key);
  if (conf->twopannel)
  {
    PANEL_CHAIN = 2;
  }
  else
  {
    PANEL_CHAIN = 1;
  }
  conf->isnightmode = EEPROM.read(14 + len_city + len_key);
  Serial.println("load config success!");
  Serial.println(String("config city:") + String(conf->city));
  Serial.println(String("config zx_key:") + String(conf->zx_key));
}

void saveconfig(CONF &conf)
{
  EEPROM.begin(1024);
  e_int.val = conf.temp_mod;
  EEPROM.write(0, e_int.val_b[0]); // temo
  EEPROM.write(1, e_int.val_b[1]);
  e_int.val = conf.hum_mod;
  EEPROM.write(2, e_int.val_b[0]); // hum
  EEPROM.write(3, e_int.val_b[1]);

  // 保存城市和key 用8，9存储长度
  int i = 0;
  len_city = conf.city.length();
  len_key = conf.zx_key.length();
  EEPROM.write(4, len_city);
  EEPROM.write(5, len_key);
  char citychar[conf.city.length()];
  strcpy(citychar, conf.city.c_str());
  for (; i < strlen(citychar); i++)
  {
    EEPROM.write(6 + i, citychar[i]);
  }
  Serial.println(conf.city);
  // 保存key
  char keychar[len_key];
  strcpy(keychar, conf.zx_key.c_str());
  for (int j = 0; j < strlen(keychar); j++)
  {
    EEPROM.write(7 + len_city + j, keychar[j]);
  }
  // 保存亮度
  e_int.val = conf.light;
  EEPROM.write(8 + len_city + len_key, e_int.val_b[0]); // temo
  EEPROM.write(9 + len_city + len_key, e_int.val_b[1]);
  EEPROM.write(10 + len_city + len_key, conf.soundon);
  EEPROM.write(11 + len_city + len_key, conf.caidaion);
  EEPROM.write(12 + len_city + len_key, conf.isDoubleBuffer);
  EEPROM.write(13 + len_city + len_key, conf.twopannel);
  EEPROM.write(14 + len_city + len_key, conf.isnightmode);

  EEPROM.commit();
  Serial.print("save config success!");
}

void dht11read(DATACLOCK *dclock)
{
  int err = SimpleDHTErrSuccess;
  //Serial.print(err);
  dht11.read(&dclock->temperature, &dclock->humidity, NULL);
}






float sensor_Read()
{
  int sensorValue = analogRead(34);            // 读取模拟引脚34上的输入
  float voltage = sensorValue * (10 / 1023.0); // 将模拟读数（从0 - 1023变为0）转换为电压（0 - 5V）
  //  Serial.print("电压：");
  //  Serial.println(voltage); // 打印出模拟口读取的值
  if (voltage < 1)
  {
    voltage = 1;
  }
  return voltage;
}

void refreshData(void *parameter)
{
  // 每天0点更新网络时间
  DATATIME * t = (DATATIME *) parameter;
  if (t->minu == 0 && t->sec == 0)
  {
    setSyncProvider(getNtpTime);
    t->GetTime();
    getNongli(&clockinfo, timenow);
  }
  //

  vTaskDelete(NULL);
}

void refreshTQ(void *parameter)
{
  getWeather(&clockinfo, conf);
  get3DayWeather(&weatherinfo, nightcolor, conf);
  getBirth(&clockinfo, macAddr);
  vTaskDelete(NULL);
}

void get_system_info() {
  printf("\n\n-------------------------------- Get Systrm Info------------------------------------------\n");
  //获取IDF版本
  printf("     SDK version:%s\n", esp_get_idf_version());
  //获取芯片可用内存
  printf("     esp_get_free_heap_size : %d  \n", esp_get_free_heap_size());
  //获取从未使用过的最小内存
  printf("     esp_get_minimum_free_heap_size : %d  \n", esp_get_minimum_free_heap_size());
  //获取芯片的内存分布，返回值具体见结构体 flash_size_map
  //printf("     system_get_flash_size_map(): %d \n", system_get_flash_size_map());
  //获取mac地址（station模式）
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  printf("     esp_read_mac(): %02x:%02x:%02x:%02x:%02x:%02x \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("--------------------------------------------------------------------------\n\n");

}

void setup()
{
  Serial.begin(115200);
  get_system_info();

  myconfig(&conf);
  initOLED(PANEL_CHAIN, conf.light);
  int i = 0;
  // connect to WiFi
  drawText("Try to connect WIFI!", 64, 0);

  connectToWiFi(15); // apConfig();

  if (WiFi.status() == WL_CONNECTED)
  {
    updateServer();
    clearOLED();
    Serial.println(" CONNECTED");
    drawText("CONNECTED!", 64, 0);
    drawText(WiFi.localIP().toString().c_str(), 0, 20);
    Serial.print(WiFi.localIP());
    // init and get the time
    setSyncProvider(getNtpTime);
    timenow.GetTime();

    getNongli(&clockinfo, timenow);
    getWeather(&clockinfo, conf);
    get3DayWeather(&weatherinfo, nightcolor, conf);
    getBirth(&clockinfo, macAddr);
    dht11read(&clockinfo);
    showTime(timenow, conf, clockinfo, weatherinfo, nightcolor, true);
  }

  SPIFFS.begin();
  while (!SPIFFS.begin(true))
  {
    Serial.print("****");
  }

  Serial.println("SPIFFS OK!");
}

void loop()
{

  // delay(1000);

  // const IPAddress remote_ip(192, 168, 3, 1);
  // int ret = Ping.ping(remote_ip, 1);
  // int avg_time_ms = Ping.averageTime();
  // Serial.println(GetTimeStr() + String(" ret:") + String(ret) + String(" avg:") + String(avg_time_ms));
  // //printf("ret: %d avg:%d \n", ret, avg_time_ms);
  // return ;

  
  handleWiFiRequest();
  handleApiRequest();

  // 读温度
  dht11read(&clockinfo);


  // 刷新亮度
  if (sensor_Read() < 1.2)
  {
    nightcolor.nightMode();
  }
  else
  {
    nightcolor.set_isnight(false);
  }

  if (timenow.minu == 0 && timenow.sec == 0)
  {
    setSyncProvider(getNtpTime);
    timenow.GetTime();
    getNongli(&clockinfo, timenow);
  }
  if (timenow.minu % 2 == 0 && timenow.sec == 0 && timenow.minu % 10 != 0)
  {
    getBirth(&clockinfo, macAddr);
  }
  if (timenow.sec % 10 == 0)
  {
    if (getConf(&conf, macAddr)) {
      saveconfig(conf);
    }
  }

  setBrightness(sensor_Read(), conf.light);
  cleanTab();

  if (WiFi.status() == WL_CONNECTED || netpage_wait > 50)
  {
    // 任务 每天0点更新网络时间
    xTaskCreate(
        refreshData,
        "refreshData",
        100000,
        &timenow,
        1,
        NULL);
    if (conf.isnightmode)
    {
      if (hour() > 5 && hour() < 23)
      {
        // 更新天气
        if (timenow.minu % 10 == 0 && timenow.sec == 0 && timenow.minu != 0)
        {
          xTaskCreate(refreshTQ, "refreshTQ", 10000, NULL, 1, NULL);
        }
        timenow.GetTime();
        showTime(timenow, conf, clockinfo, weatherinfo, nightcolor, true);
        showTigger(timenow, conf, clockinfo, true);
      }
      else
      { // 夜间只显示时间
        if (conf.twopannel)
        {
          set_screen_num(0);
          onlyShowTime(timenow, conf, nightcolor, true);
        }
        else
        {
          set_screen_num(0);
          onlyShowTime2(timenow, conf, nightcolor, true);
        }
        
        nightcolor.set_isnight(true);

      }
    }
    else
    {
      timenow.GetTime();
      showTime(timenow, conf, clockinfo, weatherinfo, nightcolor, true);
      showTigger(timenow, conf, clockinfo, true);
    }
    fillScreenTab(conf.twopannel, timenow.minu, true);
  }
  else
  {
    drawBit(116, 52, iconwifi, 12, 12, TFT_DARKGREY, true);
    Serial.println(" s" + WiFi.status());
    connectToWiFi(15);
  }
  if (netpage_wait < 52)
  {
    netpage_wait++;
  }
  delay(350);
}
