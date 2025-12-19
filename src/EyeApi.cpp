#include "EyeApi.h"

#include <Arduino.h>

#include "config.h"
#include "sd_manager.h"
#include "skin_assets.h"
#include "skin_config.h"

#include "RenderApi.h"
#include "render_eye.h"

#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "LidsApi.h"
#include "TftManager.h"

static SkinAssets gSkin;
static EyeFrame   gFrame;

static void dieBlink(const char* msg) {
  Serial.println(msg);
  while (true) delay(250);
}

namespace EyeApi {

void init() {
  Serial.begin(115200);
  delay(80);
  Serial.println("[BOOT] start");

  applyDefaults();

  TftManager::init();
  TftManager::showBootScreen();

  if (!sdInit() || !sdIsReady()) dieBlink("[SD] init FAILED");
  Serial.println("[SD] OK");

  loadSkinConfigIfExists(cfg.skinDir, cfg);

  if (!skinScanDir(gSkin, cfg.skinDir)) dieBlink("[skin] scan FAILED");

  EyeGrid::build(
    cfg.screenW / 2, cfg.screenH / 2,
    cfg.irisCircleRadiusPx,
    cfg.stepX, cfg.stepY,
    cfg.edgeBands
  );

  EyePupil::init(cfg);

  // ---- Render API ----
  RenderApi::init(gSkin);
  RenderApi::setupRendererFromConfig();

  if (!RenderApi::loadAssets())
    dieBlink("[render] load assets FAILED");

  RenderApi::drawStatic();

  // Hook na kreslení víček
  RenderApi::setLidDrawFn(
    [](const SkinAssets& skin,
       uint16_t lidTop,
       uint16_t lidBot,
       const DirtyRect&) {
      eyeRenderDrawLids(lidTop, lidBot, skin);
    }
  );

  // Otevřená víčka vykreslíme JEDNOU jako overlay
  eyeRenderDrawLids(0, 0, gSkin);

  EyeGaze::init(
    cfg.dwellMinMs, cfg.dwellMaxMs,
    cfg.travelTickMinMs, cfg.travelTickMaxMs,
    cfg.centerPct, cfg.avoidSame,
    cfg.hopMin, cfg.hopMax,
    cfg.edgeBands, cfg.edgeWeightPct, cfg.edgeSoft
  );

  memset(&gFrame, 0, sizeof(gFrame));

  Serial.println("[READY]");
}

void update(uint32_t dtMs) {
  TftManager::showAliveTick(millis());

  const uint32_t now = millis();

  if (EyeGaze::update(now)) {
    gFrame.irisX = EyeGaze::x();
    gFrame.irisY = EyeGaze::y();
    gFrame.irisDirty = true;
  }

  if (cfg.lidsEnabled) {
    LidsApi::update(dtMs, gFrame);
  } else {
    gFrame.lidTop = 0;
    gFrame.lidBot = 0;
    gFrame.lidsDirty = false;
  }
}

void render() {
  RenderApi::renderFrame(gFrame);
}

} // namespace EyeApi
