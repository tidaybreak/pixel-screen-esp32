#include "WifiClient.h"
#include "define.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include "OLEDDriver.h"
#include <WebServer.h>

static WebServer server_client(80);
// AP模式IP
IPAddress apIP(192, 168, 4, 1);
const char *AP_SSID = "Pixel"; // 设置AP热点名称
String wifi_ssid = "";
String wifi_pass = "";
bool first_conn = true;
int ap_timestamp = 0;

#define TIME_ZONE_LEN 100
char timeZone[TIME_ZONE_LEN] = MYTIMEZONE;

// 定义成功页面HTML源代码
#define SUCCESS_HTML "<html><body><font size=\"10\">successd,wifi connecting...<br />Please close this page manually.</font></body></html>"
String scanNetworksID = ""; // 用于储存扫描到的WiFi ID
// 定义根目录首页网页HTML源代码
#define ROOT_HTML "<!DOCTYPE html><html><head><title>WIFI Config by lwang</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><style type=\"text/css\">.input{display: block; margin-top: 10px;}.input span{width: 100px; float: left; float: left; height: 36px; line-height: 36px;}.input input{height: 30px;width: 200px;}.btn{width: 120px; height: 35px; background-color: #000000; border:0px; color:#ffffff; margin-top:15px; margin-left:100px;}</style><body><form method=\"POST\" action=\"configwifi\"><label class=\"input\"><span>WiFi SSID</span><input type=\"text\" name=\"ssid\" value=\"\"></label><label class=\"input\"><span>WiFi PASS</span><input type=\"text\"  name=\"pass\"></label><input class=\"btn\" type=\"submit\" name=\"submit\" value=\"Submie\"> <p><span> Nearby wifi:</P></form>"

//**************************************************************************************************
bool autoConfig()
{
  WiFi.begin(); // 默认连接保存的WIFI
  for (int i = 0; i < 20; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      return true;
    }
    else
    {
      Serial.print("AutoConfig Waiting......");
      Serial.println(WiFi.status());
      delay(1000);
    }
  }
  delay(1000);
  Serial.println("AutoConfig Faild!");
  return false;
}
//***************AP模式设置wifi
// void apConfig(){
//  WiFiManager wifiManager;
//  wifiManager.autoConnect("DaMatrixClock");
//}
//**************************************************************************************************
void smartConfig()
{
  int i = 0;

  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  for (i = 0; i < 30; i++)
  {
    Serial.print(".");
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true); // 设置自动连接
      ESP.restart();
      break;
    }
    delay(2000);
  }
}
//********************************
//***********页面配网****************
//************************************
// 初始化AP模式
void initSoftAP()
{
  ap_timestamp = now();
  WiFi.mode(WIFI_AP);                                         // 配置为AP模式
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // 设置AP热点IP和子网掩码
  if (WiFi.softAP(AP_SSID))
  { // 开启AP热点,如需要密码则添加第二个参数
    // 打印相关信息
    Serial.println("ESP-32S SoftAP is right.");
    Serial.print("Soft-AP IP address = ");
    Serial.println(WiFi.softAPIP());
    Serial.println(String("MAC address = ") + WiFi.softAPmacAddress().c_str());
  }
  else
  { // 开启热点失败
    Serial.println("WiFiAP Failed");
    delay(1000);
    Serial.println("restart now...");
    ESP.restart(); // 重启复位esp32
  }
}

void handleWiFiRequest() {
  server_client.handleClient();
}

// 处理网站根目录“/”(首页)的访问请求,将显示配置wifi的HTML页面
void handleRoot()
{
  if (server_client.hasArg("selectSSID"))
  {
    server_client.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
  }
  else
  {
    server_client.send(200, "text/html", ROOT_HTML + scanNetworksID + "</body></html>");
  }
}

// 设置处理404情况的函数'handleNotFound'
void handleNotFound()
{               // 当浏览器请求的网络资源无法在服务器找到时通过此自定义函数处理
  handleRoot(); // 访问不存在目录则返回配置页面
  //   server.send(404, "text/plain", "404: Not found");
}






