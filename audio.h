/*
*******************************************************************************
* Copyright (c) 2021 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit the website for more information：https://docs.m5stack.com/en/core/core2
* 获取更多资料请访问：https://docs.m5stack.com/zh_CN/core/core2
*
* describe：NS4168--I2S power amplifier.  功放示例
* date：2021/7/21
*******************************************************************************
*/
#include <M5Core2.h>
#include <driver/i2s.h>
#include <EEPROM.h>

#ifndef EEPROM_SIZE
    #define EEPROM_SIZE 8
#endif

#define CONFIG_I2S_BCK_PIN 12 //Define I2S related ports.  定义I2S相关端口
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0  // Define the speaker port.  定义扬声器端口

#define MODE_MIC 0  // Define the working mode.  定义工作模式
#define MODE_SPK 1
#define DATA_SIZE 1024


bool InitI2SSpeakOrMic(int mode){  //Init I2S.  初始化I2S
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); // Uninstall the I2S driver.  卸载I2S驱动
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),  // Set the I2S operating mode.  设置I2S工作模式
        .sample_rate = 44100, // Set the I2S sampling rate.  设置I2S采样率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Fixed 12-bit stereo MSB.  固定为12位立体声MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Set the channel format.  设置频道格式
        .communication_format = I2S_COMM_FORMAT_I2S,  // Set the format of the communication.  设置通讯格式
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Set the interrupt flag.  设置中断的标志
        .dma_buf_count = 2, //DMA buffer count.  DMA缓冲区计数
        .dma_buf_len = 128, //DMA buffer length.  DMA缓冲区长度
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  //I2S clock setup.  I2S时钟设置
        i2s_config.tx_desc_auto_clear = true; // Enables auto-cleanup descriptors for understreams.  开启欠流自动清除描述符
    }
    // Install and drive I2S.  安装并驱动I2S
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  // Link the BCK to the CONFIG_I2S_BCK_PIN pin. 将BCK链接至CONFIG_I2S_BCK_PIN引脚
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  //          ...
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;  //       ...
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; //      ...
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); // Set the I2S pin number.  设置I2S引脚编号
    err += i2s_set_clk(Speak_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set the clock and bitwidth used by I2S Rx and Tx. 设置I2S RX、Tx使用的时钟和位宽
    return true;
}


/* After M5Core2 is started or reset
the program in the setUp () function will be run, and this part will only be run once. */

void setup_audio(){
  Serial.printf("Starting Speaker...\n");
  M5.Axp.SetSpkEnable(true);  //Enable speaker power.  启用扬声器电源
  delay(100); //delay 100ms.  延迟100ms

  if (!EEPROM.begin(EEPROM_SIZE)){  //Request storage of SIZE size(success return 1).  申请SIZE大小的存储(成功返回1)
     Serial.println("\nFailed to initialise EEPROM!"); //串口输出格式化字符串.  Serial output format string
     delay(1000);
  }


}


size_t record(uint8_t* data_0){
    Serial.printf("Recording sounds.\n");
    
    size_t data_offset = 0;
    InitI2SSpeakOrMic(MODE_MIC);
    size_t byte_read;
    while (1){
        i2s_read(Speak_I2S_NUMBER, (char *)(data_0 + data_offset), DATA_SIZE, &byte_read, (100 / portTICK_RATE_MS));
        data_offset += byte_read;
        if(data_offset >= DATA_SIZE * 100)
            break;
    }
    Serial.printf("End recording %i bytes.\n", data_offset);

    return data_offset;

}

size_t play(uint8_t* data_0, size_t data_size){
    size_t bytes_written = 0;
    InitI2SSpeakOrMic(MODE_SPK);
    i2s_write(Speak_I2S_NUMBER, data_0, data_size, &bytes_written, portMAX_DELAY);
    Serial.printf("End playing %i bytes.\n", bytes_written);

    return bytes_written;
}

void vibrate(unsigned int time){
    M5.Axp.SetLDOEnable(3,true);   //Open the vibration.   开启震动马达
    delay(time);
    M5.Axp.SetLDOEnable(3,false);
}


void save_audio(size_t eeprom_address, uint8_t* data_0, size_t file_size){
    for (uint8_t i = 0;i<4;i++ )
        EEPROM.write(eeprom_address+i, (uint8_t) ((file_size>>(i*8))&0x00FF));

    for(size_t i = 0; i < file_size; i++){
        int addr = eeprom_address+i+4;
        EEPROM.write(addr, (uint8_t) data_0[i]);
        uint8_t val = EEPROM.read(addr);
        if (i< 50)
            Serial.printf("WRITE ADDR: %i  - Value:  %i  \n",addr, val);
        //Serial.print(data_0[i]);
    }
}

size_t read_audio(size_t eeprom_address, uint8_t* data_0){
    size_t file_size = 0;
    for (uint8_t i = 0;i<4;i++ )
         file_size += ((size_t) EEPROM.read(eeprom_address+i))<<(i*8);

    Serial.printf("Reading %i bytes from the EEPROM.\n", file_size);

    for(size_t i = 0; i < file_size; i++){
        int addr = eeprom_address+i+4;
        uint8_t val = EEPROM.read(addr);
        if (i< 50)
            Serial.printf("Address: %i  - Value:  %i  \n",addr, val);
        data_0[i] = val;
    }

    
    return file_size;
}
