#pragma once
#include <stdint.h>
#include "skin_assets.h"

// Konfigurace rendereru (jednou při init)
struct EyeRenderConfig {
  int baseW = 240;
  int baseH = 240;
  uint16_t keyColor565 = 0xF81F; // magenta key
  bool swapBytes = true;         // RAW565 B režim: swapBytes=true
};

void eyeRenderInit(const EyeRenderConfig& cfg);
bool eyeRenderLoadAssets(const SkinAssets& skin);

void eyeRenderDrawStatic(const SkinAssets& skin);

// irisX/irisY = pozice referenčního bodu irisu (tak jak to render už používá v Patch 9)
void eyeRenderDrawIris(int irisX, int irisY, const SkinAssets& skin);

// Patch 10 hook (zatím BEZ blikání):
// funkce jen zajistí, že se po pohybu irisu znovu překreslí otevřená víčka.
// lidTop/lidBot jsou připravené, ale zatím ignorované.
void eyeRenderDrawLids(uint16_t lidTop, uint16_t lidBot, const SkinAssets& skin);
