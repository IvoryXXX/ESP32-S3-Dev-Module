#include <Arduino.h>
#include "EyeApi.h"

static uint32_t gLastMs = 0;

void setup() {
  EyeApi::init();
  gLastMs = millis();
}

void loop() {
  const uint32_t now = millis();
  uint32_t dt = now - gLastMs;
  gLastMs = now;

  // dt clamp – ať se to nerozsype po připojení monitoru / breakpointech
  if (dt > 100) dt = 100;

  EyeApi::update(dt);
  EyeApi::render();

  // Patch 9: žádné pacing přes delay(5)
  // yield() je bezpečný „dech“ pro RTOS/USB; nedělá pevnou latenci
  yield();
}
