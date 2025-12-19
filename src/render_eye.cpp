#include "render_eye.h"
#include "_render_internal.h"
#include "config.h"
#include <SD.h>

#include "eye_pupil.h"
// RAW565 loader (Patch 10 hotfix)
// Musí mít přesně tento podpis, jinak to linker nenajde:
// loadRaw565(char const*, unsigned short*, unsigned int)
bool loadRaw565(const char* path, uint16_t* outPixels, unsigned int pixelCount) {
  if (!path || !outPixels || pixelCount == 0) return false;

  File f = SD.open(path, FILE_READ);
  if (!f) return false;

  const size_t wantBytes = (size_t)pixelCount * 2u;
  size_t got = 0;

  // Čteme přímo do bufferu (RAW je 16-bit 565)
  while (got < wantBytes) {
    int n = f.read(((uint8_t*)outPixels) + got, wantBytes - got);
    if (n <= 0) break;
    got += (size_t)n;
  }

  f.close();
  return got == wantBytes;
}


// --- internal state ---
static EyeRenderConfig gCfg;

static uint16_t* baseBuf      = nullptr;
static uint16_t* irisBuf      = nullptr;
static uint16_t* topOpenBuf   = nullptr;
static uint16_t* botOpenBuf   = nullptr;
static uint16_t* topClosedBuf = nullptr;
static uint16_t* botClosedBuf = nullptr;

static uint16_t* scratchBuf  = nullptr;
static size_t    scratchCap  = 0;        // number of uint16_t

static bool baseLoaded = false;
static bool irisLoaded = false;

static int lastX0 = 999999;
static int lastY0 = 999999;

// --- helpers ---
static inline uint8_t r565(uint16_t p){ return (uint8_t)(((p >> 11) & 0x1F) * 255 / 31); }
static inline uint8_t g565(uint16_t p){ return (uint8_t)(((p >>  5) & 0x3F) * 255 / 63); }
static inline uint8_t b565(uint16_t p){ return (uint8_t)(((p >>  0) & 0x1F) * 255 / 31); }

static inline bool isKey(uint16_t p) {
  if (!gCfg.useKey) return false;
  const uint16_t k = gCfg.keyColor565;
  int dr = (int)r565(p) - (int)r565(k);
  int dg = (int)g565(p) - (int)g565(k);
  int db = (int)b565(p) - (int)b565(k);
  if (dr < 0) dr = -dr;
  if (dg < 0) dg = -dg;
  if (db < 0) db = -db;
  return (dr <= (int)gCfg.tolR) && (dg <= (int)gCfg.tolG) && (db <= (int)gCfg.tolB);
}

// AssetInfo in your project doesn't have `.valid` and x/y are getters.
// We treat asset as present if it has dimensions and non-empty path.
static inline bool hasAsset(const AssetInfo& a) {
  if (a.w <= 0 || a.h <= 0) return false;
  const char* p = a.path.c_str();        // works for String and std::string
  return (p && p[0] != '\0');
}

static bool ensureScratch(size_t pxCount) {
  if (scratchBuf && scratchCap >= pxCount) return true;
  if (scratchBuf) { free(scratchBuf); scratchBuf = nullptr; scratchCap = 0; }
  scratchBuf = (uint16_t*)malloc(pxCount * sizeof(uint16_t));
  if (!scratchBuf) return false;
  scratchCap = pxCount;
  return true;
}

// --- API ---
void eyeRenderInit(const EyeRenderConfig& cfg) {
  gCfg = cfg;
}

void eyeRenderSetKey(bool useKey, uint16_t keyColor565, uint8_t tolR, uint8_t tolG, uint8_t tolB) {
  gCfg.useKey = useKey;
  gCfg.keyColor565 = keyColor565;
  gCfg.tolR = tolR;
  gCfg.tolG = tolG;
  gCfg.tolB = tolB;
}

bool eyeRenderLoadAssets(const SkinAssets& skin) {
  // base
  if (!baseBuf) {
    baseBuf = (uint16_t*)malloc((size_t)skin.base.w * (size_t)skin.base.h * sizeof(uint16_t));
    if (!baseBuf) return false;
  }
  if (!loadRaw565(skin.base.path.c_str(), baseBuf, (size_t)skin.base.w * (size_t)skin.base.h)) return false;
  baseLoaded = true;

  // iris
  if (!irisBuf) {
    irisBuf = (uint16_t*)malloc((size_t)skin.iris.w * (size_t)skin.iris.h * sizeof(uint16_t));
    if (!irisBuf) return false;
  }
  if (!loadRaw565(skin.iris.path.c_str(), irisBuf, (size_t)skin.iris.w * (size_t)skin.iris.h)) return false;
  irisLoaded = true;

  // lids (open)
  if (hasAsset(skin.topOpen)) {
    if (!topOpenBuf) {
      topOpenBuf = (uint16_t*)malloc((size_t)skin.topOpen.w * (size_t)skin.topOpen.h * sizeof(uint16_t));
      if (!topOpenBuf) return false;
    }
    if (!loadRaw565(skin.topOpen.path.c_str(), topOpenBuf, (size_t)skin.topOpen.w * (size_t)skin.topOpen.h)) return false;
  }

  if (hasAsset(skin.botOpen)) {
    if (!botOpenBuf) {
      botOpenBuf = (uint16_t*)malloc((size_t)skin.botOpen.w * (size_t)skin.botOpen.h * sizeof(uint16_t));
      if (!botOpenBuf) return false;
    }
    if (!loadRaw565(skin.botOpen.path.c_str(), botOpenBuf, (size_t)skin.botOpen.w * (size_t)skin.botOpen.h)) return false;
  }

  // lids (closed)
  if (hasAsset(skin.topClosed)) {
    if (!topClosedBuf) {
      topClosedBuf = (uint16_t*)malloc((size_t)skin.topClosed.w * (size_t)skin.topClosed.h * sizeof(uint16_t));
      if (!topClosedBuf) return false;
    }
    if (!loadRaw565(skin.topClosed.path.c_str(), topClosedBuf, (size_t)skin.topClosed.w * (size_t)skin.topClosed.h)) return false;
  }

  if (hasAsset(skin.botClosed)) {
    if (!botClosedBuf) {
      botClosedBuf = (uint16_t*)malloc((size_t)skin.botClosed.w * (size_t)skin.botClosed.h * sizeof(uint16_t));
      if (!botClosedBuf) return false;
    }
    if (!loadRaw565(skin.botClosed.path.c_str(), botClosedBuf, (size_t)skin.botClosed.w * (size_t)skin.botClosed.h)) return false;
  }

  return true;
}

