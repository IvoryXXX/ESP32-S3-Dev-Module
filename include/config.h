#pragma once
#include <Arduino.h>
#include <stdint.h>

struct Config {
  // --------------------------------------------------------------------------
  // ZÁKLAD
  // --------------------------------------------------------------------------
  int screenW = 240;
  int screenH = 240;

  // skin
  const char* skinDir = "/skins/human";

  // --------------------------------------------------------------------------
  // SD (očekává sd_manager.cpp)
  // --------------------------------------------------------------------------
  int sdCs   = 4;
  int sdSck  = 13;
  int sdMosi = 18;
  int sdMiso = 12;
  uint32_t sdFreqHz = 250000; // z logu používáš 250k (stabilní)

  // --------------------------------------------------------------------------
  // Magenta key (renderer)
  // --------------------------------------------------------------------------
  bool     useKey      = true;
  uint16_t keyColor565 = 0xF81F;
  uint8_t  keyTolR     = 3;
  uint8_t  keyTolG     = 6;
  uint8_t  keyTolB     = 3;

  // --------------------------------------------------------------------------
  // IRIS / GRID
  // --------------------------------------------------------------------------
  int irisCircleRadiusPx = 70;

  // (skin_config.cpp to parsuje, tak to musí existovat)
  uint8_t irisCircleMode = 0;      // 0=radius, 1=mask/whatever (teď jen držíme kontrakt)
  int irisSafetyMarginPx = 0;

  int stepX = 6;
  int stepY = 6;

  int edgeBands = 3;
  int edgeWeightPct = 35;
  bool edgeSoft = true;

  // --------------------------------------------------------------------------
  // LID mask shift (skin_config.cpp to parsuje)
  // --------------------------------------------------------------------------
  int topLidMaskShiftPx = 0;
  int botLidMaskShiftPx = 0;

  // --------------------------------------------------------------------------
  // GAZE (EyeGaze)
  // --------------------------------------------------------------------------
  uint32_t dwellMinMs = 450;
  uint32_t dwellMaxMs = 1200;

  uint32_t travelTickMinMs = 16;
  uint32_t travelTickMaxMs = 28;

  int centerPct = 25;
  bool avoidSame = true;

  int hopMin = 1;
  int hopMax = 3;

  // --------------------------------------------------------------------------
  // DEBUG overlay (debug_grid.cpp + render_eye.cpp)
  // --------------------------------------------------------------------------
  bool dbgOverlay    = true;
  bool dbgGrid       = true;
  bool dbgAllPoints  = false;
  bool dbgIrisCircle = true;
  bool dbgLidRects   = true;
  bool dbgTarget     = true;

  // --------------------------------------------------------------------------
  // PUPIL (eye_pupil.cpp + skin_config.cpp)
  // --------------------------------------------------------------------------
  bool pupilEnabled = true;
  uint16_t pupilColor565 = 0x0000; // černá

  int pupilBasePx = 10;
  int pupilMinPx  = 6;
  int pupilMaxPx  = 18;

  bool pupilAnimEnabled = true;
  uint32_t pupilChangeMsMin = 250;
  uint32_t pupilChangeMsMax = 800;
  uint32_t pupilHoldMsMin   = 600;
  uint32_t pupilHoldMsMax   = 1800;

  uint32_t pupilArriveExtraAfterMs = 120;
  int pupilStepDeltaPx = 1;

  // --------------------------------------------------------------------------
  // FEATURE FLAGS (EyeApi.cpp očekává enableLids/enableBlink)
  // --------------------------------------------------------------------------
  bool enableLids  = false; // zatím vypnuté – nic se nebude kreslit/řešit
  bool enableBlink = false; // mrkání připravené, ale OFF (tvůj návrh)
};

extern Config cfg;

void applyDefaults();
