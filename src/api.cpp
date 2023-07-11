#include "api.h"
#include "define.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "OLEDDriver.h"



void http_url(String URL) {

}

// 获取API里的纪念日
void getBirth(DATACLOCK *cinfo, String &macAddr)
{
  HTTPClient http;
  // We now create a URI for the request
  /*
    {
      DATA: [ ]
    }
  */
  String url = "http://82.157.26.5/getShowText?clockid=" + macAddr;
  http.begin(url.c_str());
  String payload;
  // 发起http get请求
  int httpResponseCode = http.GET();
  Serial.println(String(url) + String(" getBirth HTTP Response code:") + String(httpResponseCode));

  if (httpResponseCode == 200)
  {
    payload = http.getString();
    Serial.println(payload);
    // String input;
    StaticJsonDocument<192> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }

    for (JsonObject DATA_item : doc["DATA"].as<JsonArray>())
    {

      const char *DATA_item_Title = DATA_item["Title"]; // "爸爸生日", "妈妈生日"
      const char *DATA_item_Time = DATA_item["Time"];   // "三月二十", "八月二十九"
      cinfo->jieri = DATA_item_Title;
    }
  }
}
/*
   获取配置信息
*/
bool getConf(CONF *conf, String &macAddr)
{
  HTTPClient http;
  String url = "http://82.157.26.5/index?clockid=" + macAddr;
  http.begin(url.c_str());
  String payload;
  // 发起http get请求
  int httpResponseCode = http.GET();
  Serial.println(String(url) + String(" getConf HTTP Response code:") + String(httpResponseCode));
  if (httpResponseCode == 200)
  {
    payload = http.getString();
    Serial.println(payload);
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
      //Serial.print("getConf deserializeJson() failed: ");
      //Serial.println(error.c_str());
      //return;
    }

    JsonObject DATA = doc["DATA"];
    const char *DATA_isupdate = "0"; //DATA["isupdate"];             // "0"
    const char *DATA_isnightmode = "false"; // DATA["isnightmode"];       // "false"
    const char *DATA_temp = "-3";  //DATA["temp"];                     // "-3"
    const char *DATA_hum = "0"; // DATA["hum"];                       // "0"
    const char *DATA_chengshi = "101011200"; //DATA["chengshi"];             // "101011200"
    const char *DATA_tianqikey = "6967b386afb341999ee91c0321176ab7"; //DATA["tianqikey"];           // "6967b386afb341999ee91c0321176ab7"
    const char *DATA_light = "10";// DATA["light"];                   // "10"
    const char *DATA_starnum = "15"; // DATA["starnum"];               // "15"
    const char *DATA_soundon = "true"; // DATA["soundon"];               // "true"
    const char *DATA_caidaion = "false"; // DATA["caidaion"];             // "false"
    const char *DATA_isDoubleBuffer = "false"; // DATA["isDoubleBuffer"]; // "false"
    const char *DATA_twopannel = "false"; // DATA["twopannel"];           // "false"
    if (strcmp(DATA_isupdate, "true") == 0)
    {
      Serial.println("run update");
      conf->temp_mod = atoi(DATA_temp);
      conf->hum_mod = atoi(DATA_hum);
      conf->city = DATA_chengshi;
      conf->zx_key = DATA_tianqikey;
      conf->light = atoi(DATA_light);
      conf->starnum = atoi(DATA_starnum);
      if (strcmp(DATA_soundon, "true") == 0)
      {
        conf->soundon = true;
      }
      else
      {
        conf->soundon = false;
      }
      if (strcmp(DATA_caidaion, "true") == 0)
      {
        conf->caidaion = true;
      }
      else
      {
        conf->caidaion = false;
      }
      if (strcmp(DATA_isDoubleBuffer, "true") == 0)
      {
        conf->isDoubleBuffer = true;
      }
      else
      {
        conf->isDoubleBuffer = false;
      }
      if (strcmp(DATA_isnightmode, "true") == 0)
      {
        conf->isnightmode = true;
      }
      else
      {
        conf->isnightmode = false;
      }
      if (strcmp(DATA_twopannel, "true") == 0)
      {
        conf->twopannel = true;
      }
      else
      {
        conf->twopannel = false;
      }
      return true;
    }
  }
  return false;
}


