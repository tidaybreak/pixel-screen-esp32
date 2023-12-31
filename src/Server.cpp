#include "Server.h"
#include "define.h"
#include "SPIFFS.h"
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include "ShowDisplay.h"
#include "OLEDDriver.h"
#include "config.h"


WebServer server(80);
const char *host = "esp32";
const char *username = "admin";
const char *userpass = "000000";
bool is_running = false;

/*-------- NTP code ----------*/
//const int NTP_PACKET_SIZE = 48;     // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; // buffer to hold incoming & outgoing packets
File fsUploadFile;
const char *ntpServerName = "time1.aliyun.com";
const int timeZone = 8;

WiFiUDP udp;

const char *serverIndex =
    "<!DOCTYPE HTML><html><head>"
    "<script src='https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js'></script>"
    "<title>ESP32_DISPLAYV1.0</title><meta name='viewport' content='width=device-width, initial-scale=1' charset='UTF-8'>"
    "<script type='text/javascript'>"
    "function submit2() {      $.ajax({ type: 'GET', url: '/get', data: {reboot:1 }, dataType: 'html', success: function(result) {   alert('已重启！');}     });   };"
    "function submit3() {      $.ajax({ type: 'GET', url: '/regclock', data: {clockname: $('#clockname').val()}, dataType: 'html', success: function(result) {   alert('已注册！');}     });   };"
    "function submit4() {      $.ajax({ type: 'GET', url: '/text', data: {text: $('#text').val(), args: $('#args').val()}, dataType: 'html', success: function(result) {   }     });   };"
    "function sub_config() {      $.ajax({ type: 'POST', url: '/config', data: {content: $('#config').val()}, dataType: 'html', success: function(result) {   }     });   };"
    "</script></head>"
    "<body bgcolor=lightyellow >"
    "<table border='1' bgcolor=lightblue><tr><td border='0' colspan='4'  align=center><h1><a href='https://github.com/tidaybreak/pixel-screen-esp32'>设置</a></h1></td>"
    "<tr><td><form method='POST' action='configwifi'><label class='input'><span>WiFi SSID</span></td><td><input type='text' name='ssid' value=''></label></td></tr>"
    "<tr><td><label><span>WiFi PASS</span></td><td><input type='text'  name='pass'></label></td></tr><tr><td><input  type='submit' name='submit' value='Submie'></td><td><input type='submit' value='重启时钟'/ onclick='submit2()'></input></td></tr></form>"
    "<tr><td><input type='submit' value='配置'/ onclick='sub_config()'></td><td><textarea style='width:1000px;height:500px' id='config' name='config' placeholder='json config'></textarea></td></tr>"
    "<tr><td><form method='POST' action='/upload' enctype='multipart/form-data'>"
    "<tr><td>更新固件：</td>"
    "<td><form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update'>"
    "<input type='submit' value='Update'>"
    "</form></br>"
    "<div id='prg'>progress: 0%</div><td></tr></table>"
    "</body>"
    "<script>"
    "$('#upload_form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    " $.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!')"
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "$(document).ready(function() {"
    "var configStr = '{\"action\":\"\",\"nodes\":[{\"element\":\"countdown\",\"secs\":18},{\"element\":\"command\",\"command\":\"command\",\"args\":\"1,-1,-1,FFFF00,1,FreeSans9pt7b\",\"delay\":10},{\"element\":\"clock_default\",\"url\":\"http://192.168.2.80/clock\"}]}';"
    "var jsonData = JSON.parse(configStr);"
    "var formattedData = JSON.stringify(jsonData, null, 2);"
    "$('#config').val(formattedData);"
    ""
    "});"
    "</script>";
// json https://www.json.cn/json/jsonzip.html


// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); // NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

void uploadFinish() {
  server.send(200, "text/html", serverIndex );
}

void handlereg()

