#include "LidsApi.h"

#include "EyeFrame.h"

// Patch 7: zatím jednoduchý blink scheduler (random-ish, bez RNG závislosti)
// - interval: 3500–7500 ms
// - délka mrknutí: 180 ms (close+open)
static uint32_t gNextBlinkAt = 0;
static uint32_t gBlinkStart = 0;
static bool gBlinking = false;

// 0..1000
static uint16_t gLidPos = 0; // 0=open, 1000=closed

// jednoduchý “pseudo” rozptyl bez rand(): posouváme interval podle času
static uint32_t nextInterval(uint32_t nowMs) {
  // rozptyl 3500..7500 podle “hash” času
  uint32_t x = nowMs;
  x ^= x << 13; x ^= x >> 17; x ^= x << 5;
  return 3500u + (x % 4001u);
}

namespace LidsApi {

void init() {
  gBlinking = false;
  gLidPos = 0;
  gBlinkStart = 0;
  gNextBlinkAt = 0; // naplánuje se při prvním update
}

static uint16_t clamp1000(int v) {
  if (v < 0) return 0;
  if (v > 1000) return 1000;
  return (uint16_t)v;
}

void triggerBlink(uint32_t nowMs) {
  if (gBlinking) return;
  gBlinking = true;
  gBlinkStart = nowMs;
}

bool isBlinking() { return gBlinking; }

void update(uint32_t nowMs, EyeFrame& ioFrame) {
  if (gNextBlinkAt == 0) {
    gNextBlinkAt = nowMs + nextInterval(nowMs);
  }

  // auto trigger
  if (!gBlinking && nowMs >= gNextBlinkAt) {
    triggerBlink(nowMs);
  }

  const uint16_t prev = gLidPos;

  if (gBlinking) {
    // blink duration 180ms: 0..90 close, 90..180 open
    const uint32_t t = nowMs - gBlinkStart;
    if (t >= 180) {
      gBlinking = false;
      gLidPos = 0;
      gNextBlinkAt = nowMs + nextInterval(nowMs);
    } else if (t <= 90) {
      // closing 0..90 -> 0..1000
      gLidPos = clamp1000((int)( (t * 1000u) / 90u ));
    } else {
      // opening 90..180 -> 1000..0
      const uint32_t u = t - 90u;
      gLidPos = clamp1000(1000 - (int)((u * 1000u) / 90u));
    }
  } else {
    gLidPos = 0;
  }

  // aplikuj do frame (top i bot zatím symetricky)
  ioFrame.lidTop = gLidPos;
  ioFrame.lidBot = gLidPos;

  if (gLidPos != prev) {
    ioFrame.lidsDirty = true;
  }
}

} // namespace LidsApi
