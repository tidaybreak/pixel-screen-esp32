//#include <TimeLib.h>
//#include "time.h"
//#include <SPI.h>
#include "SPIFFS.h"
#include <FS.h>
#include <WiFi.h>
#include <SimpleDHT.h>
#include <ArduinoJson.h>
#include "define.h"
#include "config.h"
#include "OLEDDriver.h"
#include "ShowDisplay.h"
#include "api.h"
#include "Server.h"
#include "WiFiClient.h"
//#include "ConfigPortal.h"

//#include <ESP32Ping.h>
//#include <ArduinoJson.h>

#include "element/clock/default/default.h"
#include "element/countdown/countdown.h"
#include "element/command/command.h"


// bdd
// const uint16_t* bddgif[] = {bdd1, bdd2, bdd3, bdd4, bdd5, bdd6, bdd7, bdd8, bdd9, bdd10, bdd11, bdd12, bdd13, bdd14,bdd15};
int PANEL_CHAIN = 2; // Total number of panels chained one to another
int pinDHT11 = 21; // IO2
SimpleDHT11 dht11(pinDHT11);
boolean isnight = false;
String macAddr = WiFi.macAddress();
String clockname = "";
int netpage_wait = 0;

CONF conf;
StaticJsonDocument<MAX_STRING_LENGTH> doc_conf;
static int curr_idx = -1;
DATATIME timenow;
DATACLOCK clockinfo;
WEATHER weatherinfo;
NIGHTCOLOR nightcolor;
para_value e_int;


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
  // DATATIME * t = (DATATIME *) parameter;
  // if (t->minu == 0 && t->sec == 0)
  // {
  //   setSyncProvider(getNtpTime);
  //   t->GetTime();
  //   getNongli(&clockinfo, timenow);
  // }
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
  initconfig(&curr_idx);
  loadconfig(&conf, &doc_conf);
  initOLED(PANEL_CHAIN, conf.light);

  SPIFFS.begin();
  while (!SPIFFS.begin(true))
  {
    Serial.print("****");
  }
}

void loop()
{

  handleApiRequest();

  int wifi_status = wifi_check();
  if(wifi_status == 0 or wifi_status == 1)
  {
    delay(350);
    return;
  } else if (wifi_status == 3) {
    setServer("ntp.aliyun.com");
    waitForSync(5000);
    Serial.println("UTC:" + UTC.dateTime());

    get_system_info();
    startServer();
  }

  // 读温度
  dht11read(&clockinfo);


  JsonArray nodesArray = doc_conf["nodes"].as<JsonArray>();
  if (curr_idx == -1) {
    int i = 0;
    for (JsonObject node : nodesArray) {
      curr_idx = i;
      String element = node["element"].as<String>();
      if (element == "clock_default") {
          element_clock_default_setup(node);
          break;
      } else if (element == "countdown") {
          int secs = node["secs"].as<int>();
          element_countdown_setup(secs);
          break;
      } else if (element == "command") {
          String command = node["command"].as<String>();
          String args = node["args"].as<String>();
          int de = node["delay"].as<int>();
          element_command_setup(command, args, de);
          break;
      } else {
        curr_idx = -1;
      }
      //Serial.println(element);
      i++;
    }
  }

  if (curr_idx == -1) {
    delay(100);
    return; 
  } 

  JsonObject curr_node = ((JsonObject)(nodesArray[curr_idx]));

  String element = curr_node["element"].as<String>();
  if (element == "clock_default") {
    element_clock_default_loop();
  } else if (element == "countdown") {
    if (element_countdown_loop()) {
      Serial.println("remove idx:" + String(curr_idx));
      doc_conf["nodes"].remove(curr_idx);
      curr_idx = -1;
    }
  } else if (element == "command") {
    if (element_command_loop()) {
      Serial.println("remove idx:" + String(curr_idx));
      doc_conf["nodes"].remove(curr_idx);
      curr_idx = -1;
    }
  }

  delay(100);
  return; 



  // // 刷新亮度
  // if (sensor_Read() < 1.2)
  // {
  //   nightcolor.nightMode();
  // }
  // else
  // {
  //   nightcolor.set_isnight(false);
  // }

  // if (timenow.minu == 0 && timenow.sec == 0)
  // {
  //   //setSyncProvider(getNtpTime);
  //   timenow.GetTime();
  //   getNongli(&clockinfo, timenow);
  // }
  // if (timenow.minu % 2 == 0 && timenow.sec == 0 && timenow.minu % 10 != 0)
  // {
  //   getBirth(&clockinfo, macAddr);
  // }
  // if (timenow.sec % 10 == 0)
  // {
  //   if (getConf(&conf, macAddr)) {
  //     //saveconfig(conf);
  //   }
  // }

  // setBrightness(sensor_Read(), conf.light);
  // cleanTab();

  // // 任务 每天0点更新网络时间
  // // xTaskCreate(
  // //     refreshData,
  // //     "refreshData",
  // //     100000,
  // //     &timenow,
  // //     1,
  // //     NULL);
  // if (false)
  // {
  //   if (hour() > 5 && hour() < 23)
  //   {
  //     // 更新天气
  //     if (timenow.minu % 10 == 0 && timenow.sec == 0 && timenow.minu != 0)
  //     {
  //       xTaskCreate(refreshTQ, "refreshTQ", 10000, NULL, 1, NULL);
  //     }
  //     timenow.GetTime();
  //     showTime(timenow, conf, clockinfo, weatherinfo, nightcolor, true);
  //     showTigger(timenow, conf, clockinfo, true);
  //   }
  //   else
  //   { // 夜间只显示时间
  //     if (false)
  //     {
  //       set_screen_num(0);
  //       onlyShowTime(timenow, conf, nightcolor, true);
  //     }
  //     else
  //     {
  //       set_screen_num(0);
  //       onlyShowTime2(timenow, conf, nightcolor, true);
  //     }
      
  //     nightcolor.set_isnight(true);

  //   }
  // }
  // else
  // {
  //   timenow.GetTime();
  //   showTime(timenow, conf, clockinfo, weatherinfo, nightcolor, true);
  //   showTigger(timenow, conf, clockinfo, true);
  // }
  // //fillScreenTab(conf.twopannel, timenow.minu, true);
  

  // if (netpage_wait < 52)
  // {
  //   netpage_wait++;
  // }
  // delay(350);
}
