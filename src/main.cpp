#include <Arduino.h>
#include "EyeApi.h"

static uint32_t lastMs = 0;

void setup() {
  Serial.begin(115200);
  delay(80);

  Serial.println("[BOOT] start");

  EyeApi::init();

  lastMs = millis();
}

void loop() {
  uint32_t now = millis();
  uint32_t dt  = now - lastMs;
  lastMs = now;

  EyeApi::update(dt);
  EyeApi::render();
}
