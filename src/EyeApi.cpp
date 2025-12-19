#include "EyeApi.h"

#include <Arduino.h>

#include "config.h"
#include "skin_assets.h"
#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "TftManager.h"

#include "AssetsApi.h"
#include "RenderApi.h"
#include "EyeFrame.h"
#include "LidsApi.h"

static SkinAssets gSkin;

static void dieBlink(const char* msg) {
  Serial.println(msg);
  while (true) { delay(250); }
}

static EyeFrame gFrame;

namespace EyeApi {

void init() {
  Serial.begin(115200);
  delay(80);
  Serial.println("[BOOT] start");

  applyDefaults();

  TftManager::init();
  TftManager::showBootScreen();

  if (!AssetsApi::initSd()) dieBlink("[SD] init FAILED");
  Serial.println("[SD] OK");

  if (!AssetsApi::loadSkin(cfg.skinDir, gSkin)) dieBlink("[skin] scan FAILED");

  EyeGrid::build(cfg.screenW / 2, cfg.screenH / 2,
                 cfg.irisCircleRadiusPx, cfg.stepX, cfg.stepY,
                 cfg.edgeBands);

  EyePupil::init(cfg);

  RenderApi::init(gSkin);
  RenderApi::setupRendererFromConfig();
  if (!RenderApi::loadAssets()) dieBlink("[render] load assets FAILED");
  RenderApi::drawStatic();

  EyeGaze::init(
    cfg.dwellMinMs, cfg.dwellMaxMs,
    cfg.travelTickMinMs, cfg.travelTickMaxMs,
    cfg.centerPct, cfg.avoidSame,
    cfg.hopMin, cfg.hopMax,
    cfg.edgeBands, cfg.edgeWeightPct, cfg.edgeSoft
  );

  LidsApi::init();

  gFrame.irisX = (int16_t)EyeGaze::x();
  gFrame.irisY = (int16_t)EyeGaze::y();
  gFrame.irisDirty = false;

  gFrame.lidTop = 0;
  gFrame.lidBot = 0;
  gFrame.lidsDirty = true; // první průchod víček po bootu

  Serial.println("[READY]");
}

void update(uint32_t dtMs) {
  TftManager::showAliveTick(millis());

  // iris – zatím podle existující logiky EyeGaze (millis)
  const uint32_t now = millis();
  const bool irisChanged = EyeGaze::update(now);
  if (irisChanged) {
    gFrame.irisX = (int16_t)EyeGaze::x();
    gFrame.irisY = (int16_t)EyeGaze::y();
    gFrame.irisDirty = true;
  }

  // lids – Patch 9 deterministicky přes dtMs
  LidsApi::update(dtMs, gFrame);
}

void render() {
  if (gFrame.irisDirty || gFrame.lidsDirty) {
    RenderApi::renderFrame(gFrame);
    gFrame.irisDirty = false;
    gFrame.lidsDirty = false;
  }

  // Patch 9: žádné delay(5) – pacing je věc main loop (a/nebo yield)
}

} // namespace EyeApi
