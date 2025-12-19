#include <Arduino.h>

#include "config.h"
#include "sd_manager.h"
#include "skin_assets.h"
#include "skin_config.h"
#include "render_eye.h"
#include "eye_grid.h"
#include "eye_gaze.h"
#include "eye_pupil.h"
#include "TftManager.h" // init + boot screen + alive tick (kreslení frame řeší renderer)

static SkinAssets gSkin;

static void dieBlink(const char* msg) {
  Serial.println(msg);
  while (true) {
    delay(250);
  }
}

void setup() {
  Serial.begin(115200);
  delay(80);
  Serial.println("[BOOT] start");

  applyDefaults();

  TftManager::init();
  TftManager::showBootScreen();

  if (!sdInit() || !sdIsReady()) dieBlink("[SD] init FAILED");
  Serial.println("[SD] OK");

  // pokus načíst /skins/<skin>/settings.txt (když neexistuje, jedeme dál)
  loadSkinConfigIfExists(cfg.skinDir, cfg);

  // scan assets ve skinu
  if (!skinScanDir(gSkin, cfg.skinDir)) dieBlink("[skin] scan FAILED");

  // init grid z configu (kruh + body)
  EyeGrid::build(cfg.screenW / 2, cfg.screenH / 2,
                cfg.irisCircleRadiusPx, cfg.stepX, cfg.stepY,
                cfg.edgeBands);

  // masky víček se aplikují jinde podle gSkin + shiftů (pokud to už máš hotové, nech)
  // (tady necpu nic natvrdo, protože to máš v projektu rozdělené)

  // init pupil až PO skin configu
  EyePupil::init(cfg);

  // render init/load
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

  eyeRenderDrawStatic(gSkin);

  // gaze init
  EyeGaze::init(
    cfg.dwellMinMs, cfg.dwellMaxMs,
    cfg.travelTickMinMs, cfg.travelTickMaxMs,
    cfg.centerPct, cfg.avoidSame,
    cfg.hopMin, cfg.hopMax,
    cfg.edgeBands, cfg.edgeWeightPct, cfg.edgeSoft
  );
Serial.println("[READY]");
}

void loop() {
  TftManager::showAliveTick(millis());

  const uint32_t now = millis();

  bool changed = EyeGaze::update(now);
  if (changed) {
    eyeRenderDrawIris(EyeGaze::x(), EyeGaze::y(), gSkin);
}

  delay(5);
}
