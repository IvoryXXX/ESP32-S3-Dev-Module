#include "eye_pupil.h"

static bool     s_enabled = true;
static uint16_t s_col565  = 0x0000;

static int s_basePx = 35;
static int s_minPx  = 15;
static int s_maxPx  = 50;

static bool     s_animEnabled = true;
static uint32_t s_changeMinMs = 220;
static uint32_t s_changeMaxMs = 520;
static uint32_t s_holdMinMs   = 260;
static uint32_t s_holdMaxMs   = 900;
static uint32_t s_arriveExtraAfterMs = 1000;

static int s_stepDeltaPx = 1;

// runtime
static int s_curPx = 35;
static int s_fromPx = 35;
static int s_toPx = 35;

static uint32_t s_phaseStartMs = 0;
static uint32_t s_phaseEndMs   = 0;
static uint32_t s_nextPickMs   = 0;

// “arrive event” plánování 2. změny během dwell
static bool     s_waitExtra = false;
static uint32_t s_extraAtMs = 0;

static uint32_t randRangeU32(uint32_t a, uint32_t b) {
  if (b <= a) return a;
  return a + (uint32_t)random((long)(b - a + 1));
}

static int clampi(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static int lerpInt(int a, int b, float t) {
  return (int)lroundf((float)a + ((float)b - (float)a) * t);
}

static void pickNewTarget(uint32_t nowMs) {
  s_fromPx = s_curPx;
  s_toPx   = clampi((int)random(s_minPx, s_maxPx + 1), s_minPx, s_maxPx);

  uint32_t dur = randRangeU32(s_changeMinMs, s_changeMaxMs);
  s_phaseStartMs = nowMs;
  s_phaseEndMs   = nowMs + dur;

  uint32_t hold = randRangeU32(s_holdMinMs, s_holdMaxMs);
  s_nextPickMs   = s_phaseEndMs + hold;
}

void EyePupil::init(const Config& cfg) {
  s_enabled = cfg.pupilEnabled;
  s_col565  = cfg.pupilColor565;

  s_basePx = cfg.pupilBasePx;
  s_minPx  = cfg.pupilMinPx;
  s_maxPx  = cfg.pupilMaxPx;

  s_animEnabled = cfg.pupilAnimEnabled;

  s_changeMinMs = cfg.pupilChangeMsMin;
  s_changeMaxMs = cfg.pupilChangeMsMax;
  s_holdMinMs   = cfg.pupilHoldMsMin;
  s_holdMaxMs   = cfg.pupilHoldMsMax;

  s_arriveExtraAfterMs = cfg.pupilArriveExtraAfterMs;
  s_stepDeltaPx = cfg.pupilStepDeltaPx;

  s_curPx = clampi(s_basePx, s_minPx, s_maxPx);
  s_fromPx = s_curPx;
  s_toPx   = s_curPx;

  uint32_t now = millis();
  s_phaseStartMs = now;
  s_phaseEndMs   = now;
  s_nextPickMs   = now;

  s_waitExtra = false;
  s_extraAtMs = 0;
}

bool EyePupil::update(uint32_t nowMs, bool irisMoved, bool arrivedThisTick, uint32_t newDwellMsIfArrived) {
  if (!s_enabled) return false;

  bool changed = false;

  // 1) když iris dorazil do cíle: vyber nový target a naplánuj “extra změnu”
  if (arrivedThisTick) {
    if (s_animEnabled) {
      pickNewTarget(nowMs);
      changed = true;

      // extra změna po X ms jen když dwell je dost dlouhý
      if (newDwellMsIfArrived >= (s_arriveExtraAfterMs + 50)) {
        s_waitExtra = true;
        s_extraAtMs = nowMs + s_arriveExtraAfterMs;
      } else {
        s_waitExtra = false;
      }
    }
  }

  // 2) extra změna během dwell
  if (s_waitExtra && (int32_t)(nowMs - s_extraAtMs) >= 0) {
    s_waitExtra = false;
    if (s_animEnabled) {
      pickNewTarget(nowMs);
      changed = true;
    }
  }

  // 3) časová animace (plynulá)
  if (s_animEnabled) {
    if (s_phaseEndMs > s_phaseStartMs && (int32_t)(nowMs - s_phaseStartMs) >= 0) {
      float t = 0.0f;
      if (nowMs >= s_phaseEndMs) t = 1.0f;
      else t = (float)(nowMs - s_phaseStartMs) / (float)(s_phaseEndMs - s_phaseStartMs);

      int newPx = lerpInt(s_fromPx, s_toPx, t);
      newPx = clampi(newPx, s_minPx, s_maxPx);

      if (newPx != s_curPx) {
        s_curPx = newPx;
        changed = true;
      }
    }

    // po doběhnutí a holdu vyber další (náhodně)
    if ((int32_t)(nowMs - s_nextPickMs) >= 0 && !irisMoved) {
      // během pohybu to necháme být – chceme změny hlavně při ARRIVE / během dwellu
      pickNewTarget(nowMs);
      changed = true;
    }
  }

  // 4) jemný mikrokrok na “step” (aby změny při pohybu byly malé)
  if (irisMoved && s_stepDeltaPx > 0) {
    int diff = s_toPx - s_curPx;
    if (diff != 0) {
      int step = clampi(diff, -s_stepDeltaPx, s_stepDeltaPx);
      int newPx = clampi(s_curPx + step, s_minPx, s_maxPx);
      if (newPx != s_curPx) {
        s_curPx = newPx;
        changed = true;
      }
    }
  }

  return changed;
}

static void drawFilledCircle565(uint16_t* buf, int w, int h, int cx, int cy, int r, uint16_t col) {
  if (!buf || w <= 0 || h <= 0 || r <= 0) return;

  int r2 = r * r;
  int y0 = max(0, cy - r);
  int y1 = min(h - 1, cy + r);
  int x0 = max(0, cx - r);
  int x1 = min(w - 1, cx + r);

  for (int y = y0; y <= y1; y++) {
    int dy = y - cy;
    int dy2 = dy * dy;
    for (int x = x0; x <= x1; x++) {
      int dx = x - cx;
      if (dx*dx + dy2 <= r2) {
        buf[(size_t)y * (size_t)w + (size_t)x] = col;
      }
    }
  }
}

void EyePupil::drawIntoPatch(uint16_t* patch, int w, int h) {
  if (!s_enabled || !patch) return;

  // pupil je “součást irisu” -> centrovaně v iris patch
  int cx = w / 2;
  int cy = h / 2;

  // s_curPx je průměr v px; radius = průměr/2
  int r = max(1, s_curPx / 2);

  drawFilledCircle565(patch, w, h, cx, cy, r, s_col565);
}

int EyePupil::currentPx() { return s_curPx; }
