#include "RenderApi.h"

#include "render_eye.h"
#include "skin_assets.h"
#include "config.h"
#include "EyeFrame.h"

static const SkinAssets* gSkin = nullptr;
static RenderApi::LidDrawFn gLidDrawFn = nullptr;

static DirtyRect gLastDirty;

static void resetDirty() {
  gLastDirty.valid = false;
  gLastDirty.x = 0;
  gLastDirty.y = 0;
  gLastDirty.w = 0;
  gLastDirty.h = 0;
}

static void addDirty(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (w <= 0 || h <= 0) return;

  if (!gLastDirty.valid) {
    gLastDirty.x = x;
    gLastDirty.y = y;
    gLastDirty.w = w;
    gLastDirty.h = h;
    gLastDirty.valid = true;
    return;
  }

  int16_t x1 = gLastDirty.x;
  int16_t y1 = gLastDirty.y;
  int16_t x2 = (int16_t)(gLastDirty.x + gLastDirty.w);
  int16_t y2 = (int16_t)(gLastDirty.y + gLastDirty.h);

  if (x < x1) x1 = x;
  if (y < y1) y1 = y;
  if (x + w > x2) x2 = (int16_t)(x + w);
  if (y + h > y2) y2 = (int16_t)(y + h);

  gLastDirty.x = x1;
  gLastDirty.y = y1;
  gLastDirty.w = (int16_t)(x2 - x1);
  gLastDirty.h = (int16_t)(y2 - y1);
  gLastDirty.valid = true;
}

namespace RenderApi {

void init(const SkinAssets& skin) {
  gSkin = &skin;
  gLidDrawFn = nullptr;
  resetDirty();
}

void setupRendererFromConfig() {
  if (!gSkin) return;

  EyeRenderConfig rc;
  rc.baseW = cfg.screenW;
  rc.baseH = cfg.screenH;
  rc.irisW = gSkin->iris.w;
  rc.irisH = gSkin->iris.h;

  rc.useKey = cfg.useKey;
  rc.keyColor565 = cfg.keyColor565;
  rc.tolR = cfg.keyTolR;
  rc.tolG = cfg.keyTolG;
  rc.tolB = cfg.keyTolB;

  eyeRenderInit(rc);
  eyeRenderSetKey(rc.useKey, rc.keyColor565, rc.tolR, rc.tolG, rc.tolB);
}

bool loadAssets() {
  if (!gSkin) return false;
  return eyeRenderLoadAssets(*gSkin);
}

void drawStatic() {
  if (!gSkin) return;
  eyeRenderDrawStatic(*gSkin);
  addDirty(0, 0, (int16_t)cfg.screenW, (int16_t)cfg.screenH);
}

void drawIris(int16_t cx, int16_t cy) {
  if (!gSkin) return;
  eyeRenderDrawIris((int)cx, (int)cy, *gSkin);

  // zatím nepočítáme přesně patch; ber full-screen jako bezpečnou variantu
  addDirty(0, 0, (int16_t)cfg.screenW, (int16_t)cfg.screenH);
}

void beginFrame() { resetDirty(); }
void endFrame()   { /* no-op */ }

void setLidDrawFn(LidDrawFn fn) { gLidDrawFn = fn; }

DirtyRect lastDirty() { return gLastDirty; }

void renderFrame(const EyeFrame& frame) {
  if (!gSkin) return;

  beginFrame();

  if (frame.irisDirty) {
    drawIris(frame.irisX, frame.irisY);
  }

  if (frame.lidsDirty && gLidDrawFn) {
    gLidDrawFn(*gSkin, frame.lidTop, frame.lidBot, gLastDirty);
    // víčka zasahují velkou část; zatím full-screen
    addDirty(0, 0, (int16_t)cfg.screenW, (int16_t)cfg.screenH);
  }

  endFrame();
}

} // namespace RenderApi
