// hello_aquestalk_tts - AquesTalk ESP32 漢字かな交じり文からの音声合成 for M5Stack
//         AquesTalkTTS.h/.cpp: AquesTalk-ESP32のラッパークラス
// 機能
// ・漢字テキストからの音声合成
// ・日付の音声合成
// ・シリアルポートからの任意の漢字テキストの音声合成
//
// あらかじめ、SDのaq_dicフォルダの下に辞書データファイル(aqdic_m.bin)を配置しておく

#include <M5Stack.h>
#include "AquesTalkTTS.h"

const char* licencekey = "XXX-XXX-XXX";	// AquesTalk-ESP32 licencekey

void setup()
{
  int iret;
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Hello AquesTalk TTS (KANJI demo)");
  M5.Lcd.println("");
  M5.Lcd.println("BtnA: sample message");
  M5.Lcd.println("BtnB: day 1-31");
  M5.Lcd.println("BtnC: stop");
  M5.Lcd.println("");
  M5.Lcd.println("Try sending any Kanji text(UTF8/CR) via the COM port.");
  
  iret = TTS.createK(licencekey);
  if(iret){
       Serial.print("ERR: TTS_createK():");
       Serial.println(iret);
  }
}

void loop()
{
	static bool bCountup=false;
	static int  mday=1;

  if (Serial.available()) {
    char kstr[256];
    size_t len = Serial.readBytesUntil('\r', kstr, 256);
    kstr[len]=0;
		int iret = TTS.playK(kstr, 100);
    if(iret){
      Serial.print("ERR:TTS.playK()=");
      Serial.println(iret);
    }
	}
	
	if(M5.BtnA.wasPressed()){
    TTS.playK("新しいライブラリは、漢字テキストから音声合成ができるようになりました。", 100);
	}
	else if(M5.BtnB.wasPressed()){
    bCountup = true;
	}
	else if(M5.BtnC.wasPressed()){
		TTS.stop();
    bCountup = false;
	}

	if(bCountup){
		if(TTS.isPlay()==false){
			char koe[64];
			sprintf(koe, "<NUMK VAL=%d COUNTER=nichi>.",mday);
			TTS.play(koe, 100);
			mday++;
			if(mday>31) mday=1;
		}
	}
	
  M5.update();
}
