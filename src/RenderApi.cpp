#include "RenderApi.h"

#include "render_eye.h"     // interní detail rendereru – sem patří
#include "skin_assets.h"    // definice SkinAssets
#include "config.h"         // globální cfg

static const SkinAssets* gSkin = nullptr;

namespace RenderApi {

void init(const SkinAssets& skin) {
  gSkin = &skin;
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
}

} // namespace RenderApi
