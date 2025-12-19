#pragma once
#include <stdbool.h>
#include <stdint.h>

struct SkinAssets;
struct EyeFrame;

namespace RenderApi {
  void init(const SkinAssets& skin);
  void setupRendererFromConfig();
  bool loadAssets();

  void drawStatic();
  void drawIris(int16_t irisX, int16_t irisY);

  // Patch 6+: render podle kontraktu
  void renderFrame(const EyeFrame& frame);
}
