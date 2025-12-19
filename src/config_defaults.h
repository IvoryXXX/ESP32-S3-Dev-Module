#pragma once
#include "config.h"

inline void loadConfigDefaults(Config &c) {
  // =========================
  // HW (SD / TFT)
  // =========================
  c.sdCs   = 4;
  c.sdSck  = 13;
  c.sdMiso = 12;
  c.sdMosi = 18;

  c.screenW = 240;
  c.screenH = 240;

  // aktivní skin (později můžeš měnit třeba na "/skins/lizard")
  c.skinDir = "/skins/human";

  // --------------------------------------------------------------------------
  // PRŮHLEDNOST (COLOR KEY)
  //
  // My používáme “magenta = průhledná”. V RAW565 je to typicky 0xF81F.
  // Tolerance je schválně malá, aby to nežralo detaily.
  //
  // 4 kombinace, co se typicky ladí:
  //   A) useKey=0                    → žádná průhlednost (debug, test)
  //   B) useKey=1, keyColor=0xF81F   → magenta key (nejčastější)
  //   C) useKey=1, keyColor=0xFFFF   → bílá key (když assets mají bílý key)
  //   D) useKey=1, keyColor=0x0000   → černá key (když assets mají černý key)
  //
  // Tip: když ti “mizí” i části obrázku, sniž tolerance.
  // --------------------------------------------------------------------------
  c.useKey      = true;
  c.keyColor565 = 0xF81F; // magenta
  c.keyTolR     = 3;
  c.keyTolG     = 6;
  c.keyTolB     = 3;

  // --------------------------------------------------------------------------
  // GRID sampling (vzorkování kruhu)
  // stepX/Y = rozteč bodů pohybu
  // menší = jemnější, větší = skokovější (a méně bodů)
  // --------------------------------------------------------------------------
  c.stepX = 16;
  c.stepY = 16;

  // --------------------------------------------------------------------------
  // IRIS CIRCLE (ladicí kruh + omezení)
  //
  // irisCircleMode:
  //   0 = CENTER: kruh říká, kde smí být STŘED irisu
  //       (když dáš větší radius, iris “dojede dál”, ale může vizuálně přesahovat bulvu)
  //
  //   1 = EDGE: kruh říká, kde smí být OKRAJ irisu
  //       -> když iris má třeba poloměr 35 px a kruh je 120 px,
  //          tak střed se ve skutečnosti drží cca v 120-35-marg.
  //
  // irisCircleRadiusPx:
  //   příklad: 110 menší kruh / 120 default / 130 větší kruh
  //
  // irisSafetyMarginPx:
  //   0–2  → těsně u okraje (agresivní)
  //   4–6  → bezpečné (doporučeno)
  // --------------------------------------------------------------------------
  c.irisCircleMode      = 0;
  c.irisCircleRadiusPx  = 80;
  c.irisSafetyMarginPx  = 4;

  // --------------------------------------------------------------------------
  // MASK SHIFT pro víčka (grid maska)
  // + = posune masku ke středu (víc blokuje)
  // - = posune masku ke krajům (míň blokuje)
  //
  // Default si klidně dej “trochu do víček” a pak to doladíš přes settings.txt.
  // --------------------------------------------------------------------------
  c.topLidMaskShiftPx = +6;
  c.botLidMaskShiftPx = -4;

  // --------------------------------------------------------------------------
  // GAZE (pohyb)
  // --------------------------------------------------------------------------
  c.dwellMinMs      = 400;
  c.dwellMaxMs      = 1400;
  c.travelTickMinMs = 25;
  c.travelTickMaxMs = 80;

  c.centerPct = 35;
  c.avoidSame = true;

  c.hopMin = 1;
  c.hopMax = 3;

  c.edgeBands     = 2;
  c.edgeWeightPct = 35;
  c.edgeSoft      = true;

  // --------------------------------------------------------------------------
  // PUPIL (ZORNICE) – V1.1
  //
  // pupilColor565 příklady (RGB565):
  //   0x0000 = černá
  //   0xFFFF = bílá
  //   0xF800 = červená
  //   0x07E0 = zelená
  //   0x001F = modrá
  //
  // pupilStepDeltaPx:
  //   změna na “step” pohybu – doporučeno 0..2
  //   (tím zajistíš, že pupil nebude při pohybu dělat velké skoky → méně blikání)
  // --------------------------------------------------------------------------
  c.pupilEnabled  = true;
  c.pupilColor565 = 0x0000;

  c.pupilBasePx = 35;
  c.pupilMinPx  = 15;
  c.pupilMaxPx  = 50;

  c.pupilAnimEnabled = true;

  c.pupilChangeMsMin = 220;
  c.pupilChangeMsMax = 520;

  c.pupilHoldMsMin   = 260;
  c.pupilHoldMsMax   = 900;

  c.pupilArriveExtraAfterMs = 1000;

  c.pupilStepDeltaPx = 1;

  // --------------------------------------------------------------------------
  // DEBUG
  // --------------------------------------------------------------------------
  c.dbgOverlay    = true;
  c.dbgGrid       = true;
  c.dbgIrisCircle = true;
  c.dbgLidRects   = true;
  c.dbgTarget     = true;
}
