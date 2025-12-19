#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "skin_assets.h"   // SkinAssets / SkinRect

struct EyeRenderConfig {
  int baseW = 240;
  int baseH = 240;

  int irisW = 66;
  int irisH = 70;

  bool     useKey      = true;
  uint16_t keyColor565 = 0xF81F;
  uint8_t  tolR        = 3;
  uint8_t  tolG        = 6;
  uint8_t  tolB        = 3;
};

void eyeRenderInit(const EyeRenderConfig& cfg);
void eyeRenderSetKey(bool useKey, uint16_t keyColor565, uint8_t tolR, uint8_t tolG, uint8_t tolB);

bool eyeRenderLoadAssets(const SkinAssets& skin);
void eyeRenderDrawStatic(const SkinAssets& skin);
void eyeRenderDrawIris(int centerX, int centerY, const SkinAssets& skin);

// Patch 10: nové – vykreslení víček podle procenta zavření (0..1000)
// topClosePct: 0=open, 1000=full closed
// botClosePct: 0=open, 1000=full closed
void eyeRenderDrawLids(uint16_t topClosePct, uint16_t botClosePct, const SkinAssets& skin);
