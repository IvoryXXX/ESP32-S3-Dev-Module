#pragma once
#include <Arduino.h>
#include "config.h"

// Pupil V1.1
// - drží vlastní stav velikosti
// - update() vrací true, když se vizuálně změnila velikost a je potřeba překreslit iris patch
namespace EyePupil {
  void init(const Config& cfg);
  bool update(uint32_t nowMs, bool irisMoved, bool arrivedThisTick, uint32_t newDwellMsIfArrived);

  // vykreslení do iris patch bufferu (RGB565)
  void drawIntoPatch(uint16_t* patch, int w, int h);

  // pro debug / logy
  int currentPx();
}
