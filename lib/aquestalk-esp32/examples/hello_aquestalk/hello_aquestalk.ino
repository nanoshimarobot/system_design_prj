// hello_aquestalk.ino - AquesTalk ESP32 サンプルプログラム
#include "driver/i2s.h"
#include "aquestalk.h"

#define LEN_FRAME 32
uint32_t workbuf[AQ_SIZE_WORKBUF];

void setup() {
	int iret;
	Serial.begin(115200);

	Serial.println("Initialize AquesTalk");
	iret = CAqTkPicoF_Init(workbuf, LEN_FRAME, "XXX-XXX-XXX");
	if(iret){
		Serial.println("ERR:CAqTkPicoF_Init");
	}

	DAC_Create();
	Serial.println("D/A start");
	
	playAquesTalk("konnnichiwa.");
	playAquesTalk("korewa;te'_sutode_su.");
	playAquesTalk("sa'nngatsu/<NUMK VAL=17 COUNTER=nichi> <NUMK VAL=12 COUNTER=ji>/<NUMK VAL=23 COUNTER=funn>.");
	playAquesTalk("yukkuri_siteittene?");

	DAC_Release();
	Serial.println("D/A stop");
}

void loop() {
}

// 一文の音声出力（同期型）
void playAquesTalk(const char *koe)
{
  Serial.print("Play:");
  Serial.println(koe);

	int iret = CAqTkPicoF_SetKoe((const uint8_t*)koe, 100, 0xffffU);
	if(iret)	Serial.println("ERR:CAqTkPicoF_SetKoe");

	for(;;){
		int16_t wav[LEN_FRAME];
		uint16_t len;
		iret = CAqTkPicoF_SyntheFrame(wav, &len);
		if(iret) break; // EOD
		
		DAC_Write((int)len, wav);
	}
}

////////////////////////////////
//i2s configuration 
static const int i2s_num = 0; // i2s port number
i2s_config_t i2s_config = {
		 .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
     .sample_rate = 24000,
		 .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		 .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
		 .intr_alloc_flags = 0,
		 .dma_buf_count = 4,
		 .dma_buf_len = 384,
		 .use_apll = 0
};

void DAC_Create()
{
	AqResample_Reset();

	i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);

  i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);  // 25, 26
  //i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); // 25
  //i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN ); // 26
}

void DAC_Release()
{
	i2s_driver_uninstall((i2s_port_t)i2s_num); //stop & destroy i2s driver 
}

// upsampling & write to I2S
int DAC_Write(int len, int16_t *wav)
{
	int i;
	for(i=0;i<len;i++){
		// upsampling 8KHz -> 24KHz
		int16_t wav3[3];
		AqResample_Conv(wav[i], wav3);

		// write to I2S DMA buffer
    for(int k=0;k<3; k++){
      uint16_t us = ((uint16_t)wav3[k])^0x8000U;  // signed -> unsigned data  for M5Stack internal DAC
//      uint16_t us = (uint16_t)wav3[k];          // for external I2S DAC
			uint16_t sample[2];
      size_t transBytes;
			sample[0]=sample[1]=us; // mono -> stereo
      i2s_write((i2s_port_t)i2s_num, (const char*)sample, 4, &transBytes, portMAX_DELAY);
		}
	}
	return i;
}
