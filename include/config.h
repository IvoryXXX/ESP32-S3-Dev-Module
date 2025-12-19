#pragma once
#include <Arduino.h>

struct Config {
  // --- Mrkání ---
  bool lidsEnabled;   // povolení mrkání / zavřených víček
  // --- SD pins (ESP32-S3, HSPI) ---
  int sdCs   = 4;
  int sdSck  = 13;
  int sdMiso = 12;
  int sdMosi = 18;

  // --- TFT ---
  int screenW = 240;
  int screenH = 240;

  // --- Skin folder ---
  const char* skinDir = "/skins/human";

  // --------------------------------------------------------------------------
  // PRŮHLEDNOST (COLOR KEY)
  // magenta = průhledná (typicky 0xF81F)
  // --------------------------------------------------------------------------
  bool     useKey      = true;
  uint16_t keyColor565 = 0xF81F;
  uint8_t  keyTolR     = 3;
  uint8_t  keyTolG     = 6;
  uint8_t  keyTolB     = 3;

  // --------------------------------------------------------------------------
  // GRID sampling (vzorkování kruhu)
  // --------------------------------------------------------------------------
  int stepX = 32;
  int stepY = 32;

  // --------------------------------------------------------------------------
  // IRIS "kruh reality" pro ladění + omezení pohybu (designová volba)
  // irisCircleMode:
  //   0 = CENTER (kruh je “bezpečná zóna pro STŘED irisu”)
  //   1 = EDGE   (kruh je “bezpečná zóna pro OKRAJ irisu”)
  // irisCircleRadiusPx = ladicí kruh v px
  // irisSafetyMarginPx = rezerva od okraje (jen pro EDGE mód)
  // --------------------------------------------------------------------------
  uint8_t irisCircleMode      = 0;
  int     irisCircleRadiusPx  = 120;
  int     irisSafetyMarginPx  = 4;

  // --------------------------------------------------------------------------
  // MASKA VÍČEK pro GRID (posun masky vůči "open lid" obdélníku)
  // + = ke středu (víc blokuje), - = ke krajům (míň blokuje)
  // --------------------------------------------------------------------------
  int topLidMaskShiftPx = 0;
  int botLidMaskShiftPx = 0;

  // --------------------------------------------------------------------------
  // GAZE (pohyb irisu mezi body gridu)
  // --------------------------------------------------------------------------
  uint32_t dwellMinMs      = 400;
  uint32_t dwellMaxMs      = 1400;
  uint32_t travelTickMinMs = 25;
  uint32_t travelTickMaxMs = 80;

  uint8_t centerPct = 35;
  bool    avoidSame = true;

  uint8_t hopMin = 1;
  uint8_t hopMax = 3;

  // --- Soft edge weighting ---
  uint8_t edgeBands     = 2;
  uint8_t edgeWeightPct = 35;
  bool    edgeSoft      = true;

  // --------------------------------------------------------------------------
  // PUPIL (ZORNICE) – V1.1
  // kreslí se do iris patch, takže nevzniká blikání z překreslování celé scény
  //
  // pupilBasePx = “typická” velikost, když animace není nebo je vypnutá
  // pupilMinPx / pupilMaxPx = rozsah náhodných cílů (dilatace)
  //
  // pupil_change_ms_min/max = jak dlouho trvá změna velikosti
  // pupil_hold_ms_min/max   = jak dlouho zůstane velikost, než se vybere další cíl
  //
  // pupil_arrive_extra_after_ms:
  //   po ARRIVE do cíle udělá pupil ještě jednu změnu po X ms (když je dwell dost dlouhý)
  //
  // pupil_step_delta_px:
  //   jemná změna na KAŽDÝ krok pohybu (0 = nic). Drží to “neblikací” mikrozměny.
  // --------------------------------------------------------------------------
  bool     pupilEnabled = true;
  uint16_t pupilColor565 = 0x0000; // default černá

  int pupilBasePx = 35;
  int pupilMinPx  = 15;
  int pupilMaxPx  = 50;

  bool     pupilAnimEnabled = true;

  uint32_t pupilChangeMsMin = 220;
  uint32_t pupilChangeMsMax = 520;

  uint32_t pupilHoldMsMin   = 260;
  uint32_t pupilHoldMsMax   = 900;

  uint32_t pupilArriveExtraAfterMs = 1000;

  int pupilStepDeltaPx = 1; // doporučení: 0..2

  // --- Debug HUD flags (kvůli debug_grid.cpp) ---
  bool dbgOverlay    = true;
  bool dbgGrid       = true;
  // Pokud true, overlay vykreslí i "ALL" body kruhu (včetně zamaskovaných).
  // Pokud false, kreslí jen povolené (ALLOWED) body po maskách.
  bool dbgAllPoints  = false;
  bool dbgIrisCircle = true;
  bool dbgLidRects   = true;
  bool dbgTarget     = true;

};

extern Config cfg;

// apply defaulty z config_defaults.h
void applyDefaults();
