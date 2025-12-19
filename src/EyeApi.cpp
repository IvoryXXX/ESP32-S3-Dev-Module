#include "EyeApi.h"

#include <Arduino.h>

#include "config.h"
#include "EyeFrame.h"
#include "skin_assets.h"

#include "RenderApi.h"
#include "GazeApi.h"
#include "LidsApi.h"

#include "render_eye.h" // kvůli eyeRenderDrawLids

EyeFrame gFrame;

void EyeApi::init() {
  RenderApi::init();
  GazeApi::init();
  LidsApi::init();

  // statická obrazovka + base
  RenderApi::drawStatic(gSkin);

  // Patch 10: hook – kdykoli se má kreslit víčko, zatím jen obnov otevřená víčka
  RenderApi::setLidDrawFn([](const SkinAssets& skin, uint16_t lidTop, uint16_t lidBot, const DirtyRect&) {
    eyeRenderDrawLids(lidTop, lidBot, skin);
  });

  // výchozí frame
  gFrame.irisX = cfg.screenW / 2;
  gFrame.irisY = cfg.screenH / 2;
  gFrame.lidTop = 0;
  gFrame.lidBot = 0;
  gFrame.irisDirty = true;
  gFrame.lidsDirty = true;

  RenderApi::drawFrame(gFrame);

  gFrame.irisDirty = false;
  gFrame.lidsDirty = false;
}

void EyeApi::update(uint32_t dtMs) {
  // gaze hýbe irisem (nastaví irisX/irisY + irisDirty podle potřeby)
  GazeApi::update(dtMs, gFrame);

  // lids zatím může jen držet hodnoty / připravit budoucí blink
  // (pokud nic nedělá, nechá lidTop/lidBot, a nelidsDirty bude false)
  LidsApi::update(dtMs, gFrame);

  RenderApi::drawFrame(gFrame);

  gFrame.irisDirty = false;
  gFrame.lidsDirty = false;
}
