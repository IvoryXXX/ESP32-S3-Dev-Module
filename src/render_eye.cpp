#include "render_eye.h"

// DŮLEŽITÉ: render může sahat na TftManager jen přes _render_internal.h,
// který definuje RENDERER_CAN_DRAW.
#include "_render_internal.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

static EyeRenderConfig gCfg;

// buffery
static uint16_t* baseBuf = nullptr;
static uint16_t* irisBuf = nullptr;
static uint16_t* topOpenBuf = nullptr;
static uint16_t* botOpenBuf = nullptr;

// dočasný patch pro iris kompozici
static uint16_t* irisPatch = nullptr;

static inline void freeIf(void*& p) {
  if (p) { free(p); p = nullptr; }
}

// ---- RAW565 loader z SD (Patch 9 styl) ----
static bool loadRaw565(const char* path, uint16_t* dst, unsigned int pixelCount) {
  if (!path || !dst || pixelCount == 0) return false;

  File f = SD.open(path, "r");
  if (!f) {
    Serial.printf("[RAW] open FAIL: %s\n", path);
    return false;
  }

  const size_t bytes = (size_t)pixelCount * 2;
  size_t got = f.read((uint8_t*)dst, bytes);
  f.close();

  if (got != bytes) {
    Serial.printf("[RAW] read FAIL %s got=%u need=%u\n", path, (unsigned)got, (unsigned)bytes);
    return false;
  }
  return true;
}

// otevřená víčka overlay (Patch 9 logika)
static void overlayOpenLidsIfAny(const SkinAssets& skin) {
  if (topOpenBuf && skin.topOpen.w > 0 && skin.topOpen.h > 0) {
    TftManager::pushRGB565(skin.topOpen.x(), skin.topOpen.y(),
                           skin.topOpen.w, skin.topOpen.h,
                           topOpenBuf, gCfg.swapBytes);
  }
  if (botOpenBuf && skin.botOpen.w > 0 && skin.botOpen.h > 0) {
    TftManager::pushRGB565(skin.botOpen.x(), skin.botOpen.y(),
                           skin.botOpen.w, skin.botOpen.h,
                           botOpenBuf, gCfg.swapBytes);
  }
}

void eyeRenderInit(const EyeRenderConfig& cfg) {
  gCfg = cfg;
}

bool eyeRenderLoadAssets(const SkinAssets& skin) {
  // uvolni případné staré
  freeIf((void*&)baseBuf);
  freeIf((void*&)irisBuf);
  freeIf((void*&)topOpenBuf);
  freeIf((void*&)botOpenBuf);
  freeIf((void*&)irisPatch);

  const size_t basePix = (size_t)skin.base.w * (size_t)skin.base.h;
  const size_t irisPix = (size_t)skin.iris.w * (size_t)skin.iris.h;

  if (!basePix || !irisPix) return false;

  baseBuf = (uint16_t*)malloc(basePix * 2);
  irisBuf = (uint16_t*)malloc(irisPix * 2);
  irisPatch = (uint16_t*)malloc(irisPix * 2);

  if (!baseBuf || !irisBuf || !irisPatch) {
    Serial.println("[render] malloc FAIL");
    return false;
  }

  if (!loadRaw565(skin.base.path.c_str(), baseBuf, (unsigned int)basePix)) return false;
  if (!loadRaw565(skin.iris.path.c_str(), irisBuf, (unsigned int)irisPix)) return false;

  if (skin.topOpen.w > 0 && skin.topOpen.h > 0 && skin.topOpen.path.length() > 0) {
    const size_t n = (size_t)skin.topOpen.w * (size_t)skin.topOpen.h;
    topOpenBuf = (uint16_t*)malloc(n * 2);
    if (!topOpenBuf) return false;
    if (!loadRaw565(skin.topOpen.path.c_str(), topOpenBuf, (unsigned int)n)) return false;
  }

  if (skin.botOpen.w > 0 && skin.botOpen.h > 0 && skin.botOpen.path.length() > 0) {
    const size_t n = (size_t)skin.botOpen.w * (size_t)skin.botOpen.h;
    botOpenBuf = (uint16_t*)malloc(n * 2);
    if (!botOpenBuf) return false;
    if (!loadRaw565(skin.botOpen.path.c_str(), botOpenBuf, (unsigned int)n)) return false;
  }

  Serial.println("[render] assets OK");
  return true;
}

