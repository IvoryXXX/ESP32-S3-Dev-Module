#include "render_eye.h"
#include "debug_grid.h"
#include "_render_internal.h"
#include "config.h"      // cfg.dbgOverlay (optional debug)
#include <SD.h>


// --- Debug overlay callbacks (renderer owns TFT access) ---
static void _dbgDrawPixel(void* ctx, int x, int y, uint16_t c) {
  (void)ctx;
  TftManager::tft().drawPixel(x, y, c);
}
static void _dbgDrawRect(void* ctx, int x, int y, int w, int h, uint16_t c) {
  (void)ctx;
  TftManager::tft().drawRect(x, y, w, h, c);
}
static void _dbgDrawCircle(void* ctx, int x, int y, int r, uint16_t c) {
  (void)ctx;
  TftManager::tft().drawCircle(x, y, r, c);
}
static void _dbgDrawLine(void* ctx, int x0, int y0, int x1, int y1, uint16_t c) {
  (void)ctx;
  TftManager::tft().drawLine(x0, y0, x1, y1, c);
}
#include "eye_pupil.h"   // <-- PUPIL V1.1

static EyeRenderConfig gRender;

// asset buffery (RAW565)
static uint16_t* baseBuf     = nullptr;  // baseW*baseH
static uint16_t* irisBuf     = nullptr;  // irisW*irisH
static uint16_t* topOpenBuf  = nullptr;  // topOpenW*topOpenH (optional)
static uint16_t* botOpenBuf  = nullptr;  // botOpenW*botOpenH (optional)

// scratch (na patchy: iris patch, nebo lid patch)
static uint16_t* scratchBuf  = nullptr;
static size_t    scratchCap  = 0;        // počet uint16_t

static bool baseLoaded = false;
static bool irisLoaded = false;

static int lastX0 = 999999;
static int lastY0 = 999999;

static inline uint16_t baseAt(int x, int y) {
  if (!baseBuf) return 0;
  if (x < 0 || y < 0 || x >= gRender.baseW || y >= gRender.baseH) return 0;
  return baseBuf[(size_t)y * (size_t)gRender.baseW + (size_t)x];
}

static inline void unpack565(uint16_t c, int &r, int &g, int &b) {
  r = (c >> 11) & 0x1F;
  g = (c >> 5)  & 0x3F;
  b = (c >> 0)  & 0x1F;
}

static bool isKeyLike(uint16_t p) {
  if (!gRender.useKey) return false;

  int r,gc,b;
  int kr,kg,kb;
  unpack565(p,  r,  gc,  b);
  unpack565(gRender.keyColor565, kr, kg, kb);

  int dr = abs(r  - kr);
  int dg = abs(gc - kg);
  int db = abs(b  - kb);

  return (dr <= gRender.tolR) && (dg <= gRender.tolG) && (db <= gRender.tolB);
}

static bool ensureScratch(size_t needCount) {
  if (scratchCap >= needCount && scratchBuf) return true;
  if (scratchBuf) { free(scratchBuf); scratchBuf = nullptr; scratchCap = 0; }

  scratchBuf = (uint16_t*)malloc(needCount * sizeof(uint16_t));
  if (!scratchBuf) return false;
  scratchCap = needCount;
  return true;
}

static bool loadRaw565(const char* path, uint16_t* dst, size_t countU16) {
  File f = SD.open(path, FILE_READ);
  if (!f) return false;
  size_t needBytes = countU16 * sizeof(uint16_t);
  size_t got = f.read((uint8_t*)dst, needBytes);
  f.close();
  return (got == needBytes);
}

void eyeRenderSetKey(bool useKey, uint16_t keyColor565, uint8_t tolR, uint8_t tolG, uint8_t tolB) {
  gRender.useKey = useKey;
  gRender.keyColor565 = keyColor565;
  gRender.tolR = tolR;
  gRender.tolG = tolG;
  gRender.tolB = tolB;
}

