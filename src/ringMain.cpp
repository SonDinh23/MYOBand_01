#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "BLERingManager.h"
#include "EMGSensor.h"
#include "RingState.h"

#define sampleRate    1000 //sample rate 1 KHz

byte txAddr[6] = "A0000";   // Sensor box address 20603
byte rxAddr[6] = "ZZZZZ";   // not use

#define SPI_CS        5         // SPI slave select
#define ADC_CLK       8000000  // SPI clock 1.6MHz

#define NRF_CS 			2   //CE pin NRF
#define NRF_CE 			15  //CS pin NRF

SPIClass spiMCP3208(VSPI);
SPIClass spiNRF(HSPI);
RF rf(NRF_CE, NRF_CS);
MCP3208 adc(ADC_VREF, SPI_CS, &spiMCP3208);

TaskHandle_t task_handle_signal;
TaskHandle_t task_handle_ble_signal;
TaskHandle_t task_handle_ring;
TaskHandle_t task_handle_mode;

EMGSensor emgSensor(adc);
RingState ringState(spiNRF, rf, txAddr, rxAddr);
BLERingManager ble(ringState, emgSensor);

uint8_t stateSensor = 0;
uint8_t mode = 0;
int8_t stateControl = 0;

void ReadSensorOP(void* pParamter) {
  for(;;) {
    stateControl = emgSensor.sync(1);    
    vTaskDelay(1); 
  }
}

void ReadSensorBLEOP(void* pParamter) {
  vTaskSuspend(NULL);
  for(;;) {
    ble.notifyData(emgSensor.readSensorBLE());     
    vTaskDelay(1); 
  }
}

void ModeOP(void* pParamter) {
  for(;;) {
    ringState.sync(stateControl);
    // Serial.println(stateControl);
    vTaskDelay(1);
  }
}

void RingOP(void* pParamter) {
  for(;;) {
    stateSensor = ble.getModeReadSensor();
    if (stateSensor == 0) {
      vTaskSuspend(task_handle_ble_signal);
      mode = ringState.getMode();
      vTaskResume(task_handle_mode);
      vTaskResume(task_handle_signal);
    }
    else {
      vTaskSuspend(task_handle_mode);
      vTaskSuspend(task_handle_signal);
      vTaskResume(task_handle_ble_signal);
    }
    vTaskDelay(10);
  }
}
uint16_t sensor[3];
void setup() {
  Serial.begin(115200); 
  pinMode(SPI_CS, OUTPUT);
  digitalWrite(SPI_CS, HIGH);

	SPISettings settings(ADC_CLK, MSBFIRST, SPI_MODE0);
  spiMCP3208.begin();
  spiMCP3208.beginTransaction(settings);
  ble.begin();
  ringState.begin();
  emgSensor.begin();
  
  xTaskCreatePinnedToCore(ReadSensorOP, "ReadSensorOP", 5000, NULL, 2, &task_handle_signal, 1);
  xTaskCreatePinnedToCore(ReadSensorBLEOP, "ReadSensorBLEOP", 5000, NULL, 2, &task_handle_ble_signal, 1);
  xTaskCreatePinnedToCore(ModeOP, "ModeOP", 1000, NULL, 1, &task_handle_mode, 1);
  xTaskCreatePinnedToCore(RingOP, "RingOP", 5000, NULL, 1, &task_handle_ring, 0);
}

void loop() {
  // sensor[0] = adc.read(MCP3208::Channel::SINGLE_0);
  // sensor[1] = adc.read(MCP3208::Channel::SINGLE_1);
  // sensor[2] = adc.read(MCP3208::Channel::SINGLE_2);

  // value[0] = ftLowSensor0.lowpass(sensor[0]);
  // value[1] = ftLowSensor1.lowpass(sensor[1]);
  // value[2] = ftLowSensor2.lowpass(sensor[2]);
//  Serial.printf("%d\t%d\t%d\n", sensor[0], sensor[1], sensor[2]);
  // Serial.printf("%.2f\t%.2f\t%.2f\n",sensor[0] - value[0], sensor[1] - value[1], sensor[2] - value[2]);
  // Serial.println(emgSensor.getStateControl());
  // vTaskDelay(100);
  vTaskDelete(NULL);
}