void eyeRenderDrawStatic(const SkinAssets& skin) {
  if (!baseBuf) return;
  TftManager::pushRGB565(0, 0, skin.base.w, skin.base.h, baseBuf, gCfg.swapBytes);
  overlayOpenLidsIfAny(skin);
}

// Překreslení jen patch oblasti irisu (Patch 9 logika):
// 1) z baseBuf vyřízni patch do irisPatch
// 2) přepiš ho pixely irisu s colorkey (magenta = průhledná)
// 3) push patch na TFT
static void composePatchAndPush(int irisX, int irisY, const SkinAssets& skin) {
  if (!baseBuf || !irisBuf || !irisPatch) return;

  const int baseW = skin.base.w;
  const int baseH = skin.base.h;
  const int irisW = skin.iris.w;
  const int irisH = skin.iris.h;

  // tl = levý horní roh irisu (stejně jako Patch 9)
  const int tlx = irisX - irisW / 2;
  const int tly = irisY - irisH / 2;

  // kopie base do patch
  for (int y = 0; y < irisH; y++) {
    int by = tly + y;
    if (by < 0 || by >= baseH) continue;

    uint16_t* dstRow = irisPatch + (size_t)y * (size_t)irisW;
    const uint16_t* srcRow = baseBuf + (size_t)by * (size_t)baseW;

    for (int x = 0; x < irisW; x++) {
      int bx = tlx + x;
      if (bx < 0 || bx >= baseW) continue;
      dstRow[x] = srcRow[bx];
    }
  }

  // iris overlay (magenta key)
  for (int y = 0; y < irisH; y++) {
    uint16_t* dstRow = irisPatch + (size_t)y * (size_t)irisW;
    const uint16_t* irRow = irisBuf + (size_t)y * (size_t)irisW;

    for (int x = 0; x < irisW; x++) {
      uint16_t px = irRow[x];
      if (px == gCfg.keyColor565) continue;
      dstRow[x] = px;
    }
  }

  TftManager::pushRGB565(tlx, tly, irisW, irisH, irisPatch, gCfg.swapBytes);
}

void eyeRenderDrawIris(int irisX, int irisY, const SkinAssets& skin) {
  composePatchAndPush(irisX, irisY, skin);

  // DŮLEŽITÉ: po každém push irisu obnov víčka,
  // jinak iris „přejede“ do jejich oblasti.
  overlayOpenLidsIfAny(skin);

  // debug overlay (pokud je zapnutý) – volá se přes debug_grid modul
  if (cfg.dbgOverlay) {
    DebugGridCallbacks cb{};
    cb.ctx = nullptr;
    cb.drawPixel = [](void*, int x, int y, uint16_t c){ TftManager::drawPixel(x, y, c); };
    cb.drawRect  = [](void*, int x, int y, int w, int h, uint16_t c){ TftManager::drawRect(x, y, w, h, c); };
    cb.drawLine  = [](void*, int x0, int y0, int x1, int y1, uint16_t c){ TftManager::drawLine(x0, y0, x1, y1, c); };
    cb.drawCircle= [](void*, int x, int y, int r, uint16_t c){ TftManager::drawCircle(x, y, r, c); };
    debugGridRenderOverlay(cb);
  }
}

void eyeRenderDrawLids(uint16_t /*lidTop*/, uint16_t /*lidBot*/, const SkinAssets& skin) {
  // Patch 10: zatím žádné „zavřené“ víčka.
  // Jen zajistíme, že se otevřená víčka kdykoli znovu přepíšou přes iris.
  overlayOpenLidsIfAny(skin);
}
