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
  gLastDirty.x = gLastDirty.y = gLastDirty.w = gLastDirty.h = 0;
}

static void addDirty(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (w <= 0 || h <= 0) return;

  if (!gLastDirty.valid) {
    // FIX: žádné brace assignment
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

  int16_t nx1 = x;
  int16_t ny1 = y;
  int16_t nx2 = (int16_t)(x + w);
  int16_t ny2 = (int16_t)(y + h);

  int16_t ux1 = (nx1 < x1) ? nx1 : x1;
  int16_t uy1 = (ny1 < y1) ? ny1 : y1;
  int16_t ux2 = (nx2 > x2) ? nx2 : x2;
  int16_t uy2 = (ny2 > y2) ? ny2 : y2;

  gLastDirty.x = ux1;
  gLastDirty.y = uy1;
  gLastDirty.w = (int16_t)(ux2 - ux1);
  gLastDirty.h = (int16_t)(uy2 - uy1);
  gLastDirty.valid = true;
}

namespace RenderApi {

void init(const SkinAssets& skin) { gSkin = &skin; }

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
}

bool loadAssets() {
  if (!gSkin) return false;
  return eyeRenderLoadAssets(*gSkin);
}

void drawStatic() {
  if (!gSkin) return;
  eyeRenderDrawStatic(*gSkin);
}

void drawIris(int16_t irisX, int16_t irisY) {
  if (!gSkin) return;
  eyeRenderDrawIris(irisX, irisY, *gSkin);
  addDirty(irisX, irisY, (int16_t)gSkin->iris.w, (int16_t)gSkin->iris.h);
}

void beginFrame() { resetDirty(); }
void endFrame() {}

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
    addDirty(0, 0, (int16_t)cfg.screenW, (int16_t)cfg.screenH);
  }

  endFrame();
}

} // namespace RenderApi
