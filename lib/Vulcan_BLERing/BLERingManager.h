#ifndef BLE_RING_MANAGER_H
#define BLE_RING_MANAGER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>

#include "RingState.h"
#include "BLERing.h"

#include "EMGSensor.h"
#include "BLEEMGSensor.h"

#define RING_SERVICE_UUID         "db1df223-4020-4c5a-930c-1989ea04991f"
#define SENSOR_SERVICE_UUID       "efedd9eb-24a9-492a-b66a-ed8543ee096e"

class BLERingManager: public BLEServerCallbacks {
  public:
    BLERingManager(RingState& pRingState, EMGSensor& pEMGSensor);
    ~BLERingManager();
    void begin();

    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param);
    void onDisconnect(BLEServer* pServer);
    void notifyData(uint8_t _state);
    uint8_t getModeReadSensor();
    uint8_t getModeRing();
    
  private: 
    double distance;
    RingState& ringState;
    BLERing* ringService;

    EMGSensor& emgSensor;
    BLEEMGSensor* emgSensorService;
  
    void setPower();
    void startAdvertising();
    void startServer();
};

#endif