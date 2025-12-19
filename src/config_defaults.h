#pragma once
#include "config.h"

static inline void loadConfigDefaults(Config& c) {
  // ZÁKLAD
  c.screenW = 240;
  c.screenH = 240;
  c.skinDir = "/skins/human";

  // SD (tvé ověřené piny)
  c.sdCs   = 4;
  c.sdSck  = 13;
  c.sdMosi = 18;
  c.sdMiso = 12;
  c.sdFreqHz = 250000;

  // Magenta key
  c.useKey      = true;
  c.keyColor565 = 0xF81F;
  c.keyTolR     = 3;
  c.keyTolG     = 6;
  c.keyTolB     = 3;

  // IRIS/GRID
  c.irisCircleRadiusPx = 70;
  c.irisCircleMode = 0;
  c.irisSafetyMarginPx = 0;

  c.stepX = 6;
  c.stepY = 6;

  c.edgeBands = 3;
  c.edgeWeightPct = 35;
  c.edgeSoft = true;

  // Lid mask shift
  c.topLidMaskShiftPx = 0;
  c.botLidMaskShiftPx = 0;

  // GAZE
  c.dwellMinMs = 450;
  c.dwellMaxMs = 1200;

  c.travelTickMinMs = 16;
  c.travelTickMaxMs = 28;

  c.centerPct = 25;
  c.avoidSame = true;

  c.hopMin = 1;
  c.hopMax = 3;

  // DEBUG
  c.dbgOverlay    = true;
  c.dbgGrid       = true;
  c.dbgAllPoints  = false;
  c.dbgIrisCircle = true;
  c.dbgLidRects   = true;
  c.dbgTarget     = true;

  // PUPIL
  c.pupilEnabled = true;
  c.pupilColor565 = 0x0000;
  c.pupilBasePx = 10;
  c.pupilMinPx  = 6;
  c.pupilMaxPx  = 18;

  c.pupilAnimEnabled = true;
  c.pupilChangeMsMin = 250;
  c.pupilChangeMsMax = 800;
  c.pupilHoldMsMin   = 600;
  c.pupilHoldMsMax   = 1800;

  c.pupilArriveExtraAfterMs = 120;
  c.pupilStepDeltaPx = 1;

  // FEATURE FLAGS – zatím OFF, ať se „zavřená víčka“ vůbec nepletou do obrazu
  c.enableLids  = false;
  c.enableBlink = false;
}
