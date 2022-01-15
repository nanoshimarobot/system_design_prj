// hello_aquestalk_m5stack.ino - AquesTalk ESP32 サンプルプログラム for M5Stack
#include "driver/i2s.h"
#include "aquestalk.h"
#include "M5Stack.h"

#define LEN_FRAME 32
uint32_t workbuf[AQ_SIZE_WORKBUF];

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.println("Hello AquesTalk M5Stack");
  M5.Lcd.println();
  M5.Lcd.println("BtnA:Kon nichiwa");
  M5.Lcd.println("BtnB:Kore ha test desu");
  M5.Lcd.println("BtnC:Yukkuri shite itte ne");

  Serial.println("Initialize AquesTalk");
  int iret = CAqTkPicoF_Init(workbuf, LEN_FRAME, "XXX-XXX-XXX");
  if (iret) {
    Serial.println("ERR:CAqTkPicoF_Init");
  }

  DAC_create();
  Serial.println("D/A start");

  playAquesTalk("akue_suto'-_ku/kido-shima'_shita.");
  playAquesTalk("botanno/o_shitekudasa'i.");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasReleased()) {
    // BtnA
    playAquesTalk("konnnichiwa.");
  } else if (M5.BtnB.wasReleased()) {
    // BtnB
    playAquesTalk("korewa;te'_sutode_su.");
  } else if (M5.BtnC.wasReleased()) {
    // BtnC
    playAquesTalk("yukkuri_siteittene?");
  }
}

// 一文の音声出力（同期型）
void playAquesTalk(const char *koe)
{
  Serial.print("Play:");
  Serial.println(koe);

	int iret = CAqTkPicoF_SetKoe((const uint8_t*)koe, 100, 0xffffU);
	if(iret)	Serial.println("ERR:CAqTkPicoF_SetKoe");

  DAC_start();
  for(;;){
		int16_t wav[LEN_FRAME];
		uint16_t len;
		iret = CAqTkPicoF_SyntheFrame(wav, &len);
		if(iret) break; // EOD
		
		DAC_write((int)len, wav);
	}
  DAC_stop();
}

////////////////////////////////
//  i2s configuration 
#define  SAMPLING_FREQ   8000 // 8KHz
#define DMA_BUF_COUNT 3
#define DMA_BUF_LEN  LEN_FRAME // one buffer size(one channnel samples)
#define DMA_BUF_SIZE (DMA_BUF_COUNT*DMA_BUF_LEN)
#define I2S_FIFO_LEN  (64/2)
#define  TICKS_TO_WAIT (2*LEN_FRAME/8/portTICK_PERIOD_MS)

//i2s configuration 
static const int i2s_num = 0; // i2s port number
static i2s_config_t i2s_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN),
     .sample_rate = SAMPLING_FREQ,
     .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S_MSB,
     .intr_alloc_flags = 0,
     .dma_buf_count = DMA_BUF_COUNT,
     .dma_buf_len = DMA_BUF_LEN,
     .use_apll = 0
};

static void DAC_create()
{
  i2s_driver_install((i2s_port_t)i2s_num, &i2s_config, 0, NULL);
  i2s_set_pin((i2s_port_t)i2s_num, NULL);
  i2s_stop((i2s_port_t)i2s_num);  // Create時はstop状態
}

static void DAC_release()
{
  i2s_driver_uninstall((i2s_port_t)i2s_num); //stop & destroy i2s driver 
}

static void DAC_start()
{
  i2s_start((i2s_port_t)i2s_num);
  
  // reduce pop noize
  int k;
  for(k=0;k<DMA_BUF_LEN;k++){
    DAC_write_val(0);
  }
  for(k=0;k<=32768;k+=256*3) {
    DAC_write_val((uint16_t)k);
  }
}

static void DAC_stop()
{
  // reduce pop noize
  int k;
  for(k=32768;k>=0;k-=256*3) {
    DAC_write_val((uint16_t)k);
  }
  for(k=0;k<DMA_BUF_SIZE+I2S_FIFO_LEN;k++){
    DAC_write_val(0);
  }
  i2s_stop((i2s_port_t)i2s_num);  
}

// upsampling & write to I2S
static int DAC_write(int len, int16_t *wav)
{
  int i;
  for(i=0;i<len;i++){
      int iret = DAC_write_val((wav[i])^0x8000U);// for Internal-DAC (signed -> unsigned data)
//    int iret = DAC_write_val(wav[i]);// for External SPI-DAC
      if(iret<0) return 404; // -1:parameter error
      if(iret==0) break;  //  0:TIMEOUT
  }
  return i;
}

// write to I2S DMA buffer
static int DAC_write_val(uint16_t val)
{
  uint16_t sample[2];
  sample[0]=sample[1]=val; // mono -> stereo
  size_t bytes_written;
  esp_err_t iret = i2s_write((i2s_port_t)i2s_num, sample, sizeof(uint16_t)*2, &bytes_written, TICKS_TO_WAIT);
  if(iret!=ESP_OK) return -1;
  if(bytes_written<sizeof(uint16_t)*2) return 0; // timeout
  return 1;
}