void eyeRenderInit(const EyeRenderConfig& cfg) {
  gRender = cfg;
}

bool eyeRenderLoadAssets(const SkinAssets& skin) {
  baseLoaded = false;
  irisLoaded = false;

  if (!ensureScratch((size_t)gRender.irisW * (size_t)gRender.irisH)) return false;

  if (!baseBuf) baseBuf = (uint16_t*)malloc((size_t)gRender.baseW * (size_t)gRender.baseH * sizeof(uint16_t));
  if (!irisBuf) irisBuf = (uint16_t*)malloc((size_t)gRender.irisW * (size_t)gRender.irisH * sizeof(uint16_t));

  if (!baseBuf || !irisBuf) return false;

  if (!loadRaw565(skin.base.path.c_str(), baseBuf, (size_t)gRender.baseW*(size_t)gRender.baseH)) return false;
  if (!loadRaw565(skin.iris.path.c_str(), irisBuf, (size_t)gRender.irisW*(size_t)gRender.irisH)) return false;

  baseLoaded = true;
  irisLoaded = true;

  // optional lids
  if (skin.topOpen.found) {
    if (!topOpenBuf) topOpenBuf = (uint16_t*)malloc((size_t)skin.topOpen.w * (size_t)skin.topOpen.h * sizeof(uint16_t));
    if (!topOpenBuf) return false;
    if (!loadRaw565(skin.topOpen.path.c_str(), topOpenBuf, (size_t)skin.topOpen.w*(size_t)skin.topOpen.h)) return false;
  }
  if (skin.botOpen.found) {
    if (!botOpenBuf) botOpenBuf = (uint16_t*)malloc((size_t)skin.botOpen.w * (size_t)skin.botOpen.h * sizeof(uint16_t));
    if (!botOpenBuf) return false;
    if (!loadRaw565(skin.botOpen.path.c_str(), botOpenBuf, (size_t)skin.botOpen.w*(size_t)skin.botOpen.h)) return false;
  }

  lastX0 = 999999;
  lastY0 = 999999;
  return true;
}

static void overlayLidIfHit(int px, int py, int pw, int ph,
                            const AssetInfo& lid, const uint16_t* lidBuf) {
  if (!lid.found || !lidBuf) return;

  int lx = lid.x();
  int ly = lid.y();
  int lw = lid.w;
  int lh = lid.h;

  // průnik patch vs lid rect
  int ix0 = max(px, lx);
  int iy0 = max(py, ly);
  int ix1 = min(px + pw, lx + lw);
  int iy1 = min(py + ph, ly + lh);

  if (ix1 <= ix0 || iy1 <= iy0) return;

  int iw = ix1 - ix0;
  int ih = iy1 - iy0;

  for (int y = 0; y < ih; y++) {
    int sy = iy0 + y;
    int lidY = sy - ly;
    int locY = sy - py;

    size_t lidRow = (size_t)lidY * (size_t)lw;
    size_t dstRow = (size_t)locY * (size_t)pw;

    for (int x = 0; x < iw; x++) {
      int sx = ix0 + x;
      int lidX = sx - lx;
      int locX = sx - px;

      uint16_t lp = lidBuf[lidRow + (size_t)lidX];
      if (!isKeyLike(lp)) {
        scratchBuf[dstRow + (size_t)locX] = lp;
      }
    }
  }
}

