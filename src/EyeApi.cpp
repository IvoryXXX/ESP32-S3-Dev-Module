#include "EyeApi.h"

#include <Arduino.h>

#include "config.h"
#include "skin_assets.h"
#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "TftManager.h"

#include "RenderApi.h"
#include "AssetsApi.h"
#include "EyeFrame.h"

static SkinAssets gSkin;

static void dieBlink(const char* msg) {
  Serial.println(msg);
  while (true) {
    delay(250);
  }
}

// Patch 6: oficiální kontrakt frame
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

  // první frame: statika už je, iris zatím není dirty
  gFrame.irisX = (int16_t)EyeGaze::x();
  gFrame.irisY = (int16_t)EyeGaze::y();
  gFrame.irisDirty = false;
  gFrame.lidsDirty = false;

  Serial.println("[READY]");
}

void update(uint32_t dtMs) {
  (void)dtMs;

  TftManager::showAliveTick(millis());

  const uint32_t now = millis();
  const bool changed = EyeGaze::update(now);

  if (changed) {
    gFrame.irisX = (int16_t)EyeGaze::x();
    gFrame.irisY = (int16_t)EyeGaze::y();
    gFrame.irisDirty = true;
  }
}

void render() {
  // Patch 6: renderer bere jednotný frame kontrakt
  if (gFrame.irisDirty || gFrame.lidsDirty) {
    RenderApi::renderFrame(gFrame);
    gFrame.irisDirty = false;
    gFrame.lidsDirty = false;
  }

  // zachováme pacing jako dřív
  delay(5);
}

} // namespace EyeApi
