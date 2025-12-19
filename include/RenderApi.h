#pragma once
#include <stdint.h>

struct SkinAssets; // forward declaration

namespace RenderApi {
  void init(const SkinAssets& skin);
  void drawStatic();
  void drawIris(int16_t irisX, int16_t irisY);
}
