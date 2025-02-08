// Remote controller.
// Operation: ESP-NOW wireless controller for the led panel
// when the encoder and buttons haven't been touched for a while, shuts off the ESP-NOW connection and goes into deep sleep until a button is pressed
// remote and panel are PERMANENTLY PAIRED!
#include "WiFi.h"
#include <esp_now.h>
#include <U8g2lib.h>
#include "ui.hpp"
#include "hmi.hpp"

// CONFIGURABLES

const int PANEL_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const int TIMEOUT = 30 * 1000; // in ms. if 30 seconds have passed without updates, we'll go into deep sleep
const int BUTTON_UPPER = D9;
const int BUTTON_LOWER = D10;
const int ENCODER_A = D8;
const int ENCODER_B = D7;

// END CONFIGURABLES

struct LedPanel {
  uint8_t brightness;
  uint8_t warmth;
};

LedPanel frontPanel, backPanel;


struct ControlFrame { // this is the C++ equivalent of
/*
  enum ControlFrame {
    SetLights(LedPanel, LedPanel),
    Settimer(u16)
  }
  rust, oh rust, wherefore art thou rust...
*/
  enum {
    SetTimer,
    SetLights
  } operation;
  union {
    struct {
      LedPanel front, back;
    };
    uint16_t timerDuration;
  };
};


void sentCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // skeleton, for now
}


void onReceive(const esp_now_recv_info* info, const uint8_t* data, int len) {
  // ditto
}


void setup_espnow() {
  WiFi.mode(WIFI_STA); // set up the wifi chip for esp-now
  WiFi.disconnect();
  while (!WiFi.STA.started()) {
    Serial.println("waiting on wifi station start");
    delay(100);
  }

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed.");
  }

  esp_now_peer_info info = {};
  info.channel = 0;
  info.encrypt = false;
  memcpy(info.peer_addr, PANEL_MAC, 6);
  if (esp_now_add_peer(&info) != ESP_OK) {
    Serial.println("Couldn't pair with receiver");
  }

  esp_now_register_recv_cb(&onReceive);
  esp_now_register_send_cb(&sentCallback);
}


struct {
  LedPanel front, back;
  bool frontEnabled = true;
  bool backEnabled = false;
} state;



PullupButton button1(BUTTON_UPPER);
PullupButton button2(BUTTON_LOWER);
Encoder encoder(ENCODER_A, ENCODER_B);


void setup() {
  // put your setup code here, to run once:
  Serial.println("setup 0"); // debugging The Hard Way ™
  //setup_espnow(); // set up the wifi adapter (this has to be called every time it wakes from deep sleep)
  // TODO: cycle down cpu frequency
  UI ui {
    [](bool v){}, // front toggle
    [](bool v){}, // back toggle
    [](int v){}, // back brightness
    [](int v){}, // back warmth
    [](int v){}, // front brightness
    [](int v){}, // front warmth
  };
  ui.render();
  Serial.println("setup 1"); // debugging The Hard Way ™
}

void loop() {
  // put your main code here, to run repeatedly:
  /*auto delta = encoder.poll();
  button1.poll();
  button2.poll();
  if (delta) {
    ui.encoderTurn(delta);
  }
  if (button1.wasReleasedSLC()) {
    ui.button1();
  }
  if (button2.wasReleasedSLC()) {
    ui.button2();
  }*/
}