//***send http 获取农历
void getNongli(DATACLOCK *cinfo, DATATIME &t)
{

  HTTPClient http;
  // We now create a URI for the request
  String url = "http://www.autmone.com/openapi/icalendar/queryDate?date=" + String(t.year1) + "-" + String(t.month1) + "-" + String(t.day1);
  http.begin(url.c_str());
  String payload;
  // 发起http get请求
  int httpResponseCode = http.GET();
  Serial.println(String(url) + String(" getNongli HTTP Response code:") + String(httpResponseCode));

  if (httpResponseCode > 0)
  {
    payload = http.getString();
    Serial.println(payload);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  // 利用arduinoJson库解析心知返回的json天气数据
  // 能够利用 https://arduinojson.org/v6/assistant/ Arduinojson助手生成相关json解析代码  很方便！！！
  StaticJsonDocument<768> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  int code = doc["code"];       // 0
  const char *msg = doc["msg"]; // "SUCCESS"
  long long time = doc["time"]; // 1631449659945
  JsonObject data = doc["data"];
  int data_iYear = data["iYear"];                         // 2021
  int data_iMonth = data["iMonth"];                       // 11
  int data_iDay = data["iDay"];                           // 6
  const char *data_iMonthChinese = data["iMonthChinese"]; // "十一月"
  const char *data_iDayChinese = data["iDayChinese"];     // "初六"
  int data_sYear = data["sYear"];                         // 2021
  int data_sMonth = data["sMonth"];                       // 12
  int data_sDay = data["sDay"];                           // 9
  const char *data_cYear = data["cYear"];                 // "辛丑年"
  //cinfo->china_year = new char[strlen(data_cYear) + 1];
  //strcpy(cinfo->china_year, data_cYear);
  cinfo->china_year = String(data_cYear);
  //cinfo->china_month = new char[strlen(data_iMonthChinese) + 1];
  //strcpy(cinfo->china_month, data_iMonthChinese);
  cinfo->china_month = String(data_iMonthChinese);
  //cinfo->china_day = new char[strlen(data_iDayChinese) + 1];
  //strcpy(cinfo->china_day, data_iDayChinese);
  cinfo->china_day = String(data_iDayChinese);
  Serial.print(cinfo->china_year);
  Serial.print(cinfo->china_month);
  Serial.print(cinfo->china_day);
  const char *data_solarFestival = data["solarFestival"]; // " 世界足球日"
  cinfo->jieri = data_solarFestival;
  cinfo->jieri.replace(" ", "");
  const char *data_solarTerms = data["solarTerms"];       // nullptr节气
  const char *data_lunarFestival = data["lunarFestival"]; // nullptr
  //cinfo->jieqi = new char[strlen(data_solarTerms) + 1];
  //strcpy(cinfo->jieqi, data_solarTerms);
  cinfo->jieqi = String(data_solarTerms);
  // Serial.print(sizeof(jieqi));
  const char *data_week = data["week"]; // "四"
  if (strcmp(cinfo->china_month.c_str(), "十一月") == 0)
  {
    cinfo->china_month = "冬月";
  }
  if (strcmp(cinfo->china_month.c_str(), "十二月") == 0)
  {
    cinfo->china_month = "腊月";
  }
}



//***send http 获取天气
void getWeather(DATACLOCK *cinfo, CONF &conf) {
  HTTPClient http;

  // We now create a URI for the request
  if(conf.zx_key.length() > 5 && conf.city.length() > 0){
  String url = "https://devapi.heweather.net/v7/weather/now?key="+conf.zx_key+"&location="+conf.city+"&unit=m&lang=zh&gzip=n";;
  http.begin(url.c_str());
  String payload ;
  //发起http get请求
  int httpResponseCode = http.GET();
  Serial.println(String(url) + String(" getWeather HTTP Response code:") + String(httpResponseCode));

  if (httpResponseCode ==200) {
    payload = http.getString();

    Serial.println(String(payload.length()) + " payload:" + payload);
    StaticJsonDocument<768> doc;

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    } else {

      const char* code = doc["code"]; // "200"
      const char* updateTime = doc["updateTime"]; // "2021-11-16T22:47+08:00"
      const char* fxLink = doc["fxLink"]; // "http://hfx.link/2ax1"

      JsonObject now = doc["now"];
      const char* now_obsTime = now["obsTime"]; // "2021-11-16T22:36+08:00"
      const char* now_temp = now["temp"]; // "6"
      const char* now_feelsLike = now["feelsLike"]; // "4"
      const char* now_icon = now["icon"]; // "150"
      const char* now_text = now["text"]; // "晴"
      const char* now_wind360 = now["wind360"]; // "180"
      const char* now_windDir = now["windDir"]; // "南风"
      const char* now_windScale = now["windScale"]; // "1"
      const char* now_windSpeed = now["windSpeed"]; // "4"
      const char* now_humidity = now["humidity"]; // "84"
      const char* now_precip = now["precip"]; // "0.0"
      const char* now_pressure = now["pressure"]; // "1021"
      const char* now_vis = now["vis"]; // "6"
      const char* now_cloud = now["cloud"]; // "20"
      const char* now_dew = now["dew"]; // "3"

      JsonArray refer_sources = doc["refer"]["sources"];
      const char* refer_sources_0 = refer_sources[0]; // "QWeather"
      const char* refer_sources_1 = refer_sources[1]; // "NMC"
      const char* refer_sources_2 = refer_sources[2]; // "ECMWF"

      const char* refer_license_0 = doc["refer"]["license"][0]; // "no commercial use"
      //wea_code = atoi(now_icon);
      cinfo->wea_hm = atoi(now_humidity);
      strcpy(cinfo->wea_temp1, now_temp);
      Serial.print("温度");
      Serial.println(now_temp);
      Serial.println(now_icon);
      // if(wea_code==151){
      //   wea_code=101;
      // }
    }

  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
      //wea_code = 999;
      cinfo->wea_hm = httpResponseCode;
      strcpy(cinfo->wea_temp1,"-1");
      
  }
  // Free resources
  http.end();

  //利用arduinoJson库解析心知返回的json天气数据
  //能够利用 https://arduinojson.org/v6/assistant/ Arduinojson助手生成相关json解析代码  很方便！！！
  // String input;

  // String input;
  }

}
//获取未来3天天气
void get3DayWeather(WEATHER *winfo, NIGHTCOLOR &nightcolor, CONF &conf) {
  HTTPClient http;
  // We now create a URI for the request
  if (conf.zx_key.length() > 5 && conf.city.length() > 0) {
    String url = "https://devapi.qweather.com/v7/weather/3d?key=" + conf.zx_key + "&location=" + conf.city + "&unit=m&lang=zh&gzip=n";;
    http.begin(url.c_str());
    String payload ;
    //发起http get请求
    int httpResponseCode = http.GET();
    Serial.println(String(conf.zx_key) + String("|") + String(conf.city) + String(url) + String(" get3DayWeather HTTP Response code:") + String(httpResponseCode));

    if (httpResponseCode == 200) {
      payload = http.getString();
      Serial.println(payload);
      // String input;

      DynamicJsonDocument doc(3072);

      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        return;
      }

      const char* code = doc["code"]; // "200"
      const char* updateTime = doc["updateTime"]; // "2022-02-19T21:35+08:00"
      const char* fxLink = doc["fxLink"]; // "http://hfx.link/2b81"
      int i = 0;
      for (JsonObject daily_item : doc["daily"].as<JsonArray>()) {
        const char* daily_item_fxDate = daily_item["fxDate"]; // "2022-02-19", "2022-02-20", "2022-02-21"
        const char* daily_item_tempMax = daily_item["tempMax"]; // "2", "5", "6"
        const char* daily_item_tempMin = daily_item["tempMin"]; // "-10", "-8", "-8"
        const char* daily_item_iconDay = daily_item["iconDay"]; // "100", "100", "100"
        const char* daily_item_iconNight = daily_item["iconNight"]; // "150", "151", "150"
        const char* daily_item_humidity = daily_item["humidity"]; // "50", "35", "33"
        if (i == 0) {
          winfo->wea_code_day1 = atoi(daily_item_iconDay);
          winfo->wea_code_night1 = atoi(daily_item_iconNight);
          if (winfo->wea_code_day1 == 151) {
            winfo->wea_code_day1 = 101;
          }
          if (winfo->wea_code_night1 == 151) {
            winfo->wea_code_night1 = 101;
          }
          displayNumbers2(i, 0+ i*10, 30, nightcolor.color2, true);

          strcpy(winfo->tem_day1_min,daily_item_tempMin);
          strcpy(winfo->tem_day1_max,daily_item_tempMax);
          strcpy(winfo->day1_date,daily_item_fxDate);

        }
        else if (i == 1) {
          winfo->wea_code_day2 = atoi(daily_item_iconDay);
          winfo->wea_code_night2 = atoi(daily_item_iconNight);
          if (winfo->wea_code_day2 == 151) {
            winfo->wea_code_day2 = 101;
          }
          if (winfo->wea_code_night2 == 151) {
            winfo->wea_code_night2 = 101;
          }
          strcpy(winfo->tem_day2_min ,daily_item_tempMin);
          strcpy(winfo->tem_day2_max ,daily_item_tempMax);
          strcpy(winfo->day2_date,daily_item_fxDate);
        } else {
          winfo->wea_code_day3 = atoi(daily_item_iconDay);
          winfo->wea_code_night3 = atoi(daily_item_iconNight);
          if (winfo->wea_code_day3 == 151) {
            winfo->wea_code_day3 = 101;
          }
          if (winfo->wea_code_night3 == 151) {
            winfo->wea_code_night3 = 101;
          }
          strcpy(winfo->tem_day3_min ,daily_item_tempMin);
          strcpy(winfo->tem_day3_max ,daily_item_tempMax);
          strcpy(winfo->day3_date,daily_item_fxDate);
        }
        i++;
      }

      JsonArray refer_sources = doc["refer"]["sources"];
      const char* refer_sources_0 = refer_sources[0]; // "QWeather"
      const char* refer_sources_1 = refer_sources[1]; // "NMC"
      const char* refer_sources_2 = refer_sources[2]; // "ECMWF"

      const char* refer_license_0 = doc["refer"]["license"][0]; // "no commercial use"


    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      // wea_code = 999;
      // wea_temp1="0";

    }
    // Free resources
    http.end();

    //利用arduinoJson库解析心知返回的json天气数据
    //能够利用 https://arduinojson.org/v6/assistant/ Arduinojson助手生成相关json解析代码  很方便！！！
    // String input;

    // String input;
  }

}
