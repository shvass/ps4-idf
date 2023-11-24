#include "PS4Controller.h"


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>

#include <cstring>
#include <esp_log.h>

extern "C" {
#include "ps4.h"
}

#define ESP_BD_ADDR_STR "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx"


#define ESP_BD_ADDR_HEX_PTR(addr) \
  (uint8_t*)addr + 0, (uint8_t*)addr + 1, (uint8_t*)addr + 2, \
  (uint8_t*)addr + 3, (uint8_t*)addr + 4, (uint8_t*)addr + 5

PS4Controller::PS4Controller() {}


bool isPS4btInited = false;
bool PS4Controller::begin() {
  ps4SetEventObjectCallback(this, &PS4Controller::_event_callback);
  ps4SetConnectionObjectCallback(this, &PS4Controller::_connection_callback);


  if(!isPS4btInited) { ps4Init(); isPS4btInited = true; }
  return true;
}

bool PS4Controller::begin(const char* mac) {
  esp_bd_addr_t addr;
    
  if (sscanf(mac, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX_PTR(addr)) != ESP_BD_ADDR_LEN) {
    ESP_LOGE("PS4", "Could not convert %s\n to a MAC address", mac);
    return false;
  }

  ps4SetBluetoothMacAddress(addr);

  return begin();
}

void PS4Controller::end() {}

bool PS4Controller::isConnected() { return ps4IsConnected(); }

void PS4Controller::setLed(uint8_t r, uint8_t g, uint8_t b) {
  output.r = r;
  output.g = g;
  output.b = b;
}

void PS4Controller::setRumble(uint8_t small, uint8_t large) {
  output.smallRumble = small;
  output.largeRumble = large;
}

void PS4Controller::setFlashRate(uint8_t onTime, uint8_t offTime) {
  output.flashOn = onTime / 10;
  output.flashOff = offTime / 10;
}

void PS4Controller::sendToController() { ps4SetOutput(output); }

void PS4Controller::attach(callback_t callback) { _callback_event = callback; }

void PS4Controller::attachOnConnect(callback_t callback) {
  _callback_connect = callback;
}

void PS4Controller::attachOnDisconnect(callback_t callback) {
  _callback_disconnect = callback;
}

void PS4Controller::_event_callback(
  void* object, ps4_t data, ps4_event_t event) {
  PS4Controller* This = (PS4Controller*)object;

  memcpy(&This->data, &data, sizeof(ps4_t));
  memcpy(&This->event, &event, sizeof(ps4_event_t));

  if (This->_callback_event) {
    This->_callback_event();
  }
}

void PS4Controller::_connection_callback(void* object, uint8_t isConnected) {
  PS4Controller* This = (PS4Controller*)object;

  if (isConnected) {
    vTaskDelay(pdMS_TO_TICKS(150));
    // ToDo: figure out how to know when the channel is free again
                 // so this delay can be removed

    if (This->_callback_connect) {
      This->_callback_connect();
    }
  }
  else {
    if (This->_callback_disconnect) {
      This->_callback_disconnect();
    }
  }
}

#if !defined(NO_GLOBAL_INSTANCES)
PS4Controller PS4;
#endif