{
//   HTTPClient http;
//   // We now create a URI for the request
//   if (server.hasArg("clockname"))
//   {
//     clockname = server.arg("clockname");
//   }
//   // http.addHeader("", "-------");
//   String url = "http://82.157.26.5/registerClock?clockid=" + macAddr + "&clockname=" + clockname;
//   Serial.print(url);
//   http.begin(url.c_str());
//   int httpResponseCode = http.GET();

//   if (httpResponseCode == 200)
//   {
//     server.send(200, "text/plain", "alert('注册成功！'" + macAddr + ")");
//   }
}


//上传文件到spiffs中
void handleFileUpload() {
  String filename;
  if (server.uri() != "/upload") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("handleFileUploadx Name: "); Serial.println(filename);
    SPIFFS.remove(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.print("handleFileUploadx Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUploadx Size: "); Serial.println(upload.totalSize);
    drawBmp(filename.c_str(), 0, 0);
  }
}

void dologin()
{
  if (!server.authenticate(username, userpass))
  {
    return server.requestAuthentication();
  }
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", serverIndex);
}

void handleApiRequest() {
  server.handleClient();
}

void handleSet() // 回调函数
{
  if (server.hasArg("reboot"))
  {
    ESP.restart();
  }
  server.send(200, "text/plain", "alert('已重启！')");
}


void handleConfig() {
  String requestBody = server.arg("content");

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, requestBody);
  Serial.println("Received length:" + String(requestBody.length()) + " data: " + requestBody);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    server.send(200, "text/plain", "alert('deserializeJson() failed!')");
    return;
  }

  // Accessing top-level elements
  String s1Value = doc["s1"].as<String>();
  int i1Value = doc["i1"].as<int>();

  Serial.print("s1: ");
  Serial.println(s1Value);
  Serial.print("i1: ");
  Serial.println(i1Value);

  // Accessing the "nodes" array
  JsonArray nodesArray = doc["nodes"].as<JsonArray>();

  for (JsonObject node : nodesArray) {
    String nodeName = node["name"].as<String>();
    String nodeArgs = node["args"].as<String>();

    Serial.print("Node Name: ");
    Serial.println(nodeName);
    Serial.print("Node Args: ");
    Serial.println(nodeArgs);
  }


 String compressedJson = "";
  for (size_t i = 0; i < requestBody.length(); i++) {
    char c = requestBody.charAt(i);
    if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
      compressedJson += c;
    }
  }
  saveconfig(compressedJson);

  server.send(200, "text/plain", "alert('success!')");

}


void handleText() // 回调函数
{
  String content = server.arg("text");
  String args = server.arg("args");

  char *token;
  int numbers[3]; // 假设字符串中有3个整数

  int i = 0;
  token = strtok(const_cast<char*>(args.c_str()), ",");
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

  Serial.println(String("handleText:") + content + " " + args + " " + String(x) + String(y) + String(clear) + color + String(fsize));

  if (content == "command test") {
    fill_rect(0, 0, 128, 64, color.c_str());
  } else {
    text(content, clear, x, y, color.c_str(), fsize);
  }

  server.send(200, "text/plain", "alert('success!')");
}


void startServer()
{
   if (is_running) {
    return;
   }

  /*use mdns for host name resolution*/
  //if (!MDNS.begin(host))
  { // http://esp32.local
    // Serial.println("Error setting up MDNS responder!");
    // while (1)
    // {
    //   delay(1000);
    // }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", []()
    {
      dologin();
    });
  server.on("/serverIndex", HTTP_GET, []()
            {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex); });
  // server.on(
  //     "/upload", HTTP_POST, []()
  //     { uploadFinish(); },
  //     handleFileUpload);
  /*handling uploading firmware file */
  server.on(
      "/upload", HTTP_POST, []()
      {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); },
      []()
      {
        HTTPUpload &upload = server.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { // start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { // true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });
  server.on("/get", HTTP_GET, handleSet);
  server.on("/text", HTTP_GET, handleText);
  server.on("/config", HTTP_POST, handleConfig);
  server.on("/regclock", HTTP_GET, handlereg);
  server.begin();
  is_running = true;
}


void stopServer() {
  server.stop();
  is_running = false;
}

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address
  while (udp.parsePacket() > 0)
    ; // discard any previously received packets
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  return 0; // return 0 if unable to get the time
}