void eyeRenderDrawStatic(const SkinAssets& skin) {
  if (!baseLoaded) return;
  TftManager::tft().pushImage(0, 0, skin.base.w, skin.base.h, baseBuf);
}

void eyeRenderDrawIris(int centerX, int centerY, const SkinAssets& skin) {
  if (!baseLoaded || !irisLoaded) return;

  const int pw = skin.iris.w;
  const int ph = skin.iris.h;

  const int x0 = centerX - pw / 2;
  const int y0 = centerY - ph / 2;

  // Pokud se iris posunula, nejdřív obnov base v MINULÉM patchi,
  // jinak zůstávají "duchové" staré iris.
  if (x0 == lastX0 && y0 == lastY0) return;

  if (lastX0 != 999999 && lastY0 != 999999) {
  const int oldX0 = lastX0;
  const int oldY0 = lastY0;

  // obnovíme base po řádcích (sub-rect není v paměti souvislý)
  for (int y = 0; y < ph; y++) {
    const int sy = oldY0 + y;
    if (sy < 0 || sy >= skin.base.h) continue;

    const int sx = oldX0;
    if (sx < 0 || sx + pw > skin.base.w) continue;

    const uint16_t* rowPtr = baseBuf + (size_t)sy * (size_t)skin.base.w + (size_t)sx;
    TftManager::tft().pushImage((int16_t)oldX0, (int16_t)sy, (int16_t)pw, (int16_t)1, rowPtr);
  }
}

lastX0 = x0;
lastY0 = y0;


  if (!ensureScratch((size_t)pw * (size_t)ph)) return;

  // 1) base under patch
  for (int y = 0; y < ph; y++) {
    const int sy = y0 + y;
    if (sy < 0 || sy >= skin.base.h) continue;

    const size_t srcRow = (size_t)sy * (size_t)skin.base.w;
    const size_t dstRow = (size_t)y  * (size_t)pw;

    for (int x = 0; x < pw; x++) {
      const int sx = x0 + x;
      if (sx < 0 || sx >= skin.base.w) continue;
      scratchBuf[dstRow + (size_t)x] = baseBuf[srcRow + (size_t)sx];
    }
  }

  // 2) iris overlay (key transparent)
  for (int y = 0; y < ph; y++) {
    const size_t row = (size_t)y * (size_t)pw;
    for (int x = 0; x < pw; x++) {
      uint16_t ip = irisBuf[row + (size_t)x];
      if (!isKey(ip)) scratchBuf[row + (size_t)x] = ip;
    }
  }

  // 2b) pupil (your API draws into the current patch)
  EyePupil::drawIntoPatch(scratchBuf, pw, ph);

  // 3) push patch
  TftManager::tft().pushImage(x0, y0, pw, ph, scratchBuf);
}

// topClosePct/botClosePct: 0=open, 1000=fully closed
void eyeRenderDrawLids(uint16_t topClosePct, uint16_t botClosePct, const SkinAssets& skin) {
  // TOP lid: draw upper part of closed lid
  if (hasAsset(skin.topClosed) && topClosedBuf && topClosePct > 0) {
    int h = (int)((uint32_t)skin.topClosed.h * (uint32_t)topClosePct / 1000U);
    if (h < 0) h = 0;
    if (h > skin.topClosed.h) h = skin.topClosed.h;

    if (h > 0) {
      TftManager::tft().pushImage((int16_t)skin.topClosed.x(),
                                 (int16_t)skin.topClosed.y(),
                                 (int16_t)skin.topClosed.w,
                                 (int16_t)h,
                                 topClosedBuf);
    }
  }

  // BOTTOM lid: draw lower part of closed lid
  if (hasAsset(skin.botClosed) && botClosedBuf && botClosePct > 0) {
    int h = (int)((uint32_t)skin.botClosed.h * (uint32_t)botClosePct / 1000U);
    if (h < 0) h = 0;
    if (h > skin.botClosed.h) h = skin.botClosed.h;

    if (h > 0) {
      const int yStart = skin.botClosed.y() + (skin.botClosed.h - h);
      const uint16_t* src = botClosedBuf + (size_t)(skin.botClosed.h - h) * (size_t)skin.botClosed.w;

      TftManager::tft().pushImage((int16_t)skin.botClosed.x(),
                                 (int16_t)yStart,
                                 (int16_t)skin.botClosed.w,
                                 (int16_t)h,
                                 src);
    }
  }
}