void handleConfigWifi()
{
  // 返回http状态
  // server.send(200, "text/html", SUCCESS_HTML);
  if (server_client.hasArg("ssid"))
  { // 判断是否有账号参数
    Serial.print("got ssid:");
    wifi_ssid = server_client.arg("ssid"); // 获取html表单输入框name名为"ssid"的内容
    // strcpy(sta_ssid, server.arg("ssid").c_str());//将账号参数拷贝到sta_ssid中
    Serial.println(wifi_ssid);
  }
  else
  { // 没有参数
    Serial.println("error, not found ssid");
    server_client.send(200, "text/html", "<meta charset='UTF-8'>error, not found ssid"); // 返回错误页面
    return;
  }
  // 密码与账号同理
  if (server_client.hasArg("pass"))
  {
    Serial.print("got password:");
    wifi_pass = server_client.arg("pass"); // 获取html表单输入框name名为"pwd"的内容
    // strcpy(sta_pass, server.arg("pass").c_str());
    Serial.println(wifi_pass);
  }
  else
  {
    Serial.println("error, not found password");
    server_client.send(200, "text/html", "<meta charset='UTF-8'>error, not found password");
    return;
  }
  server_client.send(200, "text/html", "<meta charset='UTF-8'>SSID：" + wifi_ssid + "<br />password:" + wifi_pass + "<br />已取得WiFi信息,正在尝试连接,请手动关闭此页面。"); // 返回保存成功页面
  delay(2000);
  WiFi.softAPdisconnect(true); // 参数设置为true，设备将直接关闭接入点模式，即关闭设备所建立的WiFi网络。
  server_client.close();              // 关闭web服务
  WiFi.softAPdisconnect();     // 在不输入参数的情况下调用该函数,将关闭接入点模式,并将当前配置的AP热点网络名和密码设置为空值.
  Serial.println("WiFi Connect SSID:" + wifi_ssid + "  PASS:" + wifi_pass);
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("开始调用连接函数connectToWiFi()..");
    connectToWiFi(15);
  }
  else
  {
    Serial.println("提交的配置信息自动连接成功..");
  }
}


// 初始化WebServer
void initWebServer()
{

  // 必须添加第二个参数HTTP_GET，以下面这种格式去写，否则无法强制门户
  server_client.on("/", HTTP_GET, handleRoot);                  //  当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server_client.on("/configwifi", HTTP_POST, handleConfigWifi); //  当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理
  server_client.onNotFound(handleNotFound);                     // 当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理
  // Tells the server to begin listening for incoming connections.Returns None
  server_client.begin(); // 启动TCP SERVER
  // server.setNoDelay(true);                                  //关闭延时发送
  Serial.println("WebServer started!");
}

// 扫描WiFi
bool scanWiFi()
{
  Serial.println("scan start");
  // 扫描附近WiFi
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
    scanNetworksID = "no networks found";
    return false;
  }
  else
  {
    // Serial.print(n);
    // Serial.println(" networks found");
    // for (int i = 0; i < n; ++i)
    // {
    //   // Print SSID and RSSI for each network found
    //   Serial.print(i + 1);
    //   Serial.print(": ");
    //   Serial.print(WiFi.SSID(i));
    //   Serial.print(" (");
    //   Serial.print(WiFi.RSSI(i));
    //   Serial.print(")");
    //   Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
    //   scanNetworksID += "<P>" + WiFi.SSID(i) + "</P>";
    //   delay(10);
    // }
    return true;
  }
}

// 用于配置WiFi
void start_ap_mode()
{
  initSoftAP();
  initWebServer();
  scanWiFi();
}

int wifi_check() {
  if (WiFi.getMode() == WIFI_MODE_APSTA) {
    handleWiFiRequest();
    Serial.print(".");
    // 超过一定时间不配置就重启，可能wifi恢复了
    if (now() - ap_timestamp > 180) {
      ESP.restart();
    }
    // 在配网模式中
    return 1;
  }

  if (WiFi.status() == WL_CONNECTED) {
    // 已经连接成功
      return 2;
  } else {
      text("Try to connect WIFI!");
      if (connectToWiFi(10)) {
        first_conn = false;
        server_client.stop();

        Serial.println("wifi conn success!");
        text(WiFi.localIP().toString().c_str(), true);
        //text("test");

        // 连网接成功
        return 3;
      } else if (first_conn) {
        // 连接不成功   启动后就连接不成功才启动AP模式，运行中短线要不断尝试连接
        Serial.println("WIFI autoconnect fail, start AP for webconfig now...");
        text(String("Config:") + WiFi.softAPIP().toString().c_str(), true);

        // 转到网页端手动配置wifi
        start_ap_mode(); 
      }
  }

  // 没连接
  return 0;
}

bool connectToWiFi(int timeOut_s)
{
  //Serial.println("进入connectToWiFi()函数 ssid:" + wifi_ssid);
  // 设置为STA模式并连接WIFI
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true); // 设置自动连接
  // 用字符串成员函数c_str()生成一个const char*指针，指向以空字符终止的数组,即获取该字符串的指针。
  if (wifi_ssid != "")
  {
    Serial.println("用web配置信息连接.");

    WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
    wifi_ssid = "";
    wifi_pass = "";
  }
  else
  {
    Serial.println("用nvs保存的信息连接.");
    WiFi.begin(); // 连接上一次连接成功的wifi
  }
  // WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());
  int Connect_time = 0; // 用于连接计时，如果长时间连接不成功，复位设备
  while (WiFi.status() != WL_CONNECTED)
  { // 等待WIFI连接成功
    Serial.print(">");
    delay(500);
    Connect_time++;
    if (Connect_time > 4 * timeOut_s)
    { // 长时间连接不上，重新进入配网页面
      return false;       // 跳出 防止无限初始化
    }
  }
  Serial.println("");
  //Serial.println(" status:" + String(WiFi.status()));

  if (WiFi.status() == WL_CONNECTED)
  {
    //Serial.println(" succ status:" + String(WiFi.status()));
    return true;
  }
  return false;
}