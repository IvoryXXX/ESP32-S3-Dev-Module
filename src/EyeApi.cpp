#include "EyeApi.h"

#include <Arduino.h>

#include "config.h"
#include "sd_manager.h"
#include "skin_assets.h"
#include "skin_config.h"
#include "render_eye.h"
#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "TftManager.h" // init + boot screen + alive tick
#include "RenderApi.h"

static SkinAssets gSkin;

static void dieBlink(const char* msg) {
  Serial.println(msg);
  while (true) {
    delay(250);
  }
}

// ----- Patch 2: stav mezi update() a render() -----
struct EyeFrameState {
  bool changed = false;
  int16_t irisX = 0;
  int16_t irisY = 0;
};
static EyeFrameState gFrame;
// --------------------------------------------------

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

  EyeGrid::build(cfg.screenW / 2, cfg.screenH / 2,
                 cfg.irisCircleRadiusPx, cfg.stepX, cfg.stepY,
                 cfg.edgeBands);

  EyePupil::init(cfg);

  EyeRenderConfig rc;
  rc.baseW = cfg.screenW;
  rc.baseH = cfg.screenH;
  rc.irisW = gSkin.iris.w;
  rc.irisH = gSkin.iris.h;

  rc.useKey = cfg.useKey;
  rc.keyColor565 = cfg.keyColor565;
  rc.tolR = cfg.keyTolR;
  rc.tolG = cfg.keyTolG;
  rc.tolB = cfg.keyTolB;

  eyeRenderInit(rc);
  if (!eyeRenderLoadAssets(gSkin)) dieBlink("[render] load assets FAILED");

  RenderApi::init(gSkin);
  RenderApi::drawStatic();


  EyeGaze::init(
    cfg.dwellMinMs, cfg.dwellMaxMs,
    cfg.travelTickMinMs, cfg.travelTickMaxMs,
    cfg.centerPct, cfg.avoidSame,
    cfg.hopMin, cfg.hopMax,
    cfg.edgeBands, cfg.edgeWeightPct, cfg.edgeSoft
  );

  gFrame.changed = false;
  gFrame.irisX = (int16_t)EyeGaze::x();
  gFrame.irisY = (int16_t)EyeGaze::y();

  Serial.println("[READY]");
}

void update(uint32_t dtMs) {
  (void)dtMs; // zachováme původní časování (EyeGaze bere millis())

  TftManager::showAliveTick(millis());

  const uint32_t now = millis();
  gFrame.changed = EyeGaze::update(now);
  if (gFrame.changed) {
    gFrame.irisX = (int16_t)EyeGaze::x();
    gFrame.irisY = (int16_t)EyeGaze::y();
  }
}

void render() {
  if (gFrame.changed) {
    RenderApi::drawIris(gFrame.irisX, gFrame.irisY);

    gFrame.changed = false;
  }

  // zachováme pacing jako dřív
  delay(5);
}

} // namespace EyeApi
