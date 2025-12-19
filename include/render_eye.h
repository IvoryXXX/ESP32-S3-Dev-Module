#pragma once
#include <Arduino.h>
#include <stdint.h>
#include "skin_assets.h"   // <-- aby existoval SkinAssets / SkinRect

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

// NOVÉ API (sedí na main.cpp i render_eye.cpp)
bool eyeRenderLoadAssets(const SkinAssets& skin);
void eyeRenderDrawStatic(const SkinAssets& skin);
void eyeRenderDrawIris(int centerX, int centerY, const SkinAssets& skin);
