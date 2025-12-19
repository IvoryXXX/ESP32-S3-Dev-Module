#include "RenderApi.h"
#include "render_eye.h"
#include "config.h"
#include "EyeFrame.h"
#include "skin_assets.h"

static RenderApi::LidDrawFn s_lidFn = nullptr;

void RenderApi::setLidDrawFn(LidDrawFn fn) { s_lidFn = fn; }

void RenderApi::setSwapBytes(bool v) { (void)v; /* řeší eyeRenderInit */ }
void RenderApi::setKeyColor565(uint16_t k) { (void)k; /* řeší eyeRenderInit */ }

void RenderApi::init() {
  EyeRenderConfig rc;
  rc.baseW = cfg.screenW;
  rc.baseH = cfg.screenH;
  rc.keyColor565 = cfg.keyColor565;
  rc.swapBytes = cfg.swapBytes;

  eyeRenderInit(rc);
}

void RenderApi::drawStatic(const SkinAssets& skin) {
  eyeRenderDrawStatic(skin);
}

void RenderApi::drawFrame(const EyeFrame& frame) {
  // Iris
  if (frame.irisDirty) {
    eyeRenderDrawIris(frame.irisX, frame.irisY, gSkin);
  }

  // Víčka (zatím jen „obnov otevřená víčka“ přes hook)
  if (frame.lidsDirty && s_lidFn) {
    DirtyRect dr{};
    s_lidFn(gSkin, frame.lidTop, frame.lidBot, dr);
  }
}
