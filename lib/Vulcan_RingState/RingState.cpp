#include "RingState.h"

RingState::RingState(SPIClass &_spiNRF, RF &_rf, byte _txAddr[], byte _rxAddr[]):
  spiNRF(_spiNRF),
  rf(_rf) {
  memcpy((void*)txAddr, (void*)_txAddr, sizeof(txAddr));
  memcpy((void*)rxAddr, (void*)_rxAddr, sizeof(rxAddr));
}

void RingState::begin() {
  rf.begin(txAddr, rxAddr, &spiNRF, false);
  // Serial.printf("%s\n",txAddr);
  // Serial.printf("%s\n",rxAddr);
}

void RingState::sync(int8_t _stateControl) {
  static int8_t lastState = 0;
  static bool isFirstSaveStateControl = true;
  static uint32_t lastTime = millis();
  isFirstSaveStateControl = _stateControl == lastState ? false:true;
  lastState = _stateControl;
  if (isFirstSaveStateControl) {
    lastTime = millis();
  }
  // Serial.print(lastState);
  // Serial.print("\t");
  // Serial.print(_stateControl);
  // Serial.print("\t");
  // Serial.print(isFirstSaveStateControl);
  // Serial.print("\t");
  // Serial.printf("%d\t%d\n",lastState, _stateControl);
  if (millis() - lastTime > 3000) {
    sendControl(0);
  }
  else {
    sendControl(_stateControl);
  }
}

void RingState::sendControl(int8_t _stateControl) {
  // Serial.println(_stateControl);
  switch (_stateControl) {
    case 1:
      {
        rf.sendData(&data[1], 1);
        break;
      }
    case -1:
      {
        rf.sendData(&data[0], 1);
        break;
      }
    default:
      {
        rf.sendData(&data[2], 1);
        break;
      }
  }
}

void RingState::onConnect() {
}

void RingState::onDisconnect() {

}

uint8_t RingState::getMode() {
  return mode;
}

void RingState::setMode(uint8_t _mode) {
  mode = _mode;
}