static void composePatchAndPush(int px, int py, int pw, int ph, bool includeIris, const SkinAssets* skinOpt) {
  if (!ensureScratch((size_t)pw * (size_t)ph)) return;

  // 1) base do scratch
  for (int y = 0; y < ph; y++) {
    int sy = py + y;
    for (int x = 0; x < pw; x++) {
      int sx = px + x;
      scratchBuf[(size_t)y * (size_t)pw + (size_t)x] = baseAt(sx, sy);
    }
  }

  // 2) iris overlay (keying)
  if (includeIris && irisBuf) {
    for (int y = 0; y < ph; y++) {
      for (int x = 0; x < pw; x++) {
        if (x < gRender.irisW && y < gRender.irisH) {
          uint16_t ip = irisBuf[(size_t)y * (size_t)gRender.irisW + (size_t)x];
          if (!isKeyLike(ip)) scratchBuf[(size_t)y * (size_t)pw + (size_t)x] = ip;
        }
      }
    }

    // 2.1) PUPIL do iris patch (V1.1) – po irisu, před víčkama
    EyePupil::drawIntoPatch(scratchBuf, pw, ph);
  }

  // 3) víčka nad irisem – jen lokálně, pokud patch protíná open-lid rect
  if (skinOpt) {
    overlayLidIfHit(px, py, pw, ph, skinOpt->topOpen, topOpenBuf);
    overlayLidIfHit(px, py, pw, ph, skinOpt->botOpen, botOpenBuf);
  }

  // 4) push patch na TFT
  TFT_eSPI &tft = TftManager::tft();
  tft.pushImage(px, py, pw, ph, scratchBuf);
}

void eyeRenderDrawStatic(const SkinAssets& skin) {
  if (!baseLoaded || !baseBuf) return;

  TFT_eSPI &tft = TftManager::tft();
  tft.pushImage(0, 0, gRender.baseW, gRender.baseH, baseBuf);

  // open lids (pokud existují) – kreslíme KEYED (magenta=průhledná)
  // Pozn.: TFT_eSPI pushImage neumí keying, proto si vyrobíme patch (base + lid s klíčem)
  if (skin.topOpen.found && topOpenBuf) {
    composePatchAndPush(skin.topOpen.x(), skin.topOpen.y(), skin.topOpen.w, skin.topOpen.h, false, &skin);
  }
  if (skin.botOpen.found && botOpenBuf) {
    composePatchAndPush(skin.botOpen.x(), skin.botOpen.y(), skin.botOpen.w, skin.botOpen.h, false, &skin);
  }

  lastX0 = 999999;
  lastY0 = 999999;
}

void eyeRenderDrawIris(int centerX, int centerY, const SkinAssets& skin) {
  if (!baseLoaded || !irisLoaded || !baseBuf || !irisBuf) return;

  const int x0 = centerX - gRender.irisW / 2;
  const int y0 = centerY - gRender.irisH / 2;

  // 1) smaž starý iris: obnov base patch + (pokud byl průnik) lokálně dokresli víčka
  if (lastX0 != 999999) {
    composePatchAndPush(lastX0, lastY0, gRender.irisW, gRender.irisH, false, &skin);
  }

  // 2) nový iris: base patch + iris + pupil + (pokud průnik) lokálně víčka nad irisem
  // Pozn.: potřebujeme SkinAssets pro overlay lids. Tady to řešíme přes gRender.lastSkin? -> jednoduché:
  // render_eye.h už typicky předává SkinAssets do drawStatic/loadAssets; iris draw nemá skin param.
  // Takže vezmeme kompromis: víčka se lokálně NEoverlayují (nebo si sem doplníš pointer).
  // Pokud chceš 100%: uprav render API tak, aby eyeRenderDrawIris dostal i SkinAssets*.
  composePatchAndPush(x0, y0, gRender.irisW, gRender.irisH, true, &skin);

  lastX0 = x0;
  lastY0 = y0;

  // debug overlay (jen přes renderer)
  if (cfg.dbgOverlay) {
    DebugGridCallbacks cb;
    cb.ctx = nullptr;
    cb.drawPixel  = _dbgDrawPixel;
    cb.drawRect   = _dbgDrawRect;
    cb.drawCircle = _dbgDrawCircle;
    cb.drawLine   = _dbgDrawLine;
    debugGridRenderOverlay(cb);
  }
}
