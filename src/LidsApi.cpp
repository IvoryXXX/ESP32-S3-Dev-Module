#include "LidsApi.h"
#include "EyeFrame.h"

// jednoduché Blink v1 (symetricky top+bot)
static const uint32_t BLINK_DURATION_MS = 180;
static const uint32_t BLINK_HALF_MS     = 90;
static const uint32_t BLINK_MIN_WAIT_MS = 3500;
static const uint32_t BLINK_MAX_WAIT_MS = 7500;

static bool gBlinking = false;
static uint32_t gBlinkT = 0;
static uint32_t gNextBlinkIn = 0;

static uint16_t gPos = 0; // 0..1000

static uint32_t gRng = 0xC0FFEEu;
static uint32_t rngNext() {
  gRng = gRng * 1664525u + 1013904223u;
  return gRng;
}
static uint32_t nextIntervalMs() {
  const uint32_t span = (BLINK_MAX_WAIT_MS - BLINK_MIN_WAIT_MS) + 1u;
  return BLINK_MIN_WAIT_MS + (rngNext() % span);
}
static uint16_t clamp1000(int v) {
  if (v < 0) return 0;
  if (v > 1000) return 1000;
  return (uint16_t)v;
}

namespace LidsApi {

void init() {
  gBlinking = false;
  gBlinkT = 0;
  gPos = 0;
  gNextBlinkIn = nextIntervalMs();
}

void triggerBlink() {
  if (gBlinking) return;
  gBlinking = true;
  gBlinkT = 0;
}

bool isBlinking() { return gBlinking; }

void update(uint32_t dtMs, EyeFrame& ioFrame) {
  const uint16_t prev = gPos;

  if (!gBlinking) {
    if (dtMs >= gNextBlinkIn) {
      gNextBlinkIn = 0;
      triggerBlink();
    } else {
      gNextBlinkIn -= dtMs;
    }
  }

  if (gBlinking) {
    gBlinkT += dtMs;

    if (gBlinkT >= BLINK_DURATION_MS) {
      gBlinking = false;
      gBlinkT = 0;
      gPos = 0;
      gNextBlinkIn = nextIntervalMs();
    } else if (gBlinkT <= BLINK_HALF_MS) {
      gPos = clamp1000((int)((gBlinkT * 1000u) / BLINK_HALF_MS));
    } else {
      const uint32_t u = gBlinkT - BLINK_HALF_MS;
      gPos = clamp1000(1000 - (int)((u * 1000u) / BLINK_HALF_MS));
    }
  } else {
    gPos = 0;
  }

  // do frame
  ioFrame.lidTop = gPos;
  ioFrame.lidBot = gPos;

  if (gPos != prev) ioFrame.lidsDirty = true;
}

} // namespace LidsApi
