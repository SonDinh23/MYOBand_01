#include "BLERingManager.h"

BLERingManager::BLERingManager(RingState& pRingState, EMGSensor& pEMGSensor)
  :ringState(pRingState)
  ,emgSensor(pEMGSensor){
}

void BLERingManager::onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {
  Serial.println("Connected");

  // Update connection configuration with peer
  // latency = 0; min_int = 0x10; max_int = 0x20; timeout = 10s;
  pServer->updateConnParams(param->connect.remote_bda, 0x10, 0x20, 0, 1000);

  // The longer distance, the more time it takes to smooth,
  // which might break the authentication from app
  distance = 40; // meter
  ringState.onConnect();
}

void BLERingManager::onDisconnect(BLEServer* pServer) {
  Serial.println("Disconnected");
  ringState.onDisconnect();

  BLEDevice::startAdvertising();
}

void BLERingManager::setPower() {
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_P9);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_P9);
}

void BLERingManager::startAdvertising() {
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(RING_SERVICE_UUID);
  advertising->addServiceUUID(SENSOR_SERVICE_UUID);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);  // for iPhone connections issue
  advertising->setMaxPreferred(0x12);

  BLEAdvertisementData data;
  data.setName(ringService->getName());
  //data.setManufacturerData(handService->getFrameNumber());
  advertising->setScanResponseData(data);

  BLEDevice::startAdvertising();
}

void BLERingManager::startServer() {
  // Init without device name, which will be set later 
  // accordingly to handService.getName()
  BLEDevice::init("");

  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(this);

  ringService = new BLERing(server->createService(RING_SERVICE_UUID), ringState);
  emgSensorService = new BLEEMGSensor(server->createService(SENSOR_SERVICE_UUID), emgSensor);

  ringService->begin();
  emgSensorService->begin();

  // Restore original name after firmware upgrade
  esp_ble_gap_set_device_name(ringService->getName());

  Serial.println("BLE started");
}

void BLERingManager::begin() {
  startServer();
  setPower();
  startAdvertising();
}

void BLERingManager::notifyData(uint8_t _state) {
  // if ((_state != 1) && (_state != 3)) return;
  emgSensorService->onNotifySignal(_state);
}

uint8_t BLERingManager::getModeReadSensor() {
  return emgSensorService->getModeReadSensor();
}

uint8_t BLERingManager::getModeRing() {
  return ringState.getMode();
}

BLERingManager::~BLERingManager() {
  free(ringService);
  free(emgSensorService);
}