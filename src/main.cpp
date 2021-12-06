#include <iostream>
#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>
#include <tuple>
#include "configuration.h"

using namespace std;

configuration config;

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
  // delay(100);

  if(!wifi_init()){
    M5.Lcd.printf("Wifi connection timed out!\r\n");
    while(1);
  }
}

void loop() {
  HTTPClient http;
  http.begin(config.script_address);
  
  switch(http.GET()){
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
  delay(500);
}