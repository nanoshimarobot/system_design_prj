#include <iostream>
#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>
#include <tuple>
#include "configuration.h"
#include <ArduinoJson.h>
#include <AquesTalkTTS.h>
#include <Avatar.h>
#include <tasks/LipSync.h>
#include <M5StackUpdater.h>
#include <Accessory.h>


using namespace std;
using namespace m5avatar;

Avatar avatar;
const char* AQUESTALK_KEY = "XXXX-XXXX-XXXX-XXXX";

configuration config;
char *payload = "{\"city\":\"Tokushima,JP\"}";

bool wifi_init(){
  WiFi.begin(config.wifi_ssid, config.wifi_pass);
  int cnt = 0;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    M5.Lcd.print(".");
    if(cnt++ > 10) return 0;
  }
  return 1;
}

void setup() {
  M5.begin(true, false, true, false);
  M5.Power.begin();
  Serial.begin(115200);
  int iret = TTS.create(AQUESTALK_KEY);
  M5.Lcd.setBrightness(30);
  M5.Lcd.clear();
  // avatar.init();
  // avatar.addTask(lipSync, "libSync");
  // delay(100);

  if(!wifi_init()){
    M5.Lcd.printf("Wifi connection timed out!\r\n");
    while(1);
  }
  
  HTTPClient http;
  http.begin(config.script_address);
  http.addHeader("Content-Type", "application/json");
  
  switch(http.POST(payload)){
    case HTTP_CODE_OK:
      M5.Lcd.printf("get success\n");
      M5.Lcd.println(http.getString());
      break;
    
    case HTTP_CODE_FOUND:
      http.end();
      http.begin(http.getLocation().c_str());
      http.GET();
      M5.Lcd.println(http.getString());
      break;
  }



  while(1);
}

void loop() {
  // M5.update();
  // TTS.play("aiueo",70);
  // avatar.setSpeechText("aiueo");

  /*HTTPClient http;
  http.begin(config.script_address);
  http.addHeader("Content-Type", "application/json");
  
  switch(http.POST(payload)){
    case HTTP_CODE_OK:
      M5.Lcd.printf("get success\n");
      M5.Lcd.println(http.getString());
      break;
    
    case HTTP_CODE_FOUND:
      http.end();
      http.begin(http.getLocation().c_str());
      http.GET();
      M5.Lcd.println(http.getString());
      break;
  }*/
  delay(10000);
  // avatar.setSpeechText("");
}