#pragma once
#include <stdbool.h>
#include <stdint.h>

struct SkinAssets;

namespace RenderApi {
  // uloží pointer na skin (musí být zavoláno před setup/load/draw)
  void init(const SkinAssets& skin);

  // vytvoří EyeRenderConfig z cfg + skin a zavolá eyeRenderInit()
  void setupRendererFromConfig();

  // zavolá eyeRenderLoadAssets(skin)
  bool loadAssets();

  void drawStatic();
  void drawIris(int16_t irisX, int16_t irisY);
